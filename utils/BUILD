package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cache",
    srcs = ["backend_redis.c"],
    hdrs = ["cache.h"],
    deps = [
        "//accelerator:ta_errors",
        "@entangled//common/trinary:flex_trit",
        "@entangled//utils:logger_helper",
        "@hiredis",
    ],
)

cc_library(
    name = "pow",
    srcs = ["pow.c"],
    hdrs = ["pow.h"],
    deps = [
        "//accelerator:ta_errors",
        "//third_party:dcurl",
        "@com_github_uthash//:uthash",
        "@entangled//common/helpers:digest",
        "@entangled//common/model:bundle",
        "@entangled//common/trinary:flex_trit",
        "@entangled//utils:logger_helper",
        "@entangled//utils:time",
    ],
)

cc_library(
    name = "fill_nines",
    srcs = ["fill_nines.c"],
    hdrs = ["fill_nines.h"],
    deps = [
        "//accelerator:ta_errors",
        "@entangled//common/model:transaction",
    ],
)
