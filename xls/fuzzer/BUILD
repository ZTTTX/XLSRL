# Copyright 2020 The XLS Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@xls_pip_deps//:requirements.bzl", "requirement")

# pytype binary, test, library
load("//xls/build_rules:py_proto_library.bzl", "xls_py_proto_library")
load("//xls/fuzzer:build_defs.bzl", "generate_crasher_regression_tests")
# cc_proto_library is used in this file

package(
    default_applicable_licenses = ["//:license"],
    default_visibility = ["//xls:xls_internal"],
    licenses = ["notice"],  # Apache 2.0
)

filegroup(
    name = "x_files",
    srcs = glob(["crashers/*.x"]),
)

py_library(
    name = "cli_helpers",
    srcs = ["cli_helpers.py"],
    srcs_version = "PY3ONLY",
)

py_library(
    name = "run_fuzz",
    srcs = ["run_fuzz.py"],
    data = [
        ":find_failing_input_main",
        ":sample_runner_main",
        ":summarize_ir_main",
        "//xls/tools:ir_minimizer_main",
    ],
    srcs_version = "PY3ONLY",
    tags = ["optonly"],
    deps = [
        ":sample_runner",
        ":sample_summary_py_pb2",
        "//xls/common:gfile",
        "//xls/common:runfiles",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_run_fuzz",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/logging",
    ],
)

py_test(
    name = "run_fuzz_minimizer_test",
    timeout = "long",
    srcs = ["run_fuzz_minimizer_test.py"],
    data = [
        "//xls/tools:parse_ir",
    ],
    python_version = "PY3",
    srcs_version = "PY3",
    tags = ["optonly"],
    deps = [
        ":run_fuzz",
        ":sample_runner",
        "//xls/common:runfiles",
        "//xls/common:test_base",
        "//xls/fuzzer/python:cpp_run_fuzz",
        "//xls/fuzzer/python:cpp_sample",
    ],
)

py_test(
    name = "run_fuzz_function_test",
    srcs = ["run_fuzz_test.py"],
    main = "run_fuzz_test.py",
    python_version = "PY3",
    shard_count = 50,
    srcs_version = "PY3",
    tags = ["optonly"],
    deps = [
        ":run_fuzz",
        "//xls/common:test_base",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/testing:parameterized",
    ],
)

py_test(
    name = "run_fuzz_function_wide_test",
    srcs = ["run_fuzz_test.py"],
    args = ["--wide=true"],
    main = "run_fuzz_test.py",
    python_version = "PY3",
    shard_count = 50,
    srcs_version = "PY3",
    tags = ["optonly"],
    deps = [
        ":run_fuzz",
        "//xls/common:test_base",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/testing:parameterized",
    ],
)

py_test(
    name = "run_fuzz_proc_test",
    srcs = ["run_fuzz_test.py"],
    args = ["--generate_proc=true"],
    main = "run_fuzz_test.py",
    python_version = "PY3",
    shard_count = 50,
    srcs_version = "PY3",
    tags = ["optonly"],
    deps = [
        ":run_fuzz",
        "//xls/common:test_base",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/testing:parameterized",
    ],
)

py_test(
    name = "run_fuzz_function_codegen_test",
    srcs = ["run_fuzz_codegen_test.py"],
    main = "run_fuzz_codegen_test.py",
    python_version = "PY3",
    shard_count = 50,
    srcs_version = "PY3",
    tags = ["optonly"],  # Times out in fastbuild TAP 2020-10-20.
    deps = [
        ":run_fuzz",
        "//xls/common:test_base",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/testing:parameterized",
    ],
)

py_test(
    name = "run_fuzz_proc_codegen_test",
    srcs = ["run_fuzz_codegen_test.py"],
    args = ["--generate_proc=true"],
    main = "run_fuzz_codegen_test.py",
    python_version = "PY3",
    shard_count = 50,
    srcs_version = "PY3",
    # Based on run_fuzz_function_codegen_test, times out in fastbuild TAP 2020-10-20.
    tags = ["optonly"],
    deps = [
        ":run_fuzz",
        "//xls/common:test_base",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/testing:parameterized",
    ],
)

py_test(
    name = "run_fuzz_function_wide_codegen_test",
    srcs = ["run_fuzz_codegen_test.py"],
    args = ["--wide=true"],
    main = "run_fuzz_codegen_test.py",
    python_version = "PY3",
    shard_count = 50,
    srcs_version = "PY3",
    # Based on run_fuzz_function_codegen_test, times out in fastbuild TAP 2020-10-20.
    tags = ["optonly"],
    deps = [
        ":run_fuzz",
        "//xls/common:test_base",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/testing:parameterized",
    ],
)

py_binary(
    name = "run_crasher",
    srcs = ["run_crasher.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":run_fuzz",
        ":sample_runner",
        "//xls/common:gfile",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl:app",
        "@com_google_absl_py//absl/flags",
    ],
)

filegroup(
    name = "run_crasher_sh",
    srcs = ["run_crasher.sh"],
)

cc_library(
    name = "native_sample_runner",
    srcs = ["sample_runner.cc"],
    hdrs = ["sample_runner.h"],
    data = [
        "//xls/dslx/ir_convert:ir_converter_main",
        "//xls/tools:codegen_main",
        "//xls/tools:eval_ir_main",
        "//xls/tools:eval_proc_main",
        "//xls/tools:opt_main",
        "//xls/tools:simulate_module_main",
    ],
    deps = [
        ":cpp_sample",
        ":cpp_sample_runner",
        ":sample_cc_proto",
        ":sample_summary_cc_proto",
        "//xls/common:cpp_check_simulator",
        "//xls/common:cpp_revision",
        "//xls/common:stopwatch",
        "//xls/common:subprocess",
        "//xls/common/file:filesystem",
        "//xls/common/file:get_runfile_path",
        "//xls/common/logging",
        "//xls/common/logging:log_lines",
        "//xls/common/logging:vlog_is_on",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx:channel_direction",
        "//xls/dslx:create_import_data",
        "//xls/dslx:import_data",
        "//xls/dslx:interp_value",
        "//xls/dslx:interp_value_helpers",
        "//xls/dslx:parse_and_typecheck",
        "//xls/dslx:warning_kind",
        "//xls/dslx/bytecode",
        "//xls/dslx/bytecode:bytecode_emitter",
        "//xls/dslx/bytecode:bytecode_interpreter",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/type_system:concrete_type",
        "//xls/dslx/type_system:type_info",
        "//xls/ir:format_preference",
        "//xls/ir:ir_parser",
        "//xls/ir:value",
        "//xls/public:runtime_build_actions",
        "//xls/tools:eval_helpers",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "native_sample_runner_test",
    srcs = ["sample_runner_test.cc"],
    deps = [
        ":cpp_sample",
        ":cpp_sample_runner",
        ":native_sample_runner",
        ":sample_cc_proto",
        "//xls/common:cpp_check_simulator",
        "//xls/common:xls_gunit",
        "//xls/common:xls_gunit_main",
        "//xls/common/file:filesystem",
        "//xls/common/file:temp_directory",
        "//xls/common/logging",
        "//xls/common/status:matchers",
        "//xls/common/status:status_macros",
        "//xls/dslx:interp_value",
        "//xls/dslx:interp_value_helpers",
        "//xls/ir:bits",
        "//xls/ir:ir_parser",
        "//xls/ir:value",
        "//xls/tools:eval_helpers",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

py_library(
    name = "sample_runner",
    srcs = ["sample_runner.py"],
    data = [
        "//xls/dslx/ir_convert:ir_converter_main",
        "//xls/tools:codegen_main",
        "//xls/tools:eval_ir_main",
        "//xls/tools:eval_proc_main",
        "//xls/tools:opt_main",
        "//xls/tools:simulate_module_main",
    ],
    srcs_version = "PY3ONLY",
    deps = [
        ":sample_summary_py_pb2",
        "//xls/common:check_simulator",
        "//xls/common:revision",
        "//xls/common:runfiles",
        "//xls/common:xls_error",
        "//xls/dslx/python:create_import_data",
        "//xls/dslx/python:interp_value",
        "//xls/dslx/python:interpreter",
        "//xls/dslx/python:parse_and_typecheck",
        "//xls/fuzzer/python:cpp_sample",
        "//xls/fuzzer/python:cpp_sample_runner",
        "//xls/ir/python:value",
        "//xls/public/python:runtime_build_actions",
        "//xls/tools/python:eval_helpers",
        "@com_google_absl_py//absl/logging",
    ],
)

py_test(
    name = "sample_runner_test",
    srcs = ["sample_runner_test.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":sample_runner",
        "//xls/common:check_simulator",
        "//xls/common:test_base",
        "//xls/dslx/python:interp_value",
        "//xls/fuzzer/python:cpp_sample",
        "//xls/ir/python:bits",
        "//xls/ir/python:value",
        "@com_google_absl_py//absl/logging",
    ],
)

generate_crasher_regression_tests(
    name = "regression_tests",
    srcs = glob(["crashers/*"]),
    failing = [
        # TODO(https://github.com/google/xls/issues/746): Remove when fixed.
        "crashers/crasher_2022-10-19_a3a3.x",
        # TODO(https://github.com/google/xls/issues/746): Remove when fixed.
        "crashers/crasher_2022-10-19_5531.x",
        # TODO(https://github.com/google/xls/issues/929): Remove when fixed.
        "crashers/crasher_2023-04-09_1f76.x",
        # TODO(https://github.com/google/xls/issues/1055): Remove when fixed.
        "crashers/crasher_2023-07-06_feae.x",
    ],
    prefix = package_name(),
    tags = {
        # Tests which timeout unless run in opt mode.
        "crashers/crasher_2021-03-27_2259.x": [
            "optonly",
        ],

        # Tests which use iverilog and iverilog does not work with sanitizers.
        "crashers/crasher_2022-09-03_4d43.x": [
            "optonly",
        ],
        "crashers/crasher_2022-10-24_40eb.x": [
            "optonly",
        ],
        "crashers/crasher_2022-10-25_133f.x": [
            "optonly",
        ],
        "crashers/multiple-sends.x": [
            "optonly",
        ],
        "crashers/crasher_2022-12-01_9a43.x": [
            "optonly",
        ],
        "crashers/crasher_2023-02-26_ccb9.x": [
            "optonly",
        ],
        "crashers/crasher_2023-03-27_83b8.x": [
            "optonly",
        ],
        "crashers/crasher_2023-06-05_c720.x": [
            "optonly",
        ],
        "crashers/crasher_2023-07-02_c303.x": [
            "optonly",
        ],
    },
)

py_library(
    name = "run_fuzz_multiprocess_lib",
    srcs = ["run_fuzz_multiprocess.py"],
    srcs_version = "PY3ONLY",
    deps = [
        ":run_fuzz",
        ":sample_runner",
        requirement("termcolor"),
        "//xls/common:gfile",
        "//xls/common:multiprocess",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
    ],
)

cc_library(
    name = "cpp_sample",
    srcs = ["sample.cc"],
    hdrs = ["sample.h"],
    deps = [
        ":sample_cc_proto",
        ":scrub_crasher",
        "//xls/common:proto_adaptor_utils",
        "//xls/common/file:filesystem",
        "//xls/common/logging",
        "//xls/common/status:ret_check",
        "//xls/dslx:interp_value",
        "//xls/dslx:interp_value_helpers",
        "//xls/ir:ir_parser",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)

py_binary(
    name = "run_fuzz_multiprocess",
    srcs = ["run_fuzz_multiprocess_main.py"],
    main = "run_fuzz_multiprocess_main.py",
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":cli_helpers",
        ":run_fuzz_multiprocess_lib",
        requirement("psutil"),
        "//xls/common:gfile",
        "//xls/common:multiprocess",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl:app",
        "@com_google_absl_py//absl/flags",
    ],
)

py_binary(
    name = "py_sample_runner_main",
    srcs = ["sample_runner_main.py"],
    main = "sample_runner_main.py",
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":sample_runner",
        "@com_google_absl_py//absl:app",
        "@com_google_absl_py//absl/flags",
    ],
)

cc_binary(
    name = "sample_runner_main",
    srcs = ["sample_runner_main.cc"],
    deps = [
        ":native_sample_runner",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/file:temp_directory",
        "//xls/common/logging",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
    ],
)

py_test(
    name = "sample_test",
    srcs = ["sample_test.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        "//xls/dslx/python:interp_value",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/testing:absltest",
    ],
)

cc_binary(
    name = "summarize_ir_main",
    srcs = ["summarize_ir_main.cc"],
    deps = [
        ":sample_summary_cc_proto",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/file:filesystem",
        "//xls/common/logging",
        "//xls/common/status:status_macros",
        "//xls/ir",
        "//xls/ir:ir_parser",
        "//xls/ir:op",
        "@com_google_absl//absl/flags:flag",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "read_summary_main",
    srcs = ["read_summary_main.cc"],
    deps = [
        ":sample_summary_cc_proto",
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/file:filesystem",
        "//xls/common/logging",
        "//xls/common/status:status_macros",
        "//xls/ir:op",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_binary(
    name = "find_failing_input_main",
    srcs = ["find_failing_input_main.cc"],
    deps = [
        "//xls/common:exit_status",
        "//xls/common:init_xls",
        "//xls/common/file:filesystem",
        "//xls/common/logging",
        "//xls/interpreter:ir_interpreter",
        "//xls/ir",
        "//xls/ir:ir_parser",
        "//xls/ir:value",
        "//xls/ir:value_helpers",
        "//xls/jit:function_jit",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

py_test(
    name = "find_failing_input_main_test",
    srcs = ["find_failing_input_main_test.py"],
    data = [
        ":find_failing_input_main",
    ],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        "//xls/common:runfiles",
        "//xls/common:test_base",
    ],
)

cc_proto_library(
    name = "sample_summary_cc_proto",
    deps = [":sample_summary_proto"],
)

xls_py_proto_library(
    name = "sample_summary_py_pb2",
    srcs = ["sample_summary.proto"],
    internal_deps = [":sample_summary_proto"],
)

proto_library(
    name = "sample_summary_proto",
    srcs = ["sample_summary.proto"],
)

py_test(
    name = "run_fuzz_multiprocess_test",
    timeout = "long",
    srcs = ["run_fuzz_multiprocess_test.py"],
    data = [":run_fuzz_multiprocess"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        "//xls/common:runfiles",
        "//xls/common:test_base",
    ],
)

py_test(
    name = "fuzz_coverage_test",
    srcs = ["fuzz_coverage_test.py"],
    data = [":run_fuzz_multiprocess"],
    python_version = "PY3",
    shard_count = 3,
    srcs_version = "PY3",
    tags = [
        "manual",
        "notap",
    ],
    deps = [
        ":sample_summary_py_pb2",
        "//xls/common:runfiles",
        "//xls/common:test_base",
        "//xls/ir/python:op",
        "@com_google_absl_py//absl/logging",
        "@com_google_absl_py//absl/testing:absltest",
        "@com_google_protobuf//:protobuf_python",
    ],
)

cc_library(
    name = "ast_generator",
    srcs = ["ast_generator.cc"],
    hdrs = ["ast_generator.h"],
    deps = [
        ":value_generator",
        "//xls/common:casts",
        "//xls/common:symbolized_stacktrace",
        "//xls/common:test_macros",
        "//xls/common/logging",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx:channel_direction",
        "//xls/dslx:interp_value",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:ast_cloner",
        "//xls/dslx/frontend:scanner",
        "//xls/ir:bits",
        "//xls/ir:bits_ops",
        "//xls/ir:format_preference",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "ast_generator_test",
    srcs = ["ast_generator_test.cc"],
    deps = [
        ":ast_generator",
        ":value_generator",
        "//xls/common:xls_gunit",
        "//xls/common:xls_gunit_main",
        "//xls/common/logging",
        "//xls/common/logging:log_lines",
        "//xls/common/status:matchers",
        "//xls/common/status:status_macros",
        "//xls/dslx:command_line_utils",
        "//xls/dslx:create_import_data",
        "//xls/dslx:parse_and_typecheck",
        "//xls/dslx/frontend:ast",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "cpp_sample_generator",
    srcs = ["sample_generator.cc"],
    hdrs = ["sample_generator.h"],
    deps = [
        ":ast_generator",
        ":cpp_sample",
        ":sample_cc_proto",
        ":value_generator",
        "//xls/common/logging",
        "//xls/common/status:ret_check",
        "//xls/common/status:status_macros",
        "//xls/dslx:channel_direction",
        "//xls/dslx:create_import_data",
        "//xls/dslx:import_data",
        "//xls/dslx:interp_value",
        "//xls/dslx:parse_and_typecheck",
        "//xls/dslx:warning_kind",
        "//xls/dslx/frontend:ast",
        "//xls/dslx/frontend:ast_node",
        "//xls/dslx/frontend:ast_utils",
        "//xls/dslx/type_system:concrete_type",
        "//xls/dslx/type_system:type_info",
        "//xls/dslx/type_system:unwrap_meta_type",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "dslx_mutator",
    srcs = ["dslx_mutator.cc"],
    hdrs = ["dslx_mutator.h"],
    deps = [
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:scanner",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/random:bit_gen_ref",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_test(
    name = "dslx_mutator_test",
    srcs = ["dslx_mutator_test.cc"],
    deps = [
        ":dslx_mutator",
        "//xls/common:xls_gunit",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/random:mock_distributions",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "scrub_crasher",
    srcs = ["scrub_crasher.cc"],
    hdrs = ["scrub_crasher.h"],
    deps = [
        "@com_github_google_re2//:re2",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "sample_generator_test",
    srcs = ["sample_generator_test.cc"],
    deps = [
        ":cpp_sample_generator",
        ":value_generator",
        "//xls/common:xls_gunit",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/dslx/type_system:concrete_type",
    ],
)

# By default (no flags) the integration test generates only 10 function samples which
# serves as a smoke test for the health of the test itself.
py_test(
    name = "fuzz_integration_test",
    srcs = ["fuzz_integration_test.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":cli_helpers",
        ":run_fuzz",
        ":sample_runner",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/logging",
        "@com_google_absl_py//absl/testing:absltest",
    ],
)

# Integration test generates 10 proc samples.
py_test(
    name = "fuzz_integration_proc_test",
    srcs = ["fuzz_integration_test.py"],
    args = ["--generate_proc=true"],
    main = "fuzz_integration_test.py",
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":cli_helpers",
        ":run_fuzz",
        ":sample_runner",
        "//xls/fuzzer/python:cpp_ast_generator",
        "//xls/fuzzer/python:cpp_sample",
        "@com_google_absl_py//absl/flags",
        "@com_google_absl_py//absl/logging",
        "@com_google_absl_py//absl/testing:absltest",
    ],
)

cc_library(
    name = "value_generator",
    srcs = ["value_generator.cc"],
    hdrs = ["value_generator.h"],
    deps = [
        "//xls/common:visitor",
        "//xls/common/status:ret_check",
        "//xls/dslx:interp_value",
        "//xls/dslx/type_system:concrete_type",
        "//xls/dslx/type_system:unwrap_meta_type",
        "//xls/ir:bits",
        "//xls/ir:bits_ops",
        "//xls/ir:number_parser",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "value_generator_test",
    srcs = ["value_generator_test.cc"],
    deps = [
        ":value_generator",
        "//xls/common:xls_gunit",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/common/status:status_macros",
        "//xls/dslx/frontend:ast",
    ],
)

cc_test(
    name = "sample_cc_test",
    srcs = ["sample_cc_test.cc"],
    deps = [
        ":cpp_sample",
        "//xls/common:xls_gunit",
        "//xls/common:xls_gunit_main",
        "//xls/common/status:matchers",
        "//xls/dslx:interp_value",
    ],
)

cc_library(
    name = "cpp_run_fuzz",
    srcs = ["cpp_run_fuzz.cc"],
    hdrs = ["cpp_run_fuzz.h"],
    deps = [
        ":cpp_sample",
        "//xls/common:subprocess",
        "//xls/common/file:filesystem",
        "//xls/common/file:get_runfile_path",
        "//xls/common/logging:log_lines",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "cpp_sample_runner",
    srcs = ["cpp_sample_runner.cc"],
    hdrs = ["cpp_sample_runner.h"],
    deps = [
        "//xls/dslx:interp_value",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_proto_library(
    name = "sample_cc_proto",
    deps = [":sample_proto"],
)

xls_py_proto_library(
    name = "sample_py_pb2",
    srcs = ["sample.proto"],
    internal_deps = [":sample_proto"],
)

proto_library(
    name = "sample_proto",
    srcs = ["sample.proto"],
)
