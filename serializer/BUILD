package(default_visibility = ["//visibility:public"])

cc_library(
    name = "serializer",
    srcs = ["serializer.c"],
    hdrs = ["serializer.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//request",
        "//response",
        "@cJSON",
        "@entangled//cclient/types",
    ],
)

cc_test(
    name = "test_serializer",
    srcs = [
        "test/test_serializer.c",
        "test/test_serializer.h",
    ],
    deps = [
        ":serializer",
        "@unity",
    ],
)
