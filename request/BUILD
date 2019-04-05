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
        "@entangled//cclient/types",
        "//accelerator:ta_errors",
    ],
)
