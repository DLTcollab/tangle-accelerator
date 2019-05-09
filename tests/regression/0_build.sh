#!/usr/bin/env bash

make

bazel run //accelerator &
pid=$!

kill $pid
wait $pid

trap 'exit 0' SIGTERM
