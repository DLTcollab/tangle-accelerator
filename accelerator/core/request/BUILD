cc_library(
    name = "request",
    srcs = glob([
        "*.c",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    include_prefix = "accelerator/core/request",
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@org_iota_common//common:errors",
        "@org_iota_common//common/model:transaction",
        "@org_iota_common//common/trinary:tryte",
        "@org_iota_common//utils/containers/hash:hash243_queue",
        "@org_iota_common//utils/containers/hash:hash81_queue",
    ],
)
