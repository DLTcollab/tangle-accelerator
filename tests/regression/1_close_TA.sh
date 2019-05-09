#!/usr/bin/env bash

wait $(ps aux | grep '[r]unner.py' | awk '{print $2}')
kill $(ps aux | grep '[.]/accelerator' | awk '{print $2}')
wait $!

trap 'exit 0' SIGTERM

