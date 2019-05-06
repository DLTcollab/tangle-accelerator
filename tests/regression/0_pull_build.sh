#!/usr/bin/env bash

git fetch origin develop
git checkout -b pullrequest FETCH_HEAD

make

bazel run //accelerator &
pid=$!

kill $pid
wait $pid

trap 'exit 0' SIGTERM
