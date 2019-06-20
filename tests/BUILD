package(default_visibility = ["//visibility:public"])

cc_test(
    name = "test_common",
    srcs = [
        "test_common.cc",
    ],
    deps = [
        ":iota_api_mock",
        "//accelerator:common_core",
    ],
)

cc_library(
    name = "iota_api_mock",
    srcs = [
        "iota_api_mock.cc",
    ],
    hdrs = ["iota_api_mock.hh"],
    deps = [
        ":test_define",
        "//accelerator:common_core",
        "@com_google_googletest//:gtest_main",
        "@entangled//cclient/api",
    ],
)

cc_test(
    name = "driver",
    srcs = [
        "driver.c",
    ],
    deps = [
        ":test_define",
        "//accelerator:apis",
    ],
)

cc_binary(
    name = "driver_stat",
    srcs = [
        "driver.c",
    ],
    copts = ["-DENABLE_STAT"],
    deps = [
        ":test_define",
        "//accelerator:apis",
    ],
)

cc_test(
    name = "test_cache",
    srcs = [
        "test_cache.c",
    ],
    deps = [
        ":test_define",
        "//utils:cache",
        "@unity",
    ],
)

cc_test(
    name = "test_serializer",
    srcs = [
        "test_serializer.c",
    ],
    deps = [
        ":test_define",
        "//serializer",
    ],
)

cc_test(
    name = "test_pow",
    srcs = [
        "test_pow.c",
    ],
    deps = [
        ":test_define",
        "//utils:pow",
    ],
)

cc_test(
    name = "test_map_mode",
    srcs = [
        "test_map_mode.c",
    ],
    deps = [
        ":test_define",
        "//map:mode",
    ],
)

cc_library(
    name = "test_define",
    hdrs = ["test_define.h"],
    deps = [
        "//accelerator:ta_config",
        "@unity",
    ],
)
