package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cache",
    srcs = ["backend_redis.c"],
    hdrs = ["cache.h"],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "@entangled//common/trinary:flex_trit",
        "@hiredis",
    ],
)
