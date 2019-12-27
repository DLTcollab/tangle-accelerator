package(default_visibility = ["//visibility:public"])

cc_test(
    name = "test_core",
    srcs = [
        "test_core.cc",
    ],
    deps = [
        ":iota_api_mock",
        "//accelerator/core",
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
        "//accelerator/core",
        "@com_google_googletest//:gtest_main",
        "@entangled//cclient/api",
    ],
)

cc_test(
    name = "driver",
    srcs = [
        "driver.c",
    ],
    copts = select({
        "//connectivity/mqtt:mqtt_enable": [
            "-DMQTT_ENABLE",
        ],
        "//conditions:default": [],
    }) + select({
        "//storage:db_enable": ["-DDB_ENABLE"],
        "//conditions:default": [],
    }),
    deps = [
        ":test_define",
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
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
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
    ],
)

cc_test(
    name = "test_cache",
    srcs = [
        "test_cache.c",
    ],
    deps = [
        ":test_define",
        "//utils/cache",
        "@unity",
    ],
)

cc_test(
    name = "test_serializer",
    srcs = [
        "test_serializer.c",
    ],
    copts = select({
        "//connectivity/mqtt:mqtt_enable": ["-DMQTT_ENABLE"],
        "//conditions:default": [],
    }),
    deps = [
        ":test_define",
        "//accelerator/core/serializer",
    ],
)

cc_test(
    name = "test_pow",
    srcs = [
        "test_pow.c",
    ],
    deps = [
        ":test_define",
        "//accelerator/core:pow",
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

cc_test(
    name = "test_scylladb",
    srcs = select({
        "//storage:db_enable": ["test_scylladb.c"],
        "//conditions:default": ["empty_test.c"],
    }),
    deps = [
        ":test_define",
    ] + select({
        "//storage:db_enable": ["//storage"],
        "//conditions:default": [],
    }),
)

cc_test(
    name = "test_timer",
    srcs = [
        "test_timer.c",
    ],
    deps = [
        ":test_define",
        "//utils:timer",
    ],
)
