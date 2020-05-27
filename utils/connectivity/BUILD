package(default_visibility = ["//visibility:public"])

cc_library(
    name = "conn_http",
    srcs = ["conn_http.c"],
    hdrs = [
        "conn_http.h",
    ],
    deps = [
        "//common",
        "//pem:cert",
        "@http_parser",
        "@mbedtls_2_16_6",
    ],
)
