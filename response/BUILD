cc_library(
    name = "response",
    srcs = glob([
        "*.c",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    include_prefix = "response",
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_errors",
        "@entangled//common:errors",
        "@entangled//common/model:transaction",
        "@entangled//utils/containers/hash:hash243_queue",
        "@entangled//utils/containers/hash:hash243_stack",
    ],
)
