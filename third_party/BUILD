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
    name = "hiredis",
    hdrs = [
        "hiredis/async.h",
        "hiredis/dict.h",
        "hiredis/fmacros.h",
        "hiredis/hiredis.h",
        "hiredis/net.h",
        "hiredis/read.h",
        "hiredis/sds.h",
        "hiredis/sdsalloc.h",
    ],
    static_library = "hiredis/libhiredis.a",
)
