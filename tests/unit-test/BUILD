cc_test(
    name = "test_cache",
    srcs = [
        "test_cache.c",
    ],
    linkopts = ["-luuid"],
    deps = [
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
        "//tests:test_define",
    ],
)

cc_test(
    name = "test_scylladb",
    srcs = ["test_scylladb.c"],
    deps = select({
        "//accelerator:db_enable": [
            "//storage",
            "//tests:test_define",
        ],
        "//conditions:default": [],
    }),
)

cc_test(
    name = "test_timer",
    srcs = [
        "test_timer.c",
    ],
    deps = [
        "//tests:test_define",
        "//utils:timer",
    ],
)
