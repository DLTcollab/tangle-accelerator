package(default_visibility = ["//visibility:public"])

cc_import(
    name = "dcurl",
    hdrs = [
        "dcurl/src/dcurl.h",
        "dcurl/src/trinary.h",
    ],
    shared_library = "dcurl/build/libdcurl.so",
)

cc_import(
    name = "mosquitto",
    hdrs = [
        "mosquitto/config.h",
        "mosquitto/lib/mosquitto.h",
        "mosquitto/lib/mqtt_protocol.h",
    ],
    shared_library = "mosquitto/lib/libmosquitto.so",
)
