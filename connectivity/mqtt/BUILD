config_setting(
    name = "mqtt_enable",
    values = {"define": "mqtt=enable"},
)

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
    copts = ["-DMQTT_ENABLE"],
    visibility = ["//visibility:public"],
    deps = [
        ":mqtt_common",
        "//accelerator/core",
        "//accelerator/core:apis",
        "//common:ta_errors",
        "@entangled//common/model:transaction",
    ],
)

cc_library(
    name = "mqtt_common",
    srcs = [
        "mqtt_common.c",
        "pub_utils.c",
        "sub_utils.c",
    ],
    hdrs = [
        "mqtt_common.h",
        "pub_utils.h",
        "sub_utils.h",
    ],
    copts = ["-DMQTT_ENABLE"],
    visibility = ["//accelerator/core/serializer:__pkg__"],
    deps = [
        "//accelerator:ta_config",
        "//common:ta_errors",
        "//third_party:mosquitto",
    ],
)
