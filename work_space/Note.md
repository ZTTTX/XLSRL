# To make with tests

bazel test -c opt -- //xls/...

# To make without tests

bazel build -c opt //xls/...

# To run my own pass

./bazel-bin/xls/tools/RL_main ./work_space/test.opt.ir ./work_space/instruct.json ./work_space/my_test.opt.ir --clock_period_ps=1000 --delay_model=sky130 > ./work_space/my_test_schedule.txt

# To Run with python generated json file:

./bazel-bin/xls/tools/RL_main ./work_space/test.opt.ir ./third_party/graph_extraction/test_ins.json ./work_space/my_test.opt.ir --clock_period_ps=1000 --delay_model=sky130 > ./work_space/my_test_schedule.txt


## To run unit tests with python generated instruction files

./bazel-bin/xls/tools/RL_main ./work_space/all_unit_test.ir ./third_party/graph_extraction/test_ins.json ./work_space/all_unit_test_output.ir --clock_period_ps=1000 --delay_model=sky130 > ./work_space/my_test_schedule.txt

# ==================================================

# Run stuff

./bazel-bin/xls/contrib/xlscc/xlscc ./work_space/test.cc > ./work_space/test.ir

./bazel-bin/xls/tools/opt_main ./work_space/test.ir > ./work_space/test.opt.ir

./bazel-bin/xls/tools/codegen_main ./work_space/test.opt.ir \
  --generator=combinational \
  --delay_model="unit" \
  --output_verilog_path=./work_space/test.v \
  --module_name=xls_test \
  --top=add3


 ./bazel-bin/xls/tools/codegen_main ./work_space/test.opt.ir \
  --generator=pipeline \
  --delay_model="asap7" \
  --output_verilog_path=./work_space/test.v \
  --module_name=xls_test_unroll \
  --top=test_unroll \
  --pipeline_stages=5 \
  --flop_inputs=true \
  --flop_outputs=true

## Scheduling Probelm

bazel run -c opt //xls/tools:benchmark_main -- $PWD/bazel-bin/xls/examples/crc32.opt.ir --clock_period_ps=1000 --delay_model=sky130

## IR Visualization tool

bazel run -c opt //xls/visualization/ir_viz:app -- --delay_model=unit


# To get ir-python graph(Deprecated)

bazel run //third_party/graph_extraction:graph_extraction -- /home/miao/xls/work_space/my_test.opt.ir test_unroll

# Unit tests(Deprecated)

## Commutativity

./bazel-bin/xls/tools/RL_main ./work_space/UnitTests/CommutativityUnitTest.opt.ir ./work_space/UnitTests/CommutativityUnitTest.json > ./work_space/UnitTests/CommutativityUnitTest.rewrite.ir

## Associativity

./bazel-bin/xls/tools/RL_main ./work_space/UnitTests/AssociativityUnitTest.opt.ir ./work_space/UnitTests/AssociativityUnitTest.json > ./work_space/UnitTests/AssociativityUnitTest.rewrite.ir

## DistributeMultOverAdd

./bazel-bin/xls/tools/RL_main ./work_space/UnitTests/DistributeMultOverAddUnitTest.opt.ir ./work_space/UnitTests/DistributeMultOverAddUnitTest.json > ./work_space/UnitTests/DistributeMultOverAddUnitTest.rewrite.ir