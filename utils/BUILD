package(default_visibility = ["//visibility:public"])

cc_library(
    name = "fill_nines",
    srcs = ["fill_nines.h"],
    deps = [
        "//common:ta_errors",
        "@org_iota_common//common/model:transaction",
    ],
)

cc_library(
    name = "bundle_array",
    srcs = ["bundle_array.h"],
    deps = [
        "//common:ta_errors",
        "@com_github_uthash//:uthash",
        "@org_iota_common//common/model:bundle",
    ],
)

cc_library(
    name = "hash_algo_djb2",
    hdrs = ["hash_algo_djb2.h"],
)

cc_library(
    name = "timer",
    srcs = ["timer.c"],
    hdrs = ["timer.h"],
    linkopts = [
        "-lpthread",
        "-lrt",
    ],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
    ],
)

cc_library(
    name = "char_buffer_str",
    srcs = ["char_buffer_str.h"],
    deps = ["@org_iota_common//utils:char_buffer"],
)

cc_library(
    name = "tryte_byte_conv",
    srcs = ["tryte_byte_conv.c"],
    hdrs = ["tryte_byte_conv.h"],
    deps = ["//common:ta_errors"],
)

cc_library(
    name = "cpuinfo",
    hdrs = ["cpuinfo.h"],
)
