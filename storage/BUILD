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
        "//accelerator:ta_errors",
        "@entangled//utils/containers/hash:hash243_queue",
    ],
)

cc_library(
    name = "scylla_table",
    srcs = ["scylla_table.c"],
    hdrs = ["scylla_table.h"],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = ["-lcassandra"],
    deps = [
        "//accelerator:ta_errors",
        "//utils:ta_logger",
        "@com_github_uthash//:uthash",
        "@entangled//common/model:bundle",
    ],
)
