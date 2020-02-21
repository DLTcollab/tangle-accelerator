load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
load("@io_bazel_rules_docker//container:container.bzl", "container_image")

cc_binary(
    name = "accelerator",
    srcs = select({
        ":mqtt_enable": [
            "mqtt_main.c",
        ],
        "//conditions:default": ["main.c"],
    }),
    copts = select({
        ":debug_mode": ["-g"],
        ":profile_mode": [
            "-pg",
        ],
        "//conditions:default": [],
    }),
    deps = [
        ":ta_config",
        "//connectivity/http",
        "@entangled//utils/handles:signal",
    ] + select({
        ":mqtt_enable": [
            "//connectivity/mqtt",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "ta_config",
    srcs = ["config.c"],
    hdrs = ["config.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":cli_info",
        "//accelerator/core:pow",
        "//common",
        ":build_option",
        "//utils/cache",
        "@entangled//cclient/api",
        "@yaml",
    ] + select({
        ":db_enable": ["//storage"],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "cli_info",
    srcs = ["cli_info.h"],
)

cc_image(
    name = "docker_base_image",
    binary = ":accelerator",
)

container_image(
    name = "docker_image",
    base = ":docker_base_image",
    repository = "dltcollab",
    stamp = True,
)

cc_library(
    name = "build_option",
    defines = select({
        ":mqtt_enable": ["MQTT_ENABLE"],
        "//conditions:default": [],
    }) + select({
        ":db_enable": ["DB_ENABLE"],
        "//conditions:default": [],
    }) + select({
        ":debug_mode": [],
        "//conditions:default": ["NDEBUG"],
    }),
    visibility = ["//visibility:public"],
)

config_setting(
    name = "debug_mode",
    values = {
        "define": "build_type=debug",
    },
)

config_setting(
    name = "profile_mode",
    values = {
        "define": "build_type=profile",
    },
)

config_setting(
    name = "mqtt_enable",
    values = {"define": "mqtt=enable"},
)

config_setting(
    name = "db_enable",
    values = {"define": "db=enable"},
)
