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
    deps = [
        ":mqtt_common",
        "//accelerator:ta_errors",
    ],
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
    deps = [
        "//accelerator:ta_config",
        "//accelerator:ta_errors",
        "//third_party:mosquitto",
    ],
)
