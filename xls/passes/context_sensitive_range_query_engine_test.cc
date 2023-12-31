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

#include "xls/passes/context_sensitive_range_query_engine.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/algorithm/container.h"
#include "absl/types/span.h"
#include "xls/common/logging/logging.h"
#include "xls/common/status/matchers.h"
#include "xls/data_structures/leaf_type_tree.h"
#include "xls/ir/bits.h"
#include "xls/ir/bits_ops.h"
#include "xls/ir/function_base.h"
#include "xls/ir/function_builder.h"
#include "xls/ir/interval.h"
#include "xls/ir/interval_set.h"
#include "xls/ir/ir_test_base.h"
#include "xls/ir/node.h"
#include "xls/ir/nodes.h"
#include "xls/ir/type.h"
#include "xls/ir/value.h"
#include "xls/passes/predicate_state.h"
#include "xls/passes/range_query_engine.h"

namespace xls {
namespace {
using testing::AnyOf;
using testing::Eq;

class ContextSensitiveRangeQueryEngineTest : public IrTestBase {
 public:
  static constexpr PredicateState::ArmT kConsequentArm = 1;
  static constexpr PredicateState::ArmT kAlternateArm = 0;
  static const Bits kTrue;
  static const Bits kFalse;
};
const Bits ContextSensitiveRangeQueryEngineTest::kFalse = Bits(1);
const Bits ContextSensitiveRangeQueryEngineTest::kTrue = Bits::AllOnes(1);

class SignedContextSensitiveRangeQueryEngineTest
    : public ContextSensitiveRangeQueryEngineTest,
      public testing::WithParamInterface<bool> {
 public:
  bool IsSigned() const { return GetParam(); }
  Bits MinValue(int64_t bits) const {
    return IsSigned() ? Bits::MinSigned(bits) : Bits(bits);
  }
  Bits MaxValue(int64_t bits) const {
    return IsSigned() ? Bits::MaxSigned(bits) : Bits::AllOnes(bits);
  }
  BValue Gt(FunctionBuilder& fb, BValue l, BValue r) {
    if (IsSigned()) {
      return fb.SGt(l, r);
    }
    return fb.UGt(l, r);
  }
  BValue Lt(FunctionBuilder& fb, BValue l, BValue r) {
    if (IsSigned()) {
      return fb.SLt(l, r);
    }
    return fb.ULt(l, r);
  }
  BValue Ge(FunctionBuilder& fb, BValue l, BValue r) {
    if (IsSigned()) {
      return fb.SGe(l, r);
    }
    return fb.UGe(l, r);
  }
  BValue Le(FunctionBuilder& fb, BValue l, BValue r) {
    if (IsSigned()) {
      return fb.SLe(l, r);
    }
    return fb.ULe(l, r);
  }
};

LeafTypeTree<IntervalSet> BitsLTT(Node* node,
                                  absl::Span<const Interval> intervals) {
  XLS_CHECK(!intervals.empty());
  int64_t bit_count = intervals[0].BitCount();
  IntervalSet interval_set(bit_count);
  for (const Interval& interval : intervals) {
    XLS_CHECK_EQ(interval.BitCount(), bit_count);
    interval_set.AddInterval(interval);
  }
  interval_set.Normalize();
  XLS_CHECK(node->GetType()->IsBits());
  LeafTypeTree<IntervalSet> result(node->GetType());
  result.Set({}, interval_set);
  return result;
}

TEST_F(ContextSensitiveRangeQueryEngineTest, Eq) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x == 12) { x + 10 } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue cond = fb.Eq(x, fb.Literal(UBits(12, 8)));
  BValue add_ten = fb.Add(x, fb.Literal(UBits(10, 8)));
  BValue res = fb.Select(cond, {x, add_ten});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval::Precise(UBits(12, 8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree add_ten_ist =
      BitsLTT(x.node(), {Interval::Precise(UBits(22, 8))});
  IntervalSetTree add_ten_ist_global =
      BitsLTT(x.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(add_ten.node()), add_ten_ist_global);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);
  EXPECT_EQ(consequent_arm_range->GetIntervals(add_ten.node()), add_ten_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_F(ContextSensitiveRangeQueryEngineTest, Ne) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x != 12) { x } else { x + 10 }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue cond = fb.Ne(x, fb.Literal(UBits(12, 8)));
  BValue add_ten = fb.Add(x, fb.Literal(UBits(10, 8)));
  BValue res = fb.Select(cond, {add_ten, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval::Precise(UBits(12, 8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree add_ten_ist =
      BitsLTT(x.node(), {Interval::Precise(UBits(22, 8))});
  IntervalSetTree add_ten_ist_global =
      BitsLTT(x.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(add_ten.node()), add_ten_ist_global);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(add_ten.node()), add_ten_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, VariableLtConstantUseInIf) {
  Bits max_bits = bits_ops::Sub(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x < 12) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Lt(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       VariableLtConstantUseInElse) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x < 12) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Lt(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  RecordProperty("graph", f->DumpIr());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, VariableLeConstantUseInIf) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x <= 12) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Le(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       VariableLeConstantUseInElse) {
  Bits max_bits = bits_ops::Add(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x <= 12) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Le(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, VariableGtConstantUseInIf) {
  Bits max_bits = bits_ops::Add(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x > 12) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Gt(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       VariableGtConstantUseInElse) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x > 12) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Gt(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, VariableGeConstantUseInIf) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x >= 12) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Ge(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       VariableGeConstantUseInElse) {
  Bits max_bits = bits_ops::Sub(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x >= 12) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Ge(fb, x, fb.Literal(UBits(12, 8)));
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, ConstantLtVariableUseInIf) {
  Bits max_bits = bits_ops::Add(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 < x) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Lt(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       ConstantLtVariableUseInElse) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 < x) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Lt(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  RecordProperty("graph", f->DumpIr());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, ConstantLeVariableUseInIf) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 <= x) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Le(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       ConstantLeVariableUseInElse) {
  Bits max_bits = bits_ops::Sub(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 <= x) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Le(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, ConstantGtVariableUseInIf) {
  Bits max_bits = bits_ops::Sub(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 > x) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Gt(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       ConstantGtVariableUseInElse) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 > x) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Gt(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest, ConstantGeVariableUseInIf) {
  Bits max_bits = UBits(12, 8);
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 >= x) { x } else { y }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Ge(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {y, x});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(MinValue(8), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_P(SignedContextSensitiveRangeQueryEngineTest,
       ConstantGeVariableUseInElse) {
  Bits max_bits = bits_ops::Add(UBits(12, 8), UBits(1, 8));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (12 >= x) { y } else { x }
  BValue x = fb.Param("x", p->GetBitsType(8));
  BValue y = fb.Param("y", p->GetBitsType(8));
  BValue cond = Ge(fb, fb.Literal(UBits(12, 8)), x);
  BValue res = fb.Select(cond, {x, y});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(max_bits, MaxValue(8))});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(8)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(8)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(alternate_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_F(ContextSensitiveRangeQueryEngineTest,
       UseInComplicatedExpressionBubblesDown) {
  Bits max_bits = bits_ops::Sub(UBits(12, 64), UBits(1, 64));
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x < 12) { (x + 6) } else { y }
  BValue x = fb.Param("x", p->GetBitsType(64));
  BValue y = fb.Param("y", p->GetBitsType(64));
  BValue cond = fb.ULt(x, fb.Literal(UBits(12, 64)));
  BValue x_full = fb.Add(x, fb.Literal(UBits(6, 64)));
  BValue res = fb.Select(cond, {y, x_full});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));
  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  IntervalSetTree x_ist = BitsLTT(x.node(), {Interval(Bits(64), max_bits)});
  IntervalSetTree x_ist_global = BitsLTT(x.node(), {Interval::Maximal(64)});
  IntervalSetTree x_full_ist =
      BitsLTT(x_full.node(), {Interval(UBits(6, 64), UBits(17, 64))});
  IntervalSetTree x_full_ist_global =
      BitsLTT(x_full.node(), {Interval::Maximal(64)});

  IntervalSetTree y_ist = BitsLTT(y.node(), {Interval::Maximal(64)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist = BitsLTT(res.node(), {Interval::Maximal(64)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(x_full.node()), x_full_ist_global);
  EXPECT_EQ(engine.GetIntervals(y.node()), y_ist);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);
  EXPECT_EQ(consequent_arm_range->GetIntervals(x_full.node()), x_full_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(0, 1))}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(UBits(1, 1))}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_F(ContextSensitiveRangeQueryEngineTest, EqArray) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x == [12, 12]) { let a,b = x[0], x[1]; return [a + 10, b + 10] } else {
  // x }
  Type* type = p->GetArrayType(2, p->GetBitsType(8));
  BValue x = fb.Param("x", type);
  XLS_ASSERT_OK_AND_ASSIGN(Value twelve_array, Value::UBitsArray({12, 12}, 8));
  BValue cond = fb.Eq(x, fb.Literal(twelve_array));
  BValue a = fb.ArrayIndex(x, {fb.Literal(UBits(0, 1))});
  BValue b = fb.ArrayIndex(x, {fb.Literal(UBits(1, 1))});
  BValue add_ten = fb.Array({fb.Add(a, fb.Literal(UBits(10, 8))),
                             fb.Add(b, fb.Literal(UBits(10, 8)))},
                            p->GetBitsType(8));
  BValue res = fb.Select(cond, {x, add_ten});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));

  auto pair_interval =
      [&](Node* n, absl::Span<const Interval> first,
          absl::Span<const Interval> second) -> IntervalSetTree {
    IntervalSet first_interval(8);
    absl::c_for_each(first,
                     [&](const Interval& i) { first_interval.AddInterval(i); });
    first_interval.Normalize();
    IntervalSet second_interval(8);
    absl::c_for_each(
        second, [&](const Interval& i) { second_interval.AddInterval(i); });
    second_interval.Normalize();
    return IntervalSetTree(type, {first_interval, second_interval});
  };
  IntervalSetTree x_ist =
      pair_interval(x.node(), {Interval::Precise(UBits(12, 8))},
                    {Interval::Precise(UBits(12, 8))});
  IntervalSetTree x_ist_global =
      pair_interval(x.node(), {Interval::Maximal(8)}, {Interval::Maximal(8)});

  IntervalSetTree add_ten_ist =
      pair_interval(x.node(), {Interval::Precise(UBits(22, 8))},
                    {Interval::Precise(UBits(22, 8))});
  IntervalSetTree add_ten_ist_global =
      pair_interval(x.node(), {Interval::Maximal(8)}, {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist =
      pair_interval(res.node(), {Interval::Maximal(8)}, {Interval::Maximal(8)});

  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(add_ten.node()), add_ten_ist_global);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);
  EXPECT_EQ(consequent_arm_range->GetIntervals(add_ten.node()), add_ten_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

TEST_F(ContextSensitiveRangeQueryEngineTest, EqTuple) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());

  // if (x == (12, 12)) { let a,b = x; return (a + 10, b + 10) } else { x }
  Type* type = p->GetTupleType({p->GetBitsType(8), p->GetBitsType(8)});
  BValue x = fb.Param("x", type);
  BValue cond = fb.Eq(
      x, fb.Literal(Value::Tuple({Value(UBits(12, 8)), Value(UBits(12, 8))})));
  BValue a = fb.TupleIndex(x, 0);
  BValue b = fb.TupleIndex(x, 1);
  BValue add_ten = fb.Tuple({fb.Add(a, fb.Literal(UBits(10, 8))),
                             fb.Add(b, fb.Literal(UBits(10, 8)))});
  BValue res = fb.Select(cond, {x, add_ten});

  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ContextSensitiveRangeQueryEngine engine;

  XLS_ASSERT_OK(engine.Populate(f));

  auto pair_interval =
      [&](Node* n, absl::Span<const Interval> first,
          absl::Span<const Interval> second) -> IntervalSetTree {
    IntervalSet first_interval(8);
    absl::c_for_each(first,
                     [&](const Interval& i) { first_interval.AddInterval(i); });
    first_interval.Normalize();
    IntervalSet second_interval(8);
    absl::c_for_each(
        second, [&](const Interval& i) { second_interval.AddInterval(i); });
    second_interval.Normalize();
    return IntervalSetTree(type, {first_interval, second_interval});
  };
  IntervalSetTree x_ist =
      pair_interval(x.node(), {Interval::Precise(UBits(12, 8))},
                    {Interval::Precise(UBits(12, 8))});
  IntervalSetTree x_ist_global =
      pair_interval(x.node(), {Interval::Maximal(8)}, {Interval::Maximal(8)});

  IntervalSetTree add_ten_ist =
      pair_interval(x.node(), {Interval::Precise(UBits(22, 8))},
                    {Interval::Precise(UBits(22, 8))});
  IntervalSetTree add_ten_ist_global =
      pair_interval(x.node(), {Interval::Maximal(8)}, {Interval::Maximal(8)});
  IntervalSetTree cond_ist = BitsLTT(cond.node(), {Interval::Maximal(1)});
  IntervalSetTree res_ist =
      pair_interval(res.node(), {Interval::Maximal(8)}, {Interval::Maximal(8)});

  auto consequent_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kConsequentArm)});
  auto alternate_arm_range = engine.SpecializeGivenPredicate(
      {PredicateState(res.node()->As<Select>(), kAlternateArm)});

  EXPECT_EQ(engine.GetIntervals(x.node()), x_ist_global);
  EXPECT_EQ(engine.GetIntervals(add_ten.node()), add_ten_ist_global);
  EXPECT_EQ(engine.GetIntervals(cond.node()), cond_ist);
  EXPECT_EQ(engine.GetIntervals(res.node()), res_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(x.node()), x_ist);

  EXPECT_EQ(consequent_arm_range->GetIntervals(add_ten.node()), add_ten_ist);

  EXPECT_EQ(alternate_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kFalse)}));

  EXPECT_EQ(consequent_arm_range->GetIntervals(cond.node()),
            BitsLTT(cond.node(), {Interval::Precise(kTrue)}));
  // NB There is a restricted value for res given cond == 0 but its not clear
  // that we actually want to bother to calculate it. Instead just verify that
  // the result is less than or equal to the unconstrained case.
  EXPECT_THAT(consequent_arm_range->GetIntervals(res.node()),
              AnyOf(Eq(x_ist), Eq(res_ist)));
}

INSTANTIATE_TEST_SUITE_P(Signed, SignedContextSensitiveRangeQueryEngineTest,
                         testing::Bool());

}  // namespace
}  // namespace xls
