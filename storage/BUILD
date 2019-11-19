package(default_visibility = ["//visibility:public"])

cc_library(
    name = "storage",
    srcs = ["scylla_api.c"],
    hdrs = [
        "scylla_api.h",
    ],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = [
        "-lcassandra",
    ],
    deps = [
        ":scylla_table",
        "//accelerator:common_core",
        "//accelerator:ta_errors",
    ],
)

cc_library(
    name = "scylla_table",
    srcs = ["scylla_table.c"],
    hdrs = ["scylla_table.h"],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = ["-lcassandra"],
    deps = [
        "//accelerator:common_core",
        "//accelerator:ta_errors",
    ],
)
