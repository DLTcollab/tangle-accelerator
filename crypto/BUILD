cc_library(
    name = "ecdh",
    srcs = ["ecdh.c"],
    hdrs = ["ecdh.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":randomness",
        "//common",
        "@mbedtls",
    ],
)

cc_library(
    name = "ecdsa",
    srcs = ["ecdsa.c"],
    hdrs = ["ecdsa.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":randomness",
        "//common",
        "@mbedtls",
    ],
)

cc_library(
    name = "randomness",
    srcs = ["randomness.c"],
    hdrs = ["randomness.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@mbedtls",
    ],
)
