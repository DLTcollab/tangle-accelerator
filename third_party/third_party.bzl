load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_hiredis():
    http_archive(
        name = "hiredis",
        url = "https://github.com/redis/hiredis/archive/v0.14.0.tar.gz",
        strip_prefix = "hiredis-0.14.0",
        sha256 =
            "042f965e182b80693015839a9d0278ae73fae5d5d09d8bf6d0e6a39a8c4393bd",
        build_file = "//third_party:BUILD.hiredis",
    )

def third_party_deps():
    load_hiredis()
