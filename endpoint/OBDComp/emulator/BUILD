cc_binary(
    name = "obd_emulator",
    srcs = [
        "obd_emulator.c",
        "obd_emulator.h",
    ],
    deps = [
        "//common:ta_errors",
        "//endpoint/OBDComp:obd_pid",
        "//endpoint/OBDComp/can-bus:can-utils",
    ],
)
