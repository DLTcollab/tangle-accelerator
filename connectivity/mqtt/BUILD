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
    copts = ["-DENABLE_MQTT"],
    visibility = ["//visibility:public"],
    deps = [
        ":mqtt_common",
        "//accelerator:apis",
        "//accelerator:core",
        "//accelerator:ta_errors",
        "@entangled//common/model:transaction",
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
    copts = ["-DENABLE_MQTT"],
    deps = [
        "//accelerator:ta_config",
        "//accelerator:ta_errors",
        "//third_party:mosquitto",
    ],
)
