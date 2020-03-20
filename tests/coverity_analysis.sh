#!/bin/bash

make
bazel build //accelerator
make clean

make
bazel build --define db=enable //accelerator
make clean


make MQTT
bazel build --define mqtt=enable //accelerator
make clean
