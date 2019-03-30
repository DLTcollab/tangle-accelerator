# Tangle-accelerator

[![Build Status](https://badge.buildkite.com/46ec07b122bde13f984c241fe8b38e64698c5c0d816ee6c7e4.svg)](https://buildkite.com/dltcollab/dcurl-test) [![Gitter](https://img.shields.io/gitter/room/DLTcollab/tangle-accelerator.svg)](https://gitter.im/DLTcollab/tangle-accelerator) ![GitHub release](https://img.shields.io/github/release-pre/DLTcollab/tangle-accelerator.svg)

`Tangle-accelerator` is a caching proxy server for [IOTA](https://www.iota.org/), which
can cache API requests and rewrite their responses as needed to be routed through full
nodes. Thus, one instance of `Tangle-accelerator` can serve thousands of Tangle requests
at once without accessing remote full nodes frequently.

As an intermediate server accelerateing interactions with the Tangle, it faciliates
[dcurl](https://github.com/DLTcollab/dcurl) to perform hardware-accelerated PoW operations
on edge devices. In the meanwhile, `Tangle-accelerator` provides shortcuts for certain
use scenarios such as MAM and [TangleID](https://github.com/TangleID).

At the moment, it is not feasible to host fully-functioned full nodes on Raspberry Pi class
Arm devices, but Raspberry Pi 3 is known to be capable to execute `Tangle-accelerator`
without problems. Since it is written in C/C++ with [entangled](https://github.com/iotaledger/entangled),
both footprint and startup time are behaved pretty well.

## Architecture

`Tangle-accelerator` as an intermediate server provides services like transaction explorer, issuing transfers and even PoW Accelerate to make attach to tangle faster. Certain API queries can store into memory cache for better searching and easier to reattach.

```
                +-------------------------------------------+
+----------+    |  +-----------------+       +-----------+  |       
|          |    |  | Service         |       | Cache     |  |
|  Client  <-----> |                 | <---> |           |  |
|          |    |  | -Explorer       |       | -Trytes   |  |
+----------+    |  | -Transfer       |       | -LFU/LRU  |  |
                |  | -PoW Accelerate |       |           |  |
                |  | -Proxy          |       |           |  |
                |  +-----------------+       +-----------+  |
                |         ^                                 |
                +---------|---------------------------------+     
                          v
                +-------------------------------------------+  
                | Full Node                                 |
                |          +----------------------+         |
                |          | Consensus            |         |
                |          +----------------------+         |
                +-------------------------------------------+

```

## Documentation

This page contains basic instructions for setting up tangle-accelerator, You can generate full documentation and API reference via Doxygen. The documentation is under `docs/` after generated:

```
$ doxygen Doxyfile
```

## Requirement

Tangle-accelerator is built and launched through Bazel, it also requires Redis to cache in-memory data. Please make sure you have following tools installed:

* [Bazel](https://docs.bazel.build/versions/master/install.html)
* [Redis-server](https://redis.io/topics/quickstart)

## Build from Source

Before running tangle-accelerator, please edit binding address/port of accelerator instance, IRI, and redis server in `accelerator/config.h` unless they are all localhost and/or you don't want to provide external connection. With dependency of [entangled](https://github.com/iotaledger/entangled), IRI address doesn't support https at the moment. Here are some configurations you might need to change:

* `TA_HOST`: binding address of accelerator instance
* `TA_PORT`: port of accelerator instance
* `IRI_HOST`: binding address of IRI
* `IRI_PORT`: port of IRI

```
$ make && bazel run //accelerator
```

### Build from docker

If you prefer building a docker image, tangle-accelerator also provides build rules for it. Note that you still have to edit configurations in `accelerator/config.h`.

```
$ make && bazel run //accelerator:ta_image
```

There's also an easier option to pull image from docker hub then simply run with default configs. Please do remember a redis-server is still required in this way.

```
$ docker run -d --net=host --name tangle-accelerator wusyong/tangel-accelerator:latest
```

## Developing

The codebase of this repository follows [Google's C++ guidelines](https://google.github.io/styleguide/cppguide.html):
- Please run `hooks/autohook.sh install` after initial checkout.
- Pass `-c dbg` for building with debug symbols.

### Tools required for running git commit hook
- buildifier
- clang-format

### Buildifier
Buildifier can be installed with `bazel` or `go`

#### Install with go
1. change directory to `$GOPATH`
2. run `$ go get github.com/bazelbuild/buildtools/buildifier`
   The executable file will be located under `$GOPATH/bin`
3. make a soft link for global usage, run
   `$ sudo ln -s $HOME/go/bin/buildifier /usr/bin/buildifier`

#### Install with bazel
1. clone `bazelbuild/buildtools` repository
   `$ git clone https://github.com/bazelbuild/buildtools.git`
2. change directory to `buildtools`
3. build it with bazel command, `$ bazel build //buildifier`
   The executable file will be located under `path/to/buildtools/bazel-bin`
4. make a soft link

### clang-format
clang-format can be installed by command:
- Debian/Ubuntu based systems: `$ sudo apt-get install clang-format`
- macOS: `$ brew install clang-format`


## Licensing
`Tangle-accelerator` is freely redistributable under the MIT License. Use of this source
code is governed by a MIT-style license that can be found in the `LICENSE` file.
