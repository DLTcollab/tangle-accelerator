package(default_visibility = ["//visibility:public"])

cc_library(
    name = "fill_nines",
    srcs = ["fill_nines.h"],
    deps = [
        "//common:ta_errors",
        "@entangled//common/model:transaction",
    ],
)

cc_library(
    name = "bundle_array",
    srcs = ["bundle_array.h"],
    deps = [
        "//common:ta_errors",
        "@com_github_uthash//:uthash",
        "@entangled//common/model:bundle",
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
    deps = ["@entangled//utils:char_buffer"],
)

cc_library(
    name = "cipher",
    srcs = ["cipher.c"],
    hdrs = ["cipher.h"],
    deps = [
        "//common:defined_error",
        "//third_party:mbedtls",
    ],
)

cc_library(
    name = "https",
    srcs = ["https.c"],
    hdrs = ["https.h"],
    deps = [
        "//common:defined_error",
        "//utils/connectivity:conn_http",
    ],
)

cc_library(
    name = "text_serializer",
    srcs = ["text_serializer.c"],
    hdrs = ["text_serializer.h"],
    deps = ["//common:defined_error"],
)

cc_library(
    name = "tryte_byte_conv",
    srcs = ["tryte_byte_conv.c"],
    hdrs = ["tryte_byte_conv.h"],
    deps = ["//common:defined_error"],
)

cc_library(
    name = "cpuinfo",
    hdrs = ["cpuinfo.h"],
)
