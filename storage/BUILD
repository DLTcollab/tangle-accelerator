cc_library(
    name = "storage",
    hdrs = ["ta_storage.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":scylladb_identity",
        ":scylladb_permanode",
    ],
)

cc_library(
    name = "scylladb_identity",
    srcs = ["scylladb_identity.c"],
    hdrs = ["scylladb_identity.h"],
    linkopts = ["-lcassandra"],
    deps = [
        ":scylladb_client",
        "@com_github_uthash//:uthash",
    ],
)

cc_library(
    name = "scylladb_permanode",
    srcs = ["scylladb_permanode.c"],
    hdrs = ["scylladb_permanode.h"],
    linkopts = ["-lcassandra"],
    deps = [
        ":scylladb_client",
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
    linkopts = ["-lcassandra"],
    deps = [
        "//common",
        "@entangled//common/model:bundle",
    ],
)
