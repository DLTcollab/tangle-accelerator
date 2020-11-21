cc_test(
    name = "test_http",
    srcs = [
        "test_http.c",
    ],
    deps = [
        "//endpoint:https",
        "//endpoint/connectivity:conn_http",
        "//tests:logger_lib",
        "//tests:test_define",
    ],
)

cc_test(
    name = "test_endpoint_core",
    srcs = [
        "test_endpoint_core.c",
    ],
    deps = [
        "//common",
        "//endpoint:endpoint_core",
        "//tests:logger_lib",
        "//tests:test_define",
    ],
)
