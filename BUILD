package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "tangle_accelerator",
    srcs = ["server.cc"],
    deps = ["@boost//:beast"],
)

cc_library(
    name = "tangle_accelerator_api",
    srcs = ["tangle_accelerator_core_api.c"],
    hdrs = ["tangle_accelerator_core_api.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":common_core",
        "//serializer",
        "@entangled//cclient:api",
        "@entangled//cclient/types",
    ],
)

cc_library(
    name = "common_core",
    srcs = ["common_core.c"],
    hdrs = ["common_core.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":ta_config",
        "//request",
        "//response",
        "@com_github_uthash//:uthash",
        "@entangled//cclient:api",
        "@entangled//cclient/types",
    ],
)

cc_library(
    name = "ta_config",
    hdrs = ["config.h"],
    visibility = ["//visibility:public"],
)
