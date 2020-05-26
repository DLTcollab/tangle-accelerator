load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("//third_party:third_party.bzl", "third_party_deps")

git_repository(
    name = "rules_iota",
    commit = "e08b0038f376d6c82b80f5283bb0a86648bb58dc",
    remote = "https://github.com/iotaledger/rules_iota.git",
)

git_repository(
    name = "iota_toolchains",
    commit = "700904f445d15ef948d112bf0bccf7dd3814ae5c",
    remote = "https://github.com/iotaledger/toolchains.git",
)

git_repository(
    name = "entangled",
    commit = "fe3929b8ac6e7377eed82b83aad96369b42d0641",
    remote = "https://github.com/DLTcollab/entangled",
)

git_repository(
    name = "io_bazel_rules_docker",
    remote = "https://github.com/bazelbuild/rules_docker.git",
    tag = "v0.9.0",
)

new_git_repository(
    name = "mbedtls_2_16_6",
    build_file = "//third_party:mbedtls.BUILD",
    remote = "https://github.com/ARMmbed/mbedtls.git",
    tag = "mbedtls-2.16.6",
)

load("@rules_iota//:defs.bzl", "iota_deps")
load("@io_bazel_rules_docker//repositories:repositories.bzl", container_repositories = "repositories")

container_repositories()

load("@io_bazel_rules_docker//cc:image.bzl", _cc_image_repos = "repositories")
load("@io_bazel_rules_docker//container:pull.bzl", "container_pull")

container_pull(
    name = "ubuntu1804",
    registry = "l.gcr.io",
    repository = "google/ubuntu1804",
    tag = "latest",
)

iota_deps()

third_party_deps()

_cc_image_repos()

load("@iota_toolchains//:toolchains.bzl", "setup_initial_deps")

setup_initial_deps()

load("@iota_toolchains//:defs.bzl", "setup_toolchains_repositories")

setup_toolchains_repositories()
