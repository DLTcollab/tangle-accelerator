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
        "//accelerator:common_core",
        "//accelerator:ta_errors",
    ],
)

cc_binary(
    name = "listener",
    srcs = ["scylla_listener.c"],
    linkopts = [
        "-lzmq",
    ],
    deps = [
        ":storage",
    ],
)

cc_binary(
    name = "importer",
    srcs = ["scylla_importer.c"],
    deps = [
        ":storage",
    ],
)
