cc_library(
    name = "endpoint_core",
    srcs = ["endpoint_core.c"],
    hdrs = ["endpoint_core.h"],
    defines = [
        "ENDPOINT_HOST=\"localhost\"",
        "ENDPOINT_PORT=\"443\"",
        "ENDPOINT_SSL_SEED=\"nonce\"",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "//utils:cipher",
        "//utils:https",
        "//utils:text_serializer",
        "//utils:tryte_byte_conv",
    ],
)

cc_test(
    name = "test_endpoint_core",
    srcs = [
        "test_endpoint_core.c",
    ],
    deps = [
        ":endpoint_core",
        "//common:ta_errors",
        "//common:ta_logger",
        "//tests:test_define",
    ],
)

cc_binary(
    name = "libendpoint.so",
    srcs = [
        "endpoint_core.c",
        "endpoint_core.h",
    ],
    defines = [
        "TA_HOST=\"localhost\"",
        "TA_PORT=\"443\"",
        "TA_SSL_SEED=\"nonce\"",
    ],
    linkshared = True,
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "//utils:cipher",
        "//utils:https",
        "//utils:text_serializer",
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
