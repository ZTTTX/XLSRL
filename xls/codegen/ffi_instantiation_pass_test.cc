// Copyright 2023 The XLS Authors
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

#include "xls/codegen/ffi_instantiation_pass.h"

#include <algorithm>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "xls/codegen/codegen_pass.h"
#include "xls/common/casts.h"
#include "xls/common/status/matchers.h"
#include "xls/ir/bits.h"
#include "xls/ir/foreign_function.h"
#include "xls/ir/function_builder.h"
#include "xls/ir/instantiation.h"
#include "xls/ir/ir_test_base.h"
#include "xls/ir/nodes.h"
#include "xls/ir/type.h"
#include "xls/ir/verifier.h"
#include "xls/passes/pass_base.h"

namespace xls::verilog {
namespace {
using status_testing::IsOkAndHolds;
using status_testing::StatusIs;

class FfiInstantiationPassTest : public IrTestBase {
 protected:
  absl::StatusOr<bool> Run(Block* block) {
    PassResults results;
    CodegenPassUnit unit(block->package(), block);
    return FfiInstantiationPass().Run(&unit, CodegenPassOptions(), &results);
  }
};

TEST_F(FfiInstantiationPassTest, InvocationsReplacedByInstance) {
  auto p = CreatePackage();
  BitsType* const u32 = p->GetBitsType(32);
  BitsType* const u17 = p->GetBitsType(17);

  // Simple function that has foreign function data attached.
  FunctionBuilder fb(TestName() + "ffi_fun", p.get());
  const BValue param_a = fb.Param("a", u32);
  const BValue param_b = fb.Param("b", u17);
  const BValue add = fb.Add(param_a, fb.ZeroExtend(param_b, 32));
  XLS_ASSERT_OK_AND_ASSIGN(ForeignFunctionData ffd,
                           ForeignFunctionDataCreateFromTemplate(
                               "foo {fn} (.ma({a}), .mb{b}) .out({return})"));
  fb.SetForeignFunctionData(ffd);
  XLS_ASSERT_OK_AND_ASSIGN(Function * ffi_fun, fb.BuildWithReturnValue(add));

  // A block that contains one invocation of that ffi_fun.
  BlockBuilder bb(TestName(), p.get());
  const BValue input_port_a = bb.InputPort("block_a_input", u32);
  const BValue input_port_b = bb.InputPort("block_b_input", u17);
  bb.OutputPort("out", bb.Invoke({input_port_a, input_port_b}, ffi_fun));
  XLS_ASSERT_OK_AND_ASSIGN(Block * block, bb.Build());

  // Precondition: one invoke(), and no instantiations
  EXPECT_EQ(1, std::count_if(block->nodes().begin(), block->nodes().end(),
                             [](Node* n) { return n->Is<Invoke>(); }));
  EXPECT_THAT(block->GetInstantiations(), testing::IsEmpty());

  // First round we find an invoke to create an instantiation from.
  EXPECT_THAT(Run(block), IsOkAndHolds(true));

  // Nothing to be done the second time around.
  EXPECT_THAT(Run(block), IsOkAndHolds(false));

  // Postcondition: no invoke() and one instantiation.
  // instantiation to be in the block referencing the original function.
  EXPECT_EQ(0, std::count_if(block->nodes().begin(), block->nodes().end(),
                             [](Node* n) { return n->Is<Invoke>(); }));
  ASSERT_EQ(block->GetInstantiations().size(), 1);

  xls::Instantiation* const instantiation = block->GetInstantiations()[0];
  ASSERT_EQ(instantiation->kind(), InstantiationKind::kExtern);

  xls::ExternInstantiation* const extern_inst =
      down_cast<xls::ExternInstantiation*>(instantiation);
  EXPECT_EQ(extern_inst->function(), ffi_fun);
  for (std::string_view param : {"a", "b"}) {
    XLS_ASSERT_OK_AND_ASSIGN(InstantiationPort input_param,
                             extern_inst->GetInputPort(param));
    EXPECT_EQ(input_param.name, param);
    EXPECT_EQ(input_param.type, param == "a" ? u32 : u17);
  }

  XLS_ASSERT_OK_AND_ASSIGN(InstantiationPort return_port,
                           extern_inst->GetOutputPort("return"));
  EXPECT_EQ(return_port.name, "return");
  EXPECT_EQ(return_port.type, u32);

  // Requesting a non-existent port.
  EXPECT_THAT(extern_inst->GetInputPort("bogus"),
              StatusIs(absl::StatusCode::kNotFound));

  // Explicitly testing the resulting block passes verification.
  XLS_EXPECT_OK(VerifyPackage(p.get()));
}

TEST_F(FfiInstantiationPassTest, FunctionTakingNoParametersJustReturns) {
  constexpr int kReturnBitCount = 17;
  auto p = CreatePackage();

  // Simple function that has foreign function data attached.
  FunctionBuilder fb(TestName() + "ffi_fun", p.get());
  BValue retval = fb.Literal(UBits(42, kReturnBitCount));
  XLS_ASSERT_OK_AND_ASSIGN(
      ForeignFunctionData ffd,
      ForeignFunctionDataCreateFromTemplate("foo {fn} (.out({return}))"));
  fb.SetForeignFunctionData(ffd);
  XLS_ASSERT_OK_AND_ASSIGN(Function * ffi_fun, fb.BuildWithReturnValue(retval));

  // A block that contains one invocation of that ffi_fun.
  BlockBuilder bb(TestName(), p.get());
  bb.OutputPort("out", bb.Invoke({}, ffi_fun));
  XLS_ASSERT_OK_AND_ASSIGN(Block * block, bb.Build());

  // Convert to instance
  EXPECT_THAT(Run(block), IsOkAndHolds(true));

  xls::Instantiation* const instantiation = block->GetInstantiations()[0];
  ASSERT_EQ(instantiation->kind(), InstantiationKind::kExtern);

  xls::ExternInstantiation* const extern_inst =
      down_cast<xls::ExternInstantiation*>(instantiation);

  XLS_ASSERT_OK_AND_ASSIGN(InstantiationPort return_port,
                           extern_inst->GetOutputPort("return"));
  EXPECT_EQ(return_port.name, "return");
  EXPECT_EQ(return_port.type, p->GetBitsType(kReturnBitCount));
}

TEST_F(FfiInstantiationPassTest, FunctionReturningTuple) {
  constexpr int kReturnBitCount[] = {17, 27};
  auto p = CreatePackage();

  // Function returning a tuple.
  FunctionBuilder fb(TestName() + "ffi_fun", p.get());
  BValue retval = fb.Tuple({fb.Literal(UBits(42, kReturnBitCount[0])),
                            fb.Literal(UBits(24, kReturnBitCount[1]))});
  XLS_ASSERT_OK_AND_ASSIGN(
      ForeignFunctionData ffd,
      ForeignFunctionDataCreateFromTemplate("foo {fn} (.foo({return.0}), "
                                            ".bar({return.1}))"));
  fb.SetForeignFunctionData(ffd);
  XLS_ASSERT_OK_AND_ASSIGN(Function * ffi_fun, fb.BuildWithReturnValue(retval));

  // A block that contains one invocation of that ffi_fun.
  BlockBuilder bb(TestName(), p.get());
  bb.OutputPort("out", bb.Invoke({}, ffi_fun));
  XLS_ASSERT_OK_AND_ASSIGN(Block * block, bb.Build());

  // Convert to instance
  EXPECT_THAT(Run(block), IsOkAndHolds(true));

  xls::Instantiation* const instantiation = block->GetInstantiations()[0];
  ASSERT_EQ(instantiation->kind(), InstantiationKind::kExtern);

  xls::ExternInstantiation* const extern_inst =
      down_cast<xls::ExternInstantiation*>(instantiation);

  for (int i = 0; i < 2; ++i) {
    const std::string return_name = absl::StrCat("return.", i);
    XLS_ASSERT_OK_AND_ASSIGN(InstantiationPort return_port,
                             extern_inst->GetOutputPort(return_name));
    EXPECT_EQ(return_port.name, return_name);
    EXPECT_EQ(return_port.type, p->GetBitsType(kReturnBitCount[i]));
  }
}

}  // namespace
}  // namespace xls::verilog
