package(default_visibility = ["//visibility:public"])

cc_library(
    name = "endpoint_core",
    srcs = ["endpoint_core.c"],
    hdrs = [
        "build/endpoint_generated.h",
        "endpoint_core.h",
    ],
    copts = [
        "-Ibuild",
    ],
    defines = [
        "EP_TA_HOST=localhost",
        "EP_TA_PORT=8000",
        "EP_SSL_SEED=nonce",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "//endpoint:cipher",
        "//endpoint:https",
        "//endpoint:text_serializer",
        "//utils:tryte_byte_conv",
        "@flatcc_0_6_0",
    ],
)

cc_binary(
    name = "libendpoint.so",
    srcs = [
        "endpoint_core.c",
        "endpoint_core.h",
    ],
    defines = [
        "EP_TA_HOST=localhost",
        "EP_TA_PORT=8000",
        "EP_SSL_SEED=nonce",
    ],
    linkshared = True,
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "//endpoint:cipher",
        "//endpoint:https",
        "//endpoint:text_serializer",
        "//utils:tryte_byte_conv",
    ],
)

platform(
    name = "linux_arm",
    constraint_values = [
        "@platforms//os:linux",
        # FIXME: The generic CPU name will be replaced in the future
        # ref: https://github.com/bazelbuild/platforms/blob/master/cpu/BUILD
        "@platforms//cpu:arm",
    ],
)

cc_library(
    name = "cipher",
    srcs = ["cipher.c"],
    hdrs = ["cipher.h"],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "@mbedtls_2_16_6",
    ],
)

cc_library(
    name = "https",
    srcs = ["https.c"],
    hdrs = ["https.h"],
    defines = select({
        ":https_enable": ["ENDPOINT_HTTPS"],
        "//conditions:default": [],
    }),
    deps = [
        "//common:ta_errors",
        "//endpoint/connectivity:conn_http",
    ],
)

config_setting(
    name = "https_enable",
    values = {"define": "https=enable"},
)

cc_library(
    name = "text_serializer",
    srcs = ["text_serializer.c"],
    hdrs = ["text_serializer.h"],
    deps = [
        ":cipher",
        "//common:ta_errors",
        "//common:ta_logger",
    ],
)
