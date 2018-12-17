# Tangle-accelerator

`Tangle-accelerator` is an intermediate server accelerateing interactions 
with the Tangle. It behaves like swarm nodes which can provide expected
services as full node for most use cases.

## Building

Tangle-accelerator is built and run through [bazel](https://www.bazel.build/):

```
$ bazel run //tangle_accelerator
```

## Developing

- Please run `./hooks/autohook.sh install` after initial checkout.
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
- Debian/Ubuntu based: `$ sudo apt-get install clang-format`
- OSX: `$ brew install clang-format`

## Licensing
`Tangle-accelerator` is freely redistributable under the MIT License. Use of this source
code is governed by a MIT-style license that can be found in the `LICENSE` file.
