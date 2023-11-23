# X to IR


## sha256

./bazel-bin/xls/dslx/ir_convert/ir_converter_main --top=main ./work_space/Sha256/sha256.x > ./work_space/Sha256/sha256.ir

./bazel-bin/xls/tools/opt_main ./work_space/Sha256/sha256.ir > ./work_space/Sha256/sha256.opt.ir

##

./bazel-bin/xls/tools/RL_main ./work_space/Sha256/sha256.opt.ir ./work_space/Sha256/sha256.opt.json /work_space/Sha256/sha256.opt_substitution.ir

## clear name

./bazel-bin/xls/tools/UnifyName ./work_space/Sha256/sha256.opt.ir ./work_space/Sha256/sha256.unify.ir

## alder32
./bazel-bin/xls/dslx/ir_convert/ir_converter_main --top=main ./work_space/adler32/adler32.x > ./work_space/adler32/adler32.ir

./bazel-bin/xls/tools/opt_main ./work_space/adler32/adler32.ir > ./work_space/adler32/adler32.opt.ir


## dot product (not working)
./bazel-bin/xls/dslx/ir_convert/ir_converter_main --top=dot_product_float32 ./work_space/dot_product/dot_product.x > ./work_space/dot_product/dot_product.ir

./bazel-bin/xls/tools/opt_main ./work_space/dot_product/dot_product.ir > ./work_space/dot_product/dot_product.opt.ir

## bitnoic (does not work)

./bazel-bin/xls/dslx/ir_convert/ir_converter_main --top=bitonic_sort ./work_space/bitonic_sort/bitonic_sort.x > ./work_space/bitonic_sort/bitonic_sort.ir

./bazel-bin/xls/tools/opt_main ./work_space/bitonic_sort/bitonic_sort.ir > ./work_space/bitonic_sort/bitonic_sort.opt.ir


## MatMul (does not work)

./bazel-bin/xls/tools/opt_main --top=tile_0_0 --inline_procs ./work_space/MatMul/matmul_4x4.ir > ./work_space/MatMul/matmul_4x4.opt.ir