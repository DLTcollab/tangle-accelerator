package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mode",
    srcs = ["mode.c"],
    hdrs = ["mode.h"],
    deps = [
        "//accelerator:common_core",
        "//request",
        "//response",
        "@entangled//common/model:bundle",
        "@entangled//common/trinary:flex_trit",
        "@entangled//common/trinary:tryte_ascii",
        "@entangled//mam/api",
        "@entangled//utils/containers/hash:hash_array",
    ],
)
