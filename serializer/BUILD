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
        "@entangled//common/trinary:flex_trit",
        "@entangled//common/trinary:tryte_ascii",
        "@entangled//utils/containers/hash:hash_array",
    ],
)
