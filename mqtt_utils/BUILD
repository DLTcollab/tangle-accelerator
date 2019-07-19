cc_library(
    name = "mqtt_utils",
    srcs = [
        "duplex_callback.c",
        "duplex_utils.c",
    ],
    hdrs = [
        "duplex_callback.h",
        "duplex_utils.h",
    ],
    visibility = ["//visibility:public"],
    deps = [":mqtt_common"],
)

cc_library(
    name = "mqtt_common",
    srcs = [
        "client_common.c",
        "pub_utils.c",
        "sub_utils.c",
    ],
    hdrs = [
        "client_common.h",
        "pub_utils.h",
        "sub_utils.h",
    ],
    deps = ["//third_party:mosquitto"],
)
