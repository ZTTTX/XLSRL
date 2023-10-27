// Copyright 2024 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xls/tools/codegen.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "xls/codegen/codegen_options.h"
#include "xls/codegen/combinational_generator.h"
#include "xls/codegen/module_signature.h"
#include "xls/codegen/op_override_impls.h"
#include "xls/codegen/pipeline_generator.h"
#include "xls/codegen/ram_configuration.h"
#include "xls/common/logging/logging.h"
#include "xls/common/status/ret_check.h"
#include "xls/common/status/status_macros.h"
#include "xls/delay_model/delay_estimator.h"
#include "xls/delay_model/ffi_delay_estimator.h"
#include "xls/fdo/synthesizer.h"
#include "xls/ir/function_base.h"
#include "xls/ir/op.h"
#include "xls/ir/verifier.h"
#include "xls/scheduling/pipeline_schedule.h"
#include "xls/scheduling/scheduling_options.h"
#include "xls/scheduling/scheduling_pass.h"
#include "xls/scheduling/scheduling_pass_pipeline.h"
#include "xls/tools/codegen_flags.pb.h"
#include "xls/tools/scheduling_options_flags.h"
#include "xls/tools/scheduling_options_flags.pb.h"

namespace xls {
namespace {

verilog::CodegenOptions::IOKind ToIOKind(IOKindProto p) {
  switch (p) {
    case IO_KIND_INVALID:
    case IO_KIND_FLOP:
      return verilog::CodegenOptions::IOKind::kFlop;
    case IO_KIND_SKID_BUFFER:
      return verilog::CodegenOptions::IOKind::kSkidBuffer;
    case IO_KIND_ZERO_LATENCY_BUFFER:
      return verilog::CodegenOptions::IOKind::kZeroLatencyBuffer;
    default:
      XLS_LOG(FATAL) << "Invalid IOKindProto value: " << static_cast<int>(p);
  }
}

absl::StatusOr<verilog::CodegenOptions> CodegenOptionsFromProto(
    const CodegenFlagsProto& p) {
  verilog::CodegenOptions options;

  if (p.generator() == GENERATOR_KIND_PIPELINE) {
    options = verilog::BuildPipelineOptions();

    if (!p.input_valid_signal().empty()) {
      std::optional<std::string_view> output_signal;
      if (!p.output_valid_signal().empty()) {
        output_signal = p.output_valid_signal();
      }
      options.valid_control(p.input_valid_signal(), output_signal);
    } else if (!p.manual_load_enable_signal().empty()) {
      options.manual_control(p.manual_load_enable_signal());
    }
    options.flop_inputs(p.flop_inputs());
    options.flop_outputs(p.flop_outputs());
    options.flop_inputs_kind(ToIOKind(p.flop_inputs_kind()));
    options.flop_outputs_kind(ToIOKind(p.flop_outputs_kind()));

    options.flop_single_value_channels(p.flop_single_value_channels());
    options.add_idle_output(p.add_idle_output());

    if (!p.reset().empty()) {
      options.reset(p.reset(), p.reset_asynchronous(), p.reset_active_low(),
                    p.reset_data_path());
    }
  }

  if (!p.module_name().empty()) {
    options.module_name(p.module_name());
  }

  options.use_system_verilog(p.use_system_verilog());
  options.separate_lines(p.separate_lines());

  if (!p.gate_format().empty()) {
    options.SetOpOverride(
        Op::kGate,
        std::make_unique<verilog::OpOverrideGateAssignment>(p.gate_format()));
  }

  if (!p.assert_format().empty()) {
    options.SetOpOverride(
        Op::kAssert,
        std::make_unique<verilog::OpOverrideAssertion>(p.assert_format()));
  }

  if (!p.smulp_format().empty()) {
    options.SetOpOverride(
        Op::kSMulp,
        std::make_unique<verilog::OpOverrideInstantiation>(p.smulp_format()));
  }

  if (!p.umulp_format().empty()) {
    options.SetOpOverride(
        Op::kUMulp,
        std::make_unique<verilog::OpOverrideInstantiation>(p.umulp_format()));
  }

  options.streaming_channel_data_suffix(p.streaming_channel_data_suffix());
  options.streaming_channel_valid_suffix(p.streaming_channel_valid_suffix());
  options.streaming_channel_ready_suffix(p.streaming_channel_ready_suffix());

  std::vector<std::unique_ptr<verilog::RamConfiguration>> ram_configurations;
  ram_configurations.reserve(p.ram_configurations_size());
  for (const std::string& config_text : p.ram_configurations()) {
    XLS_ASSIGN_OR_RETURN(std::unique_ptr<verilog::RamConfiguration> config,
                         verilog::RamConfiguration::ParseString(config_text));
    ram_configurations.push_back(std::move(config));
  }
  options.ram_configurations(ram_configurations);

  options.gate_recvs(p.gate_recvs());
  options.array_index_bounds_checking(p.array_index_bounds_checking());

  return options;
}


absl::StatusOr<PipelineSchedule> RunSchedulingPipeline(
    FunctionBase* main, const SchedulingOptions& scheduling_options,
    const DelayEstimator* delay_estimator,
    synthesis::Synthesizer* synthesizer) {
  Package* p = main->package();
  SchedulingPassOptions sched_options;
  sched_options.scheduling_options = scheduling_options;
  sched_options.delay_estimator = delay_estimator;
  sched_options.synthesizer = synthesizer;
  std::unique_ptr<SchedulingCompoundPass> scheduling_pipeline =
      CreateSchedulingPassPipeline();
  SchedulingPassResults results;
  SchedulingUnit<> scheduling_unit = {p, /*schedule=*/std::nullopt};
  absl::Status scheduling_status =
      scheduling_pipeline->Run(&scheduling_unit, sched_options, &results)
          .status();
  if (!scheduling_status.ok()) {
    if (absl::IsResourceExhausted(scheduling_status)) {
      // Resource exhausted error indicates that the schedule was
      // infeasible. Emit a meaningful error in this case.
      std::string error_message = "Design cannot be scheduled";
      if (scheduling_options.pipeline_stages().has_value()) {
        absl::StrAppendFormat(&error_message, " in %d stages",
                              scheduling_options.pipeline_stages().value());
      }
      if (scheduling_options.clock_period_ps().has_value()) {
        absl::StrAppendFormat(&error_message, " with a %dps clock",
                              scheduling_options.clock_period_ps().value());
      }
      return xabsl::StatusBuilder(scheduling_status).SetPrepend()
             << error_message << ": ";
    }
    return scheduling_status;
  }
  XLS_RET_CHECK(scheduling_unit.schedule.has_value());

  return scheduling_unit.schedule.value();
}

}  // namespace

absl::StatusOr<CodegenResult> ScheduleAndCodegen(
    Package* p,
    const SchedulingOptionsFlagsProto& scheduling_options_flags_proto,
    const CodegenFlagsProto& codegen_flags_proto, bool with_delay_model) {
  if (!codegen_flags_proto.top().empty()) {
    XLS_RETURN_IF_ERROR(p->SetTopByName(codegen_flags_proto.top()));
  }
  XLS_ASSIGN_OR_RETURN(verilog::CodegenOptions codegen_options,
                       CodegenOptionsFromProto(codegen_flags_proto));

  XLS_RET_CHECK(p->GetTop().has_value())
      << "Package " << p->name() << " needs a top function/proc.";
  auto main = [&p]() -> FunctionBase* { return p->GetTop().value(); };

  XLS_ASSIGN_OR_RETURN(
      SchedulingOptions scheduling_options,
      SetUpSchedulingOptions(scheduling_options_flags_proto, p));
  if (codegen_flags_proto.generator() == GENERATOR_KIND_PIPELINE) {
    XLS_QCHECK(scheduling_options.pipeline_stages() != 0 ||
               scheduling_options.clock_period_ps() != 0)
        << "Must specify --pipeline_stages or --clock_period_ps (or both).";

    // Add IO constraints for RAMs.
    for (const std::unique_ptr<xls::verilog::RamConfiguration>& ram_config :
         codegen_options.ram_configurations()) {
      for (const IOConstraint& ram_constraint :
           ram_config->GetIOConstraints()) {
        scheduling_options.add_constraint(ram_constraint);
      }
    }

    XLS_ASSIGN_OR_RETURN(const DelayEstimator* base_estimator,
                         SetUpDelayEstimator(scheduling_options_flags_proto));
    const FfiDelayEstimator ffi_estimator(
        scheduling_options.ffi_fallback_delay_ps());

    FirstMatchDelayEstimator delay_estimator("combined_estimator",
                                             {base_estimator, &ffi_estimator});

    synthesis::Synthesizer* synthesizer = nullptr;
    if (!scheduling_options.fdo_synthesizer_name().empty()) {
      XLS_ASSIGN_OR_RETURN(synthesizer,
                           SetUpSynthesizer(scheduling_options_flags_proto));
    }

    XLS_ASSIGN_OR_RETURN(PipelineSchedule schedule,
                         RunSchedulingPipeline(main(), scheduling_options,
                                               &delay_estimator, synthesizer));

    XLS_RETURN_IF_ERROR(VerifyPackage(p, /*codegen=*/true));

    XLS_ASSIGN_OR_RETURN(
        verilog::ModuleGeneratorResult result,
        verilog::ToPipelineModuleText(schedule, main(), codegen_options,
                                      &delay_estimator));
    return CodegenResult{
        .module_generator_result = result,
        .pipeline_schedule_proto = schedule.ToProto(delay_estimator),
    };
  }

  if (codegen_flags_proto.generator() == GENERATOR_KIND_COMBINATIONAL) {
    const DelayEstimator* delay_estimator = nullptr;
    if (with_delay_model) {
      XLS_ASSIGN_OR_RETURN(delay_estimator,
                           SetUpDelayEstimator(scheduling_options_flags_proto));
    }
    XLS_ASSIGN_OR_RETURN(verilog::ModuleGeneratorResult result,
                         verilog::GenerateCombinationalModule(
                             main(), codegen_options, delay_estimator));
    return CodegenResult{.module_generator_result = result};
  }

  // Note: this should already be validated by CodegenFlagsFromAbslFlags().
  XLS_LOG(FATAL) << "Invalid generator kind: "
                 << static_cast<int>(codegen_flags_proto.generator());
}


}  // namespace xls
