package(default_visibility = ["//visibility:public"])

cc_library(
    name = "storage",
    hdrs = ["ta_storage.h"],
    deps = [
        ":scylladb_client",
        ":scylladb_identity",
        ":scylladb_permanode",
    ],
)

cc_library(
    name = "scylladb_identity",
    srcs = ["scylladb_identity.c"],
    hdrs = ["scylladb_identity.h"],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = ["-lcassandra"],
    deps = [
        ":scylladb_client",
        ":scylladb_utils",
        "@com_github_uthash//:uthash",
    ],
)

cc_library(
    name = "scylladb_permanode",
    srcs = ["scylladb_permanode.c"],
    hdrs = ["scylladb_permanode.h"],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = ["-lcassandra"],
    deps = [
        ":scylladb_client",
        ":scylladb_utils",
        "@com_github_uthash//:uthash",
        "@entangled//utils/containers/hash:hash243_queue",
    ],
)

cc_library(
    name = "scylladb_client",
    srcs = ["scylladb_client.c"],
    hdrs = [
        "scylladb_client.h",
    ],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = [
        "-lcassandra",
    ],
    deps = [
        ":scylladb_utils",
    ],
)

cc_library(
    name = "scylladb_utils",
    srcs = ["scylladb_utils.c"],
    hdrs = ["scylladb_utils.h"],
    copts = ["-DLOGGER_ENABLE"],
    linkopts = ["-lcassandra"],
    deps = [
        "//accelerator:ta_errors",
        "//utils:ta_logger",
        "@entangled//common/model:bundle",
    ],
)
