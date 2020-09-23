load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")
load("@io_bazel_rules_docker//container:container.bzl", "container_image")
load("@io_bazel_rules_docker//docker/package_managers:download_pkgs.bzl", "download_pkgs")
load("@io_bazel_rules_docker//docker/package_managers:install_pkgs.bzl", "install_pkgs")

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
        ":ta_cli",
        "//connectivity/http",
        "//accelerator/core:periodical_task",
        "@org_iota_common//utils/handles:signal",
    ] + select({
        ":mqtt_enable": [
            "//connectivity/mqtt",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "ta_cli",
    srcs = ["runtime_cli.c"],
    hdrs = ["runtime_cli.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ta_config",
        "//common:ta_logger",
        "//utils/cache",
        "@org_iota_common//utils:macros",
    ],
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
        "//utils:timer",
        ":build_option",
        "//utils/cache",
        "//utils:cpuinfo",
        "@iota.c//cclient/api:api",
        "@iota.c//cclient:service",
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

download_pkgs(
    name = "docker_runtime_pkgs",
    image_tar = "@ubuntu1804//image",
    packages = [
        "uuid-runtime",
    ],
)

install_pkgs(
    name = "docker_runtime_image",
    image_tar = "@ubuntu1804//image",
    installables_tar = ":docker_runtime_pkgs.tar",
    installation_cleanup_commands = "rm -rf /var/lib/apt/lists/*",
    output_image_name = "docker_runtime_image",
)

cc_image(
    name = "docker_base_image",
    base = ":docker_runtime_image",
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
