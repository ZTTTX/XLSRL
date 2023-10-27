// Copyright 2020 The XLS Authors
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
#include <cstdint>
#include <filesystem>  // NOLINT
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "xls/tools/RewriteHandler.h"
#include "xls/tools/JsonHandler.h"
#include "xls/tools/RunScheduler.h"
#include <fstream>


#include "absl/flags/flag.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "xls/codegen/module_signature.h"
#include "xls/common/exit_status.h"
#include "xls/common/file/filesystem.h"
#include "xls/common/init_xls.h"
#include "xls/common/logging/logging.h"
#include "xls/common/status/ret_check.h"
#include "xls/common/status/status_macros.h"
#include "xls/ir/function_base.h"
#include "xls/ir/ir_parser.h"
#include "xls/ir/verifier.h"
#include "xls/scheduling/pipeline_schedule.pb.h"
#include "xls/tools/codegen.h"
#include "xls/tools/codegen_flags.h"
#include "xls/tools/codegen_flags.pb.h"
#include "xls/tools/scheduling_options_flags.h"
#include "xls/tools/scheduling_options_flags.pb.h"

#include "xls/scheduling/pipeline_schedule.h"
#include "xls/scheduling/scheduling_pass.h"
#include "xls/scheduling/scheduling_pass_pipeline.h"

const char kUsage[] = R"(
This is a test pass for XLS RL project.

Emit combinational module:
   RL_main --output_verilog_path=DIR IR_FILE

)";

namespace xls {
// namespace {
absl::Status DumpIRToFile(const std::string& ir_content, const std::string& output_file_path) {
    std::ofstream output_file(output_file_path, std::ios::out | std::ios::trunc);
    if (!output_file.is_open()) {
        return absl::Status(absl::StatusCode::kUnknown, "Failed to open the output file.");
    }

    output_file << ir_content;
    output_file.close();

    return absl::OkStatus();
}


absl::Status SingleRewrite(std::string_view ir_path, std::string_view json_path, std::string_view out_path, std::vector<std::string_view> positional_arguments) {
  if (ir_path == "-") {
    ir_path = "/dev/stdin";
  }
  XLS_ASSIGN_OR_RETURN(std::string ir_contents, GetFileContents(ir_path));
  XLS_ASSIGN_OR_RETURN(std::unique_ptr<Package> p,
                       Parser::ParsePackage(ir_contents, ir_path));

  XLS_RET_CHECK(p->GetTop().has_value())
      << "Package " << p->name() << " needs a top function/proc.";

  //Read Json file, related files are JsonHandler.h
  std::vector<JsonSingleSub> AllSubs;
  AllSubs = ReadJsonByPath(json_path);

  //Init rewrite handler instance and handle all substitution, dump IR file after done
  //related files are RewriteHandler.cc and .h
  
  for (const JsonSingleSub& CurSub : AllSubs) {
    XLS_RETURN_IF_ERROR(HandleSingleSub(p.get(), CurSub));
  }
  std::string RewritedIR = p->DumpIr();
  XLS_RETURN_IF_ERROR(DumpIRToFile(RewritedIR, std::string(out_path)));

  //Call Scheduler
  XLS_RETURN_IF_ERROR(RunScheduler(out_path, positional_arguments));
  //


  return absl::OkStatus();
}

// }  // namespaceR
}  // namespace xls

int main(int argc, char** argv) {
  std::vector<std::string_view> positional_arguments =
      xls::InitXls(kUsage, argc, argv);

  if (positional_arguments.size() < 3) {
    XLS_LOG(QFATAL) << absl::StreamFormat("Expected invocation: %s IR_FILE",
                                          argv[0]);
  }
  
  std::string_view ir_path = positional_arguments[0];
  std::string_view json_path = positional_arguments[1];
  std::string_view out_path = positional_arguments[2];


  return xls::ExitStatus(xls::SingleRewrite(ir_path, json_path, out_path, positional_arguments));
}
