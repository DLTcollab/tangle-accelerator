package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cache",
    srcs = ["backend_redis.c"],
    hdrs = ["cache.h"],
    deps = [
        ":ta_logger",
        "//accelerator:ta_errors",
        "@entangled//common/trinary:flex_trit",
        "@hiredis",
    ],
)

cc_library(
    name = "pow",
    srcs = ["pow.c"],
    hdrs = ["pow.h"],
    deps = [
        ":ta_logger",
        "//accelerator:ta_errors",
        "//third_party:dcurl",
        "@com_github_uthash//:uthash",
        "@entangled//common/helpers:digest",
        "@entangled//common/model:bundle",
        "@entangled//common/trinary:flex_trit",
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

cc_library(
    name = "bundle_array",
    srcs = ["bundle_array.h"],
    deps = [
        "@com_github_uthash//:uthash",
        "@entangled//common/model:bundle",
    ],
)

cc_library(
    name = "ta_logger",
    srcs = ["logger.h"],
    deps = [
        "@entangled//utils:logger_helper",
    ],
)

cc_library(
    name = "hash_algo_djb2",
    hdrs = ["hash_algo_djb2.h"],
)
