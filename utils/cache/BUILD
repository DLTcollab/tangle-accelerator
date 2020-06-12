package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cache",
    srcs = ["backend_redis.c"],
    hdrs = ["cache.h"],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "@hiredis",
        "@org_iota_common//common/trinary:flex_trit",
    ],
)
