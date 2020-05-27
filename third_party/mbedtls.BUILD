cc_library(
    name = "mbedtls_2_16_6",
    srcs = glob(["library/*.c"]),
    hdrs = glob(["include/mbedtls/*.h"]) + glob(["include/psa/*.h"]),
    include_prefix = "mbedtls",
    strip_include_prefix = "include/mbedtls",
    visibility = ["//visibility:public"],
)
