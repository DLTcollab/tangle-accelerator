load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "rules_iota",
    commit = "e81ce3c6384cc1404858e53c2abebf9a18031a93",
    remote = "https://github.com/iotaledger/rules_iota.git",
)

git_repository(
    name = "entangled",
    commit = "86346c44a1cd1442947c66c675cd7ab93e08bca4",
    remote = "https://github.com/iotaledger/entangled.git",
)

load("@rules_iota//:defs.bzl", "iota_deps")

iota_deps()
