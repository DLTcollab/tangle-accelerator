package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cache",
    srcs = ["backend_redis.c"],
    hdrs = ["cache.h"],
    deps = [
        "//accelerator:ta_errors",
        "@entangled//cclient/types",
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
        "@entangled//cclient/types",
        "@entangled//common/helpers:digest",
        "@entangled//common/model:bundle",
        "@entangled//utils:time",
    ],
)
