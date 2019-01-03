package(default_visibility = ["//visibility:public"])

cc_test(
    name = "test_common",
    srcs = [
        "test_common.cc",
    ],
    deps = [
        "iota_api_mock",
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
        "//serializer:test_define",
        "@com_google_googletest//:gtest_main",
        "@entangled//cclient:api",
    ],
)
