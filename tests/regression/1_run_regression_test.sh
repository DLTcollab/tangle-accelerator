#!/usr/bin/env bash

git fetch origin develop
git checkout -b pullrequest FETCH_HEAD

make

bazel run //accelerator &
TA_pid=$!
sleep 60

if [ -z "$1" ]; then
    url="localhost:8000"
else
    url="$1"
fi

sudo pip install > requirements.txt
python3 tests/regression/runner.py $url
runner_pid=$!
wait $runner_pid
kill "$TA_pid"
wait $TA_pid

trap 'exit 0' SIGTERM
