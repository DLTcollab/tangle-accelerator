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
        "@entangled//cclient/types",
        "@entangled//common/model:transaction",
        "//accelerator:ta_errors",
    ],
)
