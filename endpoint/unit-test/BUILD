cc_test(
    name = "test_http",
    srcs = [
        "test_http.c",
    ],
    deps = [
        "//endpoint:https",
        "//endpoint/connectivity:conn_http",
        "//tests:test_define",
    ],
)

cc_test(
    name = "test_endpoint_core",
    srcs = [
        "test_endpoint_core.c",
    ],
    deps = [
        "//common:ta_errors",
        "//common:ta_logger",
        "//endpoint:endpoint_core",
        "//tests:test_define",
    ],
)
