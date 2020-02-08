#!/bin/bash

source tests/regression/common.sh

check_env
setup_build_opts

# Get command line arguments
# Current arguments parsed are <sleep_time> <remaining_args>
get_cli_args $@

# Install prerequisites
make
pip install --user -r tests/regression/requirements.txt
redis-server &

# Iterate over all available build options
for (( i = 0; i < ${#OPTIONS[@]}; i++ )); do
    option=${OPTIONS[${i}]}
    cli_arg=${option} | cut -d '|' -f 1
    build_arg=${option} | cut -d '|' -f 2

    bazel run accelerator ${build_arg} -- --ta_port=${TA_PORT} ${cli_arg} &
    TA=$!
    sleep ${sleep_time} # TA takes time to be built
    trap "kill -9 ${TA};" INT # Trap SIGINT from Ctrl-C to stop TA

    python3 tests/regression/runner.py ${remaining_args} --url localhost:${TA_PORT}
    rc=$?

    if [ $rc -ne 0 ]
    then
        echo "Build option '${option}' failed"
        fail+=("${option}")
    else
        success+=("${option}")
    fi

    bazel clean
    wait $(kill -9 ${TA})
done

echo "--------- Successful build options ---------"
for (( i = 0; i < ${#success[@]}; i++ )); do echo ${success[${i}]}; done
echo "----------- Failed build options -----------"
for (( i = 0; i < ${#fail[@]}; i++ )); do echo ${fail[${i}]}; done
