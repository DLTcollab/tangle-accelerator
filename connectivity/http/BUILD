cc_library(
    name = "http",
    srcs = ["http.c"],
    hdrs = ["http.h"],
    copts = select({
        "//connectivity/mqtt:mqtt_enable": [
            "-DMQTT_ENABLE",
        ],
        "//conditions:default": [],
    }) + select({
        "//storage:db_enable": ["-DDB_ENABLE"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_config",
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "//common:ta_errors",
        "@libmicrohttpd",
    ],
)
