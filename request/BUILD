cc_library(
    name = "request",
    srcs = glob([
        "*.c",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    include_prefix = "request",
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_errors",
        "@entangled//common:errors",
        "@entangled//common/model:transaction",
        "@entangled//common/trinary:tryte",
        "@entangled//utils/containers/hash:hash243_queue",
        "@entangled//utils/containers/hash:hash81_queue",
    ],
)
