# Tangle-accelerator

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

## Building from Source

`Tangle-accelerator` is built and launched through [bazel](https://www.bazel.build/):

```
$ make
$ bazel run //:tangle_accelerator
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
