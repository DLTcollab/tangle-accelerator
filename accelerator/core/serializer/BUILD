cc_library(
    name = "serializer",
    srcs = ["serializer.c"],
    hdrs = ["serializer.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ser_helper",
        ":ser_mam",
        "//accelerator:ta_config",
        "//common",
        "//accelerator:build_option",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//utils:fill_nines",
    ] + select({
        "//accelerator:mqtt_enable": ["//connectivity/mqtt:mqtt_common"],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "ser_helper",
    srcs = ["ser_helper.c"],
    hdrs = ["ser_helper.h"],
    deps = [
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//common",
        "@cJSON",
    ],
)

cc_library(
    name = "ser_mam",
    srcs = ["ser_mam.c"],
    hdrs = ["ser_mam.h"],
    deps = [
        ":ser_helper",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//common",
        "@cJSON",
    ],
)
