# Description:
# TensorFlow Lite Example TFLiteOverGrpc.

load("//tensorflow/lite:build_def.bzl", "tflite_linkopts")
#load("//tensorflow/core/platform/default:build_config.bzl", "tf_proto_library")
load("//tensorflow/core/platform:build_config.bzl", "tf_proto_library")
load("//tensorflow:tensorflow.bzl", "tf_grpc_cc_dependencies")

# For platform specific build config
load(
    "//tensorflow/core/platform:build_config.bzl",
    "tf_protos_profiler_service",
)

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],
)

exports_files(glob([
    "testdata/*.bmp",
]))

tf_proto_library(
    name = "TFLiteGrpc_proto",
    srcs = ["TFLiteGrpc.proto"],
    cc_api_version = 2,
    make_default_target_header_only = True,
)

cc_binary(
    name = "TFLiteGrpc_server",
    srcs = [
        "TFLiteGrpc_server.cc"
    ],
    linkopts = tflite_linkopts() + select({
        "//tensorflow:android": [
            "-pie",  # Android 5.0 and later supports only PIE
            "-lm",  # some builtin ops, e.g., tanh, need -lm
            "-Wl,-rpath=/data/local/tmp",  # for hexagon delegate
        ],
        "//conditions:default": [],
    }),
    deps = [
        ":tflite_grpc",
        ":TFLiteGrpc_proto_cc",
        "//tensorflow/lite:framework",
        "//tensorflow/lite:string_util",
        "//tensorflow/lite/c:common",
        "//tensorflow/lite/kernels:builtin_ops",
        "//tensorflow/lite/profiling:profiler",
        "//tensorflow/lite/tools:command_line_flags",
        "//tensorflow/lite/tools:tool_params",
        "//tensorflow/lite/tools/delegates:delegate_provider_hdr",
        "//tensorflow/lite/tools/delegates:tflite_execution_providers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ]+ tf_grpc_cc_dependencies(),
)

cc_binary(
    name = "TFLiteGrpc_client",
    srcs = [
        "TFLiteGrpc_client.cc"
    ],
    linkopts = tflite_linkopts() + select({
        "//tensorflow:android": [
            "-pie",  # Android 5.0 and later supports only PIE
            "-lm",  # some builtin ops, e.g., tanh, need -lm
            "-Wl,-rpath=/data/local/tmp",  # for hexagon delegate
        ],
        "//conditions:default": [],
    }),
    deps = [
        ":tflite_grpc",
        ":TFLiteGrpc_proto_cc",
        "//tensorflow/lite:framework",
        "//tensorflow/lite:string_util",
        "//tensorflow/lite/c:common",
        "//tensorflow/lite/kernels:builtin_ops",
        "//tensorflow/lite/profiling:profiler",
        "//tensorflow/lite/tools:command_line_flags",
        "//tensorflow/lite/tools:tool_params",
        "//tensorflow/lite/tools/delegates:delegate_provider_hdr",
        "//tensorflow/lite/tools/delegates:tflite_execution_providers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ] + select({
        "//tensorflow:android": [
            "//tensorflow/lite/delegates/gpu:delegate",
            "//tensorflow/lite/delegates/hexagon:hexagon_delegate",
        ],
        "//tensorflow:android_arm64": [
            "//tensorflow/lite/delegates/gpu:delegate",
            "//tensorflow/lite/delegates/hexagon:hexagon_delegate",
        ],
        "//conditions:default": [],
    }) + tf_grpc_cc_dependencies(),
)

cc_library(
    name = "tflite_grpc",
    srcs = [
    "sequential_file_reader.cc",
    "sequential_file_writer.cc",
    "utils.cc",
    ],
    hdrs = [
      "file_reader_into_stream.h",
      "sequential_file_reader.h",
      "sequential_file_writer.h",
      "utils.h",
    ],
    deps = [
        "//tensorflow/lite:builtin_op_data",
        "//tensorflow/lite:framework",
        "//tensorflow/lite:string",
        "//tensorflow/lite:string_util",
        "//tensorflow/lite/kernels:builtin_ops",
        "//tensorflow/lite/schema:schema_fbs",
    ]+ tf_grpc_cc_dependencies(),
)

