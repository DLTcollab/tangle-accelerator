package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    deps = [
        "macros",
        ":debug",
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
        ":ta_errors",
        "@org_iota_common//utils:logger_helper",
    ],
)

cc_library(
    name = "macros",
    hdrs = ["macros.h"],
    deps = [
        "@mam.c//mam/mam:message",
        "@org_iota_common//common/model:transaction",
        "@org_iota_common//common/trinary:tryte",
        "@org_iota_common//utils/containers/hash:hash243_stack",
        "@org_iota_common//utils/containers/hash:hash_array",
    ],
)

cc_library(
    name = "debug",
    srcs = ["debug.c"],
    hdrs = ["debug.h"],
    deps = [
        ":ta_logger",
        "@org_iota_common//common/model:bundle",
        "@org_iota_common//common/model:transaction",
    ],
)
