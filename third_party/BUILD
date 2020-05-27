package(default_visibility = ["//visibility:public"])

cc_import(
    name = "dcurl",
    hdrs = [
        "dcurl/src/dcurl.h",
        "dcurl/src/trinary.h",
    ],
    shared_library = "dcurl/build/libdcurl.so",
)

cc_library(
    name = "mosquitto",
    srcs = ["mosquitto/lib/libmosquitto.so.1"],
    hdrs = [
        "mosquitto/config.h",
        "mosquitto/lib/mosquitto.h",
        "mosquitto/lib/mqtt_protocol.h",
    ],
)

cc_library(
    name = "noise",
    srcs = ["noise-c/src/protocol/libnoiseprotocol.a"],
    hdrs = glob([
        "noise-c/include/noise/*.h",
    ]),
    include_prefix = "third_party/noise-c/include",
)
