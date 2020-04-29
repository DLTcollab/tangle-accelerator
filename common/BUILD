package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    deps = [
        "macros",
        ":debug",
        ":ta_errors",
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
        ":ta_errors",
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

cc_library(
    name = "debug",
    srcs = ["debug.c"],
    hdrs = ["debug.h"],
    deps = [
        ":ta_logger",
        "@entangled//common/model:bundle",
        "@entangled//common/model:transaction",
    ],
)
