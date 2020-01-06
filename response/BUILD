cc_library(
    name = "response",
    srcs = glob([
        "*.c",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    copts = select({
        "//storage:db_enable": ["-DDB_ENABLE"],
        "//conditions:default": [],
    }),
    include_prefix = "response",
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_errors",
        "@entangled//common:errors",
        "@entangled//common/model:transaction",
        "@entangled//mam/mam:message",
        "@entangled//utils/containers/hash:hash243_queue",
        "@entangled//utils/containers/hash:hash243_stack",
    ] + select({
        "//storage:db_enable": ["//storage"],
        "//conditions:default": [],
    }),
)
