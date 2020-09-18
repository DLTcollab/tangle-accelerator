cc_test(
    name = "driver",
    srcs = [
        "driver.c",
    ],
    deps = [
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//tests:common",
        "//tests:logger_lib",
        "//tests:test_define",
        "@cJSON",
    ],
)

cc_binary(
    name = "driver_stat",
    srcs = [
        "driver.c",
    ] + select({
        "//accelerator:mqtt_enable": ["//connectivity/mqtt"],
        "//conditions:default": ["//connectivity/http"],
    }),
    defines = ["ENABLE_STAT"],
    deps = [
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//tests:common",
        "//tests:logger_lib",
        "//tests:test_define",
        "@cJSON",
    ],
)

cc_test(
    name = "test_periodical_task",
    srcs = [
        "test_periodical_task.c",
    ],
    deps = [
        "//accelerator/core",
        "//accelerator/core:apis",
        "//accelerator/core:periodical_task",
        "//tests:common",
        "//tests:logger_lib",
        "//tests:test_define",
    ],
)

cc_test(
    name = "mam_test",
    srcs = [
        "mam_test.c",
    ],
    deps = [
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//common",
        "//tests:common",
        "//tests:logger_lib",
        "//tests:test_define",
    ],
)
