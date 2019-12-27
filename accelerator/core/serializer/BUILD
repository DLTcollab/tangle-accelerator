cc_library(
    name = "serializer",
    srcs = ["serializer.c"],
    hdrs = ["serializer.h"],
    copts = ["-DLOGGER_ENABLE"] + select({
        "//connectivity/mqtt:mqtt_enable": ["-DMQTT_ENABLE"],
        "//conditions:default": [],
    }) + select({
        "//storage:db_enable": ["-DDB_ENABLE"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_config",
        "//common:ta_errors",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//utils:fill_nines",
        "//common:ta_logger",
        "@cJSON",
        "@entangled//cclient/response:responses",
        "@entangled//common/trinary:flex_trit",
        "@entangled//common/trinary:tryte_ascii",
        "@entangled//utils:char_buffer",
        "@entangled//utils/containers/hash:hash_array",
    ] + select({
        "//connectivity/mqtt:mqtt_enable": ["//connectivity/mqtt:mqtt_common"],
        "//conditions:default": [],
    }),
)
