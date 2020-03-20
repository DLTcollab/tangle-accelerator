cc_library(
    name = "apis",
    srcs = ["apis.c"],
    hdrs = ["apis.h"],
    linkopts = [
        "-lpthread",
        "-luuid",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:build_option",
        "//accelerator/core",
        "//accelerator/core:mam_core",
        "//accelerator/core/serializer",
        "//common",
    ],
)

cc_library(
    name = "proxy_apis",
    srcs = ["proxy_apis.c"],
    hdrs = ["proxy_apis.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator/core/serializer",
        "//utils:char_buffer_str",
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
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_config",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//accelerator/core/serializer",
        "//utils:bundle_array",
        "//utils:char_buffer_str",
        "//utils:timer",
        "@com_github_uthash//:uthash",
        "@entangled//cclient/api",
        "@entangled//utils:time",
        "@entangled//utils/containers/hash:hash243_set",
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
        "@entangled//common/trinary:flex_trit",
        "@entangled//mam/api",
        "@entangled//utils/containers/hash:hash_array",
    ],
)

cc_library(
    name = "pow",
    srcs = ["pow.c"],
    hdrs = ["pow.h"],
    visibility = ["//visibility:public"],
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
