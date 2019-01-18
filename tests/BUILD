package(default_visibility = ["//visibility:public"])

cc_test(
    name = "test_common",
    srcs = [
        "test_common.cc",
    ],
    deps = [
        ":iota_api_mock",
        "//:common_core",
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
        "@com_google_googletest//:gtest_main",
        "@entangled//cclient:api",
    ],
)

cc_binary(
    name = "test_cache",
    srcs = [
        "test_cache.c",
    ],
    deps = [
        ":test_define",
        "//cache",
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

cc_library(
    name = "test_define",
    hdrs = ["test_define.h"],
    deps = [
        "@entangled//cclient/types",
        "@unity",
    ],
)
