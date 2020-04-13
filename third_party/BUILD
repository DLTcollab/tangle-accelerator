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
    name = "http-parser",
    srcs = glob(["http-parser/*.so.*"]),
    hdrs = ["http-parser/http_parser.h"],
    includes = ["http-parser"],
)

cc_library(
    name = "mbedtls",
    srcs = glob(["mbedtls/library/*.so.*"]),
    hdrs = glob(["mbedtls/include/mbedtls/*.h"]) + glob(["mbedtls/include/psa/*.h"]),
    includes = ["mbedtls/include"],
)
