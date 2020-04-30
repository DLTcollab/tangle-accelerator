package(default_visibility = ["//visibility:public"])

cc_library(
    name = "conn_http",
    srcs = ["conn_http.c"],
    hdrs = [
        "conn_http.h",
    ],
    deps = [
        "//common:defined_error",
        "//pem:cert",
        "//third_party:http-parser",
        "//third_party:mbedtls",
    ],
)
