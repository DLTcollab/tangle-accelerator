package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "accelerator",
    srcs = ["server.cc"],
    deps = [
        ":apis",
        ":ta_config",
        ":ta_errors",
        "@served",
    ],
)

cc_library(
    name = "apis",
    srcs = ["apis.c"],
    hdrs = ["apis.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":common_core",
        "//serializer",
        "@entangled//cclient:api",
    ],
)

cc_library(
    name = "common_core",
    srcs = ["common_core.c"],
    hdrs = ["common_core.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ta_config",
        "//request",
        "//response",
        "@com_github_uthash//:uthash",
        "@entangled//cclient:api",
        "@entangled//cclient/types",
    ],
)

cc_library(
    name = "ta_config",
    hdrs = ["config.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "ta_errors",
    hdrs = ["errors.h"],
    visibility = ["//visibility:public"],
)
