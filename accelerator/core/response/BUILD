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
        "@org_iota_common//common:errors",
        "@org_iota_common//common/model:transaction",
        "@mam.c//mam/mam:message",
        "@org_iota_common//utils/containers/hash:hash243_queue",
        "@org_iota_common//utils/containers/hash:hash243_stack",
    ] + select({
        "//accelerator:db_enable": ["//storage"],
        "//conditions:default": [],
    }),
)
