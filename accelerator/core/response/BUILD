cc_library(
    name = "response",
    srcs = glob([
        "*.c",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    include_prefix = "accelerator/core/response",
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//accelerator:build_option",
        "@entangled//common:errors",
        "@entangled//common/model:transaction",
        "@entangled//mam/mam:message",
        "@entangled//utils/containers/hash:hash243_queue",
        "@entangled//utils/containers/hash:hash243_stack",
    ] + select({
        "//accelerator:db_enable": ["//storage"],
        "//conditions:default": [],
    }),
)
