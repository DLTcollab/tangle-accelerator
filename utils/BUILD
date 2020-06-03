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
    name = "cipher",
    srcs = ["cipher.c"],
    hdrs = ["cipher.h"],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "@mbedtls_2_16_6",
    ],
)

cc_library(
    name = "https",
    srcs = ["https.c"],
    hdrs = ["https.h"],
    defines = select({
        ":https_enable": ["ENDPOINT_HTTPS"],
        "//conditions:default": [],
    }),
    deps = [
        "//common:ta_errors",
        "//utils/connectivity:conn_http",
    ],
)

config_setting(
    name = "https_enable",
    values = {"define": "https=enable"},
)

cc_library(
    name = "text_serializer",
    srcs = ["text_serializer.c"],
    hdrs = ["text_serializer.h"],
    deps = [
        ":cipher",
        "//common:ta_errors",
    ],
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
