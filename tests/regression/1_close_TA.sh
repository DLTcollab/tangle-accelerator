#!/usr/bin/env bash

wait $(ps aux | grep '[r]unner.py' | awk '{print $2}')
wait $(kill $(ps aux | grep '[r]edis' | awk '{print $2}'))
wait $(kill $(ps aux | grep '[.]/accelerator' | awk '{print $2}'))

trap 'exit 0' SIGTERM

