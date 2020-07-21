cc_library(
    name = "test_define",
    hdrs = ["test_define.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_config",
        "@unity",
    ],
)

cc_library(
    name = "common",
    srcs = ["common.c"],
    hdrs = ["common.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_config",
        "//common",
    ],
)

cc_library(
    name = "logger_lib",
    visibility = [
        "//endpoint/unit-test:__pkg__",
        "//tests:__subpackages__",
    ],
    deps = select({
        "//accelerator:mqtt_enable": ["//connectivity/mqtt"],
        "//conditions:default": ["//connectivity/http"],
    }),
)
