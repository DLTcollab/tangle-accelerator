load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("//third_party:third_party.bzl", "third_party_deps")

git_repository(
    name = "rules_iota",
    commit = "73f598ad1ce3ba79ff22d747f723d6d5cbf351e1",
    remote = "https://github.com/iotaledger/rules_iota.git",
)

git_repository(
    name = "iota_toolchains",
    commit = "0f93f76fdff9f091dd3a99aa3f19b5d3f4f7f7fc",
    remote = "https://github.com/iotaledger/toolchains.git",
)

git_repository(
    name = "entangled",
    remote = "https://github.com/iotaledger/entangled.git",
    tag = "cclient-v1.0.0-beta",
)

git_repository(
    name = "served",
    commit = "757172e1d8aa7e273e800ce8ec91974c90a2a8b1",
    remote = "https://github.com/meltwater/served.git",
)

git_repository(
    name = "io_bazel_rules_docker",
    remote = "https://github.com/bazelbuild/rules_docker.git",
    tag = "v0.7.0",
)

load("@rules_iota//:defs.bzl", "iota_deps")
load("@io_bazel_rules_docker//repositories:repositories.bzl", container_repositories = "repositories")

container_repositories()

load("@io_bazel_rules_docker//cc:image.bzl", _cc_image_repos = "repositories")

iota_deps()

third_party_deps()

_cc_image_repos()

load("@iota_toolchains//:toolchains.bzl", "setup_initial_deps")

setup_initial_deps()

load("@iota_toolchains//:defs.bzl", "setup_toolchains_repositories")

setup_toolchains_repositories()
