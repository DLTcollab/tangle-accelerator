cc_library(
    name = "http",
    srcs = ["http.c"],
    hdrs = ["http.h"],
    defines = select({
        "//storage:db_enable": ["DB_ENABLE"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//accelerator/core:apis",
        "//accelerator/core:proxy_apis",
        "@libmicrohttpd",
    ],
)
