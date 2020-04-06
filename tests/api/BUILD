cc_test(
    name = "driver",
    srcs = [
        "driver.c",
    ],
    deps = [
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//tests:test_define",
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
    ],
)
