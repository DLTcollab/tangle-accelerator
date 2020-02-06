# Tangle-accelerator

[![Build Status](https://badge.buildkite.com/0deb4c46f2f69363e4d326014843b92853733f243f379c70b5.svg)](https://buildkite.com/dltcollab/tangle-accelerator-test)  [![GitHub release](https://img.shields.io/github/release-pre/DLTcollab/tangle-accelerator.svg)](https://github.com/DLTcollab/tangle-accelerator/releases)

`Tangle-accelerator` is a caching proxy server for [IOTA](https://www.iota.org/), which
can cache API requests and rewrite their responses as needed to be routed through full
nodes. In other words, one instance of `Tangle-accelerator` can serve thousands of IOTA
requests at once without accessing remote full nodes frequently, that improves the
scalability and usability of [Tangle network](https://www.iota.org/research/meet-the-tangle).

Being at the edge as a key-value store, an edge-caching node powered by `Tangle-accelerator`
does not have to communicate to typical [IOTA](https://www.iota.org/) full nodes for every API
calls. Instead, the cached transaction data being sought is available as needed.

As an intermediate server accelerateing interactions with the Tangle, it faciliates
[dcurl](https://github.com/DLTcollab/dcurl) to perform hardware-accelerated PoW operations
on edge devices. In the meanwhile, `Tangle-accelerator` provides shortcuts for certain
use scenarios such as MAM and [TangleID](https://tangleid.github.io/).

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

### Transaction reattachment

`Tangle-accelerator` helps to reattach pending transactions were attached from `Tangle-accelerator`.
Reattachment increases chances of confirmation and prevents messages being pruned when full nodes perform snapshot.
Clients should provide a uniqle ID as the identifier to each message and it's corresponding transaction hash since a new transaction hash will be generated after reattachement. 

`Tangle-accelerator` uses ScyllaDB to store each transaction's ID, hash and status(Pending or confirmed). `Tangle-accelerator` will periodically check the status of pending transactions and reattach transactions which have been pended too long. Confirmed transactions will be stored into permanodes.

Clients can find the transaction alone with wanted message by using the ID to query.

## Connectivity

`Tangle-accelerator`, at this moment, supports the following TCP/IP derived protocols:
* `HTTP`
* `MQTT` 

### HTTP
`HTTP` can be used in the normal internet service. User can use RESTful APIs to interact with `tangle-accelerator`.

### MQTT
`MQTT` is a lightweight communication protocol which can be used in the IoT scenarios. `Tangle-accelerator`'s support to `MQTT` allows embedded devices to write data on IOTA internet with relative low quality hardware devices. We hope this will speed up DLT into our daily life.

## Documentation

This page contains basic instructions for setting up tangle-accelerator, You can generate full documentation and API reference via Doxygen. The documentation is under `docs/` after generated:

```
$ doxygen Doxyfile
```

## Requirement

Tangle-accelerator is built and launched through Bazel, it also requires Redis to cache in-memory data. Please make sure you have following tools installed:

* [Bazel](https://docs.bazel.build/versions/master/install.html)
* [Redis-server](https://redis.io/topics/quickstart)
* cmake (required by dcurl)
* openssl-dev (required by mosquitto)

## Build from Source

Before running tangle-accelerator, please edit binding address/port of accelerator instance, IRI, and redis server in `accelerator/config.h` unless they are all localhost and/or you don't want to provide external connection. With dependency of [entangled](https://github.com/iotaledger/entangled), IRI address doesn't support https at the moment. Here are some configurations you might need to change:

* `TA_HOST`: binding address of accelerator instance
* `TA_PORT`: port of accelerator instance
* `IRI_HOST`: binding address of IRI
* `IRI_PORT`: port of IRI

```
$ make && bazel run //accelerator
```
### Building Options
Tangle-accelerator supports several different build time options.

* Docker images
* MQTT connectivity
* External database

See [docs/build.md](https://github.com/DLTcollab/tangle-accelerator/docs/build.md) for more information.

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
4. make a soft link or move the executable file under `/usr/bin` 

### clang-format
clang-format can be installed by command:
- Debian/Ubuntu based systems: `$ sudo apt-get install clang-format`
- macOS: `$ brew install clang-format`


## Usage
`Tangle-accelerator` currently supports two categories of APIs
* direct API: check [wiki page](https://github.com/DLTcollab/tangle-accelerator/wiki) for details.
* Proxy API to IRI core functionalities

### IRI Porxy API
`tangle-accelerator` allows the use of IRI core APIs. The calling process does not have to be aware of the destination machine running IRI. With the exactly same format of IRI API, `tangle-accelerator` would help users forward the request to IRI and forward the response back to users.
We support two way to forward Proxy APIs to IRI:
1. Bypass Proxy APIs directly to IRI.
2. Process the Proxy APIs, then transmit them to IRI.

The user can choose which way they want with CLI argument `--proxy_passthrough`.
All the Proxy APIs are supported with the first way.
However, the second way currently only supports the followings Proxy APIs:
* checkConsistency
* findTransactions
* getBalances
* getInclusionStates
* getNodeInfo
* getTrytes

## Licensing
`Tangle-accelerator` is freely redistributable under the MIT License. Use of this source
code is governed by a MIT-style license that can be found in the `LICENSE` file.
