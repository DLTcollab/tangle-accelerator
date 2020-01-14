cc_library(
    name = "serializer",
    srcs = ["serializer.c"],
    hdrs = ["serializer.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_config",
        "//common",
        "//accelerator:build_option",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//utils:fill_nines",
        "@cJSON",
        "@entangled//cclient/response:responses",
        "@entangled//common/trinary:flex_trit",
        "@entangled//utils:char_buffer",
        "@entangled//utils/containers/hash:hash_array",
    ] + select({
        "//accelerator:mqtt_enable": ["//connectivity/mqtt:mqtt_common"],
        "//conditions:default": [],
    }),
)
