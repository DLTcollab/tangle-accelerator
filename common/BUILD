package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    deps = [
        "macros",
        ":ta_errors",
        ":ta_logger",
    ],
)

cc_library(
    name = "ta_errors",
    srcs = ["ta_errors.c"],
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

cc_library(
    name = "macros",
    hdrs = ["macros.h"],
    deps = [
        "@entangled//common/model:transaction",
        "@entangled//common/trinary:tryte",
        "@entangled//mam/mam:message",
    ],
)
