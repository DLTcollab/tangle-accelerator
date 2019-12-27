package(default_visibility = ["//visibility:public"])

cc_library(
    name = "apis",
    srcs = ["apis.c"],
    hdrs = ["apis.h"],
    copts = select({
        "//connectivity/mqtt:mqtt_enable": [
            "-DMQTT_ENABLE",
        ],
        "//conditions:default": [],
    }) + select({
        "//storage:db_enable": ["-DDB_ENABLE"],
        "//conditions:default": [],
    }),
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator/core",
        "//accelerator/core:mam_core",
        "//accelerator/core/serializer",
        "//common:ta_errors",
        "@entangled//common/model:bundle",
        "@entangled//common/trinary:trit_tryte",
        "@entangled//mam/api",
    ],
)

cc_library(
    name = "proxy_apis",
    srcs = ["proxy_apis.c"],
    hdrs = ["proxy_apis.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator/core/serializer",
        "//common:ta_errors",
        "//common:ta_logger",
        "//utils:hash_algo_djb2",
        "@entangled//cclient/api",
        "@entangled//cclient/request:requests",
        "@entangled//cclient/response:responses",
    ],
)

cc_library(
    name = "core",
    srcs = ["core.c"],
    hdrs = ["core.h"],
    deps = [
        "//accelerator:ta_config",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//common:ta_errors",
        "//utils:bundle_array",
        "//utils:timer",
        "@com_github_uthash//:uthash",
        "@entangled//cclient/api",
        "@entangled//cclient/serialization:serializer",
        "@entangled//cclient/serialization:serializer_json",
        "@entangled//common/model:bundle",
        "@entangled//utils:time",
    ],
)

cc_library(
    name = "mam_core",
    srcs = ["mam_core.c"],
    hdrs = ["mam_core.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator/core",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "@entangled//common/model:bundle",
        "@entangled//common/trinary:flex_trit",
        "@entangled//common/trinary:tryte_ascii",
        "@entangled//mam/api",
        "@entangled//utils/containers/hash:hash_array",
    ],
)

cc_library(
    name = "pow",
    srcs = ["pow.c"],
    hdrs = ["pow.h"],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "//third_party:dcurl",
        "@com_github_uthash//:uthash",
        "@entangled//common/helpers:digest",
        "@entangled//common/model:bundle",
        "@entangled//common/trinary:flex_trit",
        "@entangled//utils:time",
    ],
)
