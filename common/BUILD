package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    deps = [
        ":ta_errors",
        ":ta_logger",
    ],
)

cc_library(
    name = "ta_errors",
    hdrs = ["ta_errors.h"],
)

cc_library(
    name = "ta_logger",
    srcs = ["logger.h"],
    defines = ["LOGGER_ENABLE"],
    deps = [
        "@entangled//utils:logger_helper",
    ],
)
