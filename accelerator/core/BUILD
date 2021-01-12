cc_library(
    name = "apis",
    srcs = ["apis.c"],
    hdrs = ["apis.h"],
    linkopts = [
        "-lpthread",
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
        "@iota.c//cclient/api",
        "@iota.c//cclient/request:requests",
        "@iota.c//cclient/response:responses",
    ],
)

cc_library(
    name = "core",
    srcs = ["core.c"],
    hdrs = ["core.h"],
    linkopts = [
        "-luuid",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator:ta_config",
        "//accelerator/core/request",
        "//accelerator/core/response",
        "//accelerator/core/serializer",
        "//common",
        "//utils:bundle_array",
        "//utils:char_buffer_str",
        "//utils:timer",
        "@com_github_uthash//:uthash",
        "@iota.c//cclient/api",
        "@org_iota_common//utils:time",
        "@org_iota_common//utils/containers/hash:hash243_set",
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
        "//utils:fill_nines",
        "//utils:tryte_byte_conv",
        "@mam.c//mam/api",
        "@org_iota_common//common/trinary:flex_trit",
        "@org_iota_common//utils/containers/hash:hash_array",
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
        "@org_iota_common//common/helpers:digest",
        "@org_iota_common//common/model:bundle",
        "@org_iota_common//common/trinary:flex_trit",
        "@org_iota_common//utils:time",
    ],
)

cc_library(
    name = "periodical_task",
    srcs = ["periodical_task.c"],
    hdrs = ["periodical_task.h"],
    linkopts = [
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":core",
        ":mam_core",
        "//common:ta_errors",
        "//common:ta_logger",
    ],
)
