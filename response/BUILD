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
        "@entangled//cclient/types",
        "@entangled//common/model:transaction",
    ],
)
