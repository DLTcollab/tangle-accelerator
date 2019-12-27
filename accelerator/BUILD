package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
load("@io_bazel_rules_docker//container:container.bzl", "container_push")

cc_binary(
    name = "accelerator",
    srcs = select({
        "//connectivity/mqtt:mqtt_enable": [
            "mqtt_main.c",
        ],
        "//conditions:default": ["main.c"],
    }),
    copts = [
        "-DLOGGER_ENABLE",
    ] + select({
        ":DEBUG_MODE": ["-g"],
        ":PROFILING_MODE": [
            "-DNDEBUG",
            "-pg",
        ],
        "//conditions:default": ["-DNDEBUG"],
    }) + select({
        "//connectivity/mqtt:mqtt_enable": [
            "-DMQTT_ENABLE",
        ],
        "//conditions:default": [],
    }) + select({
        "//storage:db_enable": ["-DDB_ENABLE"],
        "//conditions:default": [],
    }),
    deps = [
        ":ta_config",
        "//common:ta_errors",
        "//connectivity/http",
        "@entangled//utils/handles:signal",
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
    ] + select({
        "//connectivity/mqtt:mqtt_enable": [
            "//connectivity/mqtt:mqtt_utils",
        ],
        "//conditions:default": [],
    }),
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
    name = "ta_config",
    srcs = ["config.c"],
    hdrs = ["config.h"],
    copts = select({
        "//connectivity/mqtt:mqtt_enable": [
            "-DMQTT_ENABLE",
        ],
        "//conditions:default": [],
    }) + select({
        "//storage:db_enable": ["-DDB_ENABLE"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        ":cli_info",
        "//accelerator/core:pow",
        "//common:ta_errors",
        "//common:ta_logger",
        "//utils/cache",
        "@entangled//cclient/api",
        "@yaml",
    ] + select({
        "//storage:db_enable": ["//storage"],
        "//conditions:default": [],
    }),
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
    name = "cli_info",
    srcs = ["cli_info.h"],
)
