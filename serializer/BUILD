package(default_visibility = ["//visibility:public"])

cc_library(
    name = "serializer",
    srcs = ["serializer.c"],
    hdrs = ["serializer.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_errors",
        "//request",
        "//response",
        "//utils:fill_nines",
        "@cJSON",
        "@entangled//cclient/serialization:serializer_json",
        "@entangled//common/trinary:flex_trit",
        "@entangled//common/trinary:tryte_ascii",
        "@entangled//utils:char_buffer",
        "@entangled//utils/containers/hash:hash_array",
    ],
)
