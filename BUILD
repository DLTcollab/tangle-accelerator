package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "tangle-accelerator",
    copts = ["-DLOGGER_ENABLE"],
    srcs = ["server.cc"], 
    deps = ["@entangled//cclient:api",],
    visibility = ["//visibility:public"],
)

