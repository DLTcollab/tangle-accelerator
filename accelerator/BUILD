load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
load("@io_bazel_rules_docker//container:container.bzl", "container_push")

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
    }),
    deps = [
        ":ta_config",
        "//common:ta_errors",
        "//connectivity/http",
        "@entangled//utils/handles:signal",
    ] + select({
        "//connectivity/mqtt:mqtt_enable": [
            "//connectivity/mqtt",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "ta_config",
    srcs = ["config.c"],
    hdrs = ["config.h"],
    defines = select({
        "//connectivity/mqtt:mqtt_enable": [
            "MQTT_ENABLE",
        ],
        "//conditions:default": [],
    }) + select({
        "//storage:db_enable": ["DB_ENABLE"],
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

cc_library(
    name = "cli_info",
    srcs = ["cli_info.h"],
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
