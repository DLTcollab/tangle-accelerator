cc_library(
    name = "common",
    srcs = ["common.c"],
    hdrs = ["common.h"],
    deps = [
        "//accelerator:ta_config",
        "//common",
    ],
)

cc_test(
    name = "driver",
    srcs = [
        "driver.c",
    ],
    deps = [
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//tests:test_define",
        "//tests/api:common",
    ],
)

cc_binary(
    name = "driver_stat",
    srcs = [
        "driver.c",
    ],
    defines = ["ENABLE_STAT"],
    deps = [
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//tests:test_define",
        "//tests/api:common",
    ],
)

cc_test(
    name = "driver_core",
    srcs = [
        "driver_core.c",
    ],
    deps = [
        "//accelerator/core",
        "//tests:test_define",
        "//tests/api:common",
    ],
)

cc_test(
    name = "mam_test",
    srcs = [
        "mam_test.c",
    ],
    deps = [
        ":common",
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//common",
        "//tests:test_define",
    ],
)
