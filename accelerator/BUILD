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
        ":ta_errors",
        "//serializer",
        "@entangled//common/trinary:trit_tryte",
        "@entangled//common/trinary:tryte_ascii",
        "@entangled//mam/api",
    ],
)

cc_library(
    name = "common_core",
    srcs = ["common_core.c"],
    hdrs = ["common_core.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ta_config",
        ":ta_errors",
        "//request",
        "//response",
        "//utils:cache",
        "//utils:pow",
        "@com_github_uthash//:uthash",
        "@entangled//common/model:bundle",
        "@entangled//utils:time",
    ],
)

cc_library(
    name = "ta_config",
    srcs = ["config.c"],
    hdrs = ["config.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ta_errors",
        "@entangled//cclient/api",
        "@entangled//cclient/types",
    ],
)

cc_library(
    name = "ta_errors",
    hdrs = ["errors.h"],
    visibility = ["//visibility:public"],
)
