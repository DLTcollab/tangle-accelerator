package(default_visibility = ["//visibility:public"])

cc_library(
    name = "fill_nines",
    srcs = ["fill_nines.h"],
    deps = [
        "//common:ta_errors",
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
    name = "hash_algo_djb2",
    hdrs = ["hash_algo_djb2.h"],
)

cc_library(
    name = "timer",
    srcs = ["timer.c"],
    hdrs = ["timer.h"],
    linkopts = [
        "-lpthread",
        "-lrt",
    ],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
    ],
)
