package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
load("@io_bazel_rules_docker//container:container.bzl", "container_push")

cc_binary(
    name = "accelerator",
    srcs = ["server.cc"],
    copts = ["-DLOGGER_ENABLE"] + select({
        ":DEBUG_MODE": ["-g"],
        ":PROFILING_MODE": [
            "-DNDEBUG",
            "-pg",
        ],
        "//conditions:default": ["-DNDEBUG"],
    }),
    deps = [
        ":apis",
        ":proxy_apis",
        ":ta_config",
        ":ta_errors",
        "@served",
    ],
)

cc_binary(
    name = "accelerator_microhttpd",
    srcs = ["main.c"],
    copts = ["-DLOGGER_ENABLE"] + select({
        ":DEBUG_MODE": ["-g"],
        ":PROFILING_MODE": [
            "-DNDEBUG",
            "-pg",
        ],
        "//conditions:default": ["-DNDEBUG"],
    }),
    deps = [
        ":http",
        ":ta_config",
        ":ta_errors",
        "@entangled//utils/handles:signal",
    ],
)

cc_binary(
    name = "accelerator_mqtt",
    srcs = [
        "config.c",
        "config.h",
        "conn_mqtt.c",
    ],
    copts = [
        "-DLOGGER_ENABLE",
        "-DENABLE_MQTT",
    ] + select({
        ":DEBUG_MODE": ["-g"],
        ":PROFILING_MODE": [
            "-DNDEBUG",
            "-pg",
        ],
        "//conditions:default": ["-DNDEBUG"],
    }),
    deps = [
        ":message",
        ":ta_config",
        ":ta_errors",
        "//connectivity/mqtt:mqtt_utils",
        "//utils:cache",
        "//utils:pow",
        "@entangled//cclient/api",
        "@entangled//utils/handles:signal",
    ],
)

cc_image(
    name = "docker",
    binary = ":accelerator",
)

container_push(
    name = "push_docker",
    format = "Docker",
    image = ":docker",
    registry = "index.docker.io",
    repository = "dltcollab/tangle-accelerator",
    tag = "latest",
)

cc_library(
    name = "apis",
    srcs = ["apis.c"],
    hdrs = ["apis.h"],
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
    deps = [
        ":common_core",
        ":ta_errors",
        "//map:mode",
        "//serializer",
        "@entangled//common/model:bundle",
        "@entangled//common/trinary:trit_tryte",
        "@entangled//mam/api",
    ],
)

cc_library(
    name = "proxy_apis",
    srcs = ["proxy_apis.c"],
    hdrs = ["proxy_apis.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ta_errors",
        "//serializer",
        "//utils:hash_algo_djb2",
        "//utils:ta_logger",
        "@entangled//cclient/api",
        "@entangled//cclient/request:requests",
        "@entangled//cclient/response:responses",
    ],
)

cc_library(
    name = "http",
    srcs = ["http.c"],
    hdrs = ["http.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":apis",
        ":proxy_apis",
        ":ta_config",
        ":ta_errors",
        "@libmicrohttpd",
    ],
)

cc_library(
    name = "common_core",
    srcs = ["common_core.c"],
    hdrs = ["common_core.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ta_config",
        ":ta_errors",
        "//request",
        "//response",
        "//utils:bundle_array",
        "//utils:timer",
        "@com_github_uthash//:uthash",
        "@entangled//cclient/api",
        "@entangled//cclient/serialization:serializer",
        "@entangled//cclient/serialization:serializer_json",
        "@entangled//common/model:bundle",
        "@entangled//utils:time",
    ],
)

cc_library(
    name = "ta_config",
    srcs = ["config.c"],
    hdrs = ["config.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":message",
        ":ta_errors",
        "//utils:cache",
        "//utils:pow",
        "//utils:ta_logger",
        "@entangled//cclient/api",
        "@yaml",
    ],
)

config_setting(
    name = "DEBUG_MODE",
    values = {
        "define": "build_type=debug",
    },
)

config_setting(
    name = "PROFILING_MODE",
    values = {
        "define": "build_type=profiling",
    },
)

cc_library(
    name = "ta_errors",
    hdrs = ["errors.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "message",
    srcs = ["message.c"],
    hdrs = ["message.h"],
)
