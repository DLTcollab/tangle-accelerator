cc_test(
    name = "test_cache",
    srcs = [
        "test_cache.c",
    ],
    linkopts = ["-luuid"],
    deps = [
        "//tests:logger_lib",
        "//tests:test_define",
        "//utils/cache",
        "@unity",
    ],
)

cc_test(
    name = "test_serializer",
    srcs = [
        "test_serializer.c",
    ],
    deps = [
        "//accelerator/core/serializer",
        "//tests:common",
        "//tests:logger_lib",
        "//tests:test_define",
    ],
)

cc_test(
    name = "test_pow",
    srcs = [
        "test_pow.c",
    ],
    deps = [
        "//accelerator/core:pow",
        "//tests:logger_lib",
        "//tests:test_define",
    ],
)

cc_test(
    name = "test_scylladb",
    srcs = [
        "test_scylladb.c",
    ],
    deps = [
        "//storage",
        "//tests:logger_lib",
        "//tests:test_define",
    ],
)

cc_test(
    name = "test_timer",
    srcs = [
        "test_timer.c",
    ],
    deps = [
        "//connectivity/http",
        "//tests:logger_lib",
        "//tests:test_define",
        "//utils:timer",
    ],
)

cc_test(
    name = "test_tryte_byte_conv",
    srcs = [
        "test_tryte_byte_conv.c",
    ],
    deps = [
        "//tests:logger_lib",
        "//tests:test_define",
        "//utils:tryte_byte_conv",
    ],
)

cc_test(
    name = "test_crypto",
    srcs = ["test_crypto.c"],
    deps = [
        "//crypto:ecdh",
        "//tests:logger_lib",
        "//tests:test_define",
        "@mbedtls",
    ],
)
