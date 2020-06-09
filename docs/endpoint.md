# Endpoint

The endpoint is one of the components provided by Tangle-accelerator, running on a resource-constrained network connectivity module. The embedded devices can send messages to blockchain network (Tangle) with a connectivity module loaded endpoint. The message would be transmitted to connectivity module through UART. Message would be encrypted and send to tangle.

# Streaming Message Channel Implementation

The encrypted message would be sent to Tangle with a streaming message channel API. The streaming message channel API would ensure the order of messages in the channel. The user who wants to fetch/send message to Tangle needs to provide `data_id`, `key` and `protocol` to identify a specific message.
A message sent by endpoint needs to be encrypted locally, which avoids message being peeked and modified. 

# How to build endpoint

## Setup Legato application framework development environment 

The endpoint uses the Legato application framework as based runtime system. Developers need to set up the Sierra development environment to build endpoint as specific target.

### How to build endpoint application for wp77xx

[Leaf](https://docs.legato.io/latest/toolsLeaf.html) is a workspace manager that will download, install and configure the required software packages for a Legato development environment.

#### prerequisite packages required by leaf

```shell
$ sudo apt install \
    python3-argcomplete \
    python3-colorama \
    python3-gnupg \
    python3-jsonschema \
    python3-requests \
    gnupg \
    bash-completion \
    xz-utils
```

Install leaf

```shell
$ curl -sLO https://downloads.sierrawireless.com/tools/leaf/leaf_latest.deb && sudo dpkg -i leaf_latest.deb
```

Create a workspace

```shell
$ mkdir -p workspace
$ cd workspace
```

Setup the wp77xx target profile

```shell
$ leaf setup legato-stable -p swi-wp77_3.0.0
```

Finally, make the wp77xx endpoint target. Be careful with the directory of tangle-accelerator. It should be located within the workspace directory.

```shell
$ git clone https://github.com/DLTcollab/tangle-accelerator.git
$ cd tangle-accelerator
$ make TARGET=wp77xx legato # build endpoint as wp77xx target
$ make TESTS=true TARGET=wp77xx legato # build endpoint as wp77xx target in test mode
```

### How to build endpoint application for native target

Install required packages:

```shell
$ sudo apt-get install -y   \
    autoconf                \
    automake                \
    bash                    \
    bc                      \
    bison                   \
    bsdiff                  \
    build-essential         \
    chrpath                 \
    cmake                   \
    cpio                    \
    diffstat                \
    flex                    \
    gawk                    \
    gcovr                   \
    git                     \
    gperf                   \
    iputils-ping            \
    libbz2-dev              \
    libcurl4-gnutls-dev     \
    libncurses5-dev         \
    libncursesw5-dev        \
    libsdl-dev              \
    libssl-dev              \
    libtool                 \
    libxml2-utils           \
    ninja-build             \
    python                  \
    python-git              \
    python-jinja2           \
    python-pkg-resources    \
    python3                 \
    texinfo                 \
    unzip                   \
    wget                    \
    zlib1g-dev
```

Create a workspace

```shell
$ mkdir -p workspace
$ cd workspace
```

Clone a specific version of `legato`. The `19.07.0` is the preferred stable version.

```shell
$ repo init -u git://github.com/legatoproject/manifest -m legato/releases/19.07.0.xml # specific legato 19.07.0 version
$ repo sync
```

Build legato as native target

```shell
$ cd legato
$ make localhost
```

Checkout the shell to legato shell

```shell
$ source framework/tools/scripts/configlegatoenv
```

Finally, use GNU Make to build endpoint application at the root directory of tangle-accelerator.

```shell
$ cd tangle-accelerator
$ make legato # build endpoint as native target
```

The endpoint will be built at `endpoint/_build_endpoint/localhost/app/endpoint/staging/read-only/bin/endpoint`

## HTTPS Connection Support

The endpoint uses HTTP connection as default. The message sent to tangle-accelerator has been encrypted, so the HTTP connection would not be unsafe. To build with HTTPS connection support, add `ENFORCE_EP_HTTPS=true` option.

For HTTPS connection support, the PEM file should also be set. The default pem file is located at `pem/cert.pem`. If the `PEM` is not set, the build system will use the default pem. The endpoint will verify the connection server with the trusted certificate from the pem file. The default pem is only for the build system. The user should provide the certificate from the server you want to connect. See pem/README.md for more information. 

```shell
$ make ENFORCE_EP_HTTPS=true PEM=/path/to/pem legato
```
