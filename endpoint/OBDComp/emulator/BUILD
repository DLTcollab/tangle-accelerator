cc_binary(
    name = "obd-emulator",
    srcs = [
        "obd_emulator.c",
        "obd_emulator.h",
    ],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "//endpoint/OBDComp:obd_pid",
        "//endpoint/OBDComp/can-bus:can-utils",
    ],
)
