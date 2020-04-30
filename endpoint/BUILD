cc_library(
    name = "endpoint",
    srcs = ["endpoint.c"],
    hdrs = ["endpoint.h"],
    defines = [
        "TA_HOST=\"tangle-accel.biilabs.io\"",
        "TA_PORT=\"443\"",
        "TA_SSL_SEED=\"nonce\"",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//common:defined_error",
        "//utils:cipher",
        "//utils:https",
        "//utils:text_serializer",
        "//utils:tryte_byte_conv",
    ],
)

cc_test(
    name = "test_endpoint",
    srcs = [
        "test_endpoint.c",
    ],
    deps = [
        ":endpoint",
        "//common",
        "//tests:test_define",
    ],
)
