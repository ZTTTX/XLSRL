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

# Scheduling Probelm

bazel run -c opt //xls/tools:benchmark_main -- $PWD/bazel-bin/xls/examples/crc32.opt.ir --clock_period_ps=1000 --delay_model=sky130

# IR Visualization tool

bazel run -c opt //xls/visualization/ir_viz:app -- --delay_model=unit