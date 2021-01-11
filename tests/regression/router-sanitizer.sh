#!/usr/bin/env bash

source tests/regression/common.sh

check_env
setup_sanitizer_opts

# Get command line arguments
# Current arguments parsed are <socket name> <remaining_args>
get_cli_args $@

# Install prerequisites
make
pip3 install --user -r tests/regression/requirements.txt

# FIXME: Check Redis status
redis-server &

# Iterate over all available build options
for ((i = 0; i < ${#SAN_OPTIONS[@]}; i++)); do
	option=${SAN_OPTIONS[${i}]}
	socket=$(mktemp)

	bazel run accelerator ${option} -c dbg -- --ta_port=${TA_PORT} --socket=${socket} &
	TA=$!
	trap "kill -9 ${TA};" INT # Trap SIGINT from Ctrl-C to stop TA

	# if tangle-accelerator takes more than 30 secs to initialize, then exit and return failure.
	timeout 30 tests/regression/ta_waiting.sh
	ret_code=$?
	if [[ $ret_code -eq 124 ]]; then
		echo "Timeout in initializing tangle-accelerator."
		kill -9 ${TA}
		exit 1
	elif [[ $ret_code -eq 1 ]]; then
		echo "Failed to connect to UNIX domain socket."
		kill -9 ${TA}
		exit 1
	fi

	python3 tests/regression/runner.py ${remaining_args} --url ${TA_HOST}:${TA_PORT}
	rc=$?

	if [ $rc -ne 0 ]; then
		echo "Build sanitizer '${option}' failed"
		fail+=("${option}")
	else
		success+=("${option}")
	fi

	bazel clean
	wait $(kill -9 ${TA})
done

echo "--------- Successful build options ---------"
for ((i = 0; i < ${#success[@]}; i++)); do echo ${success[${i}]}; done
echo "----------- Failed build options -----------"
for ((i = 0; i < ${#fail[@]}; i++)); do echo ${fail[${i}]}; done

if [ ${#fail[@]} -gt 0 ]; then
	exit 1
fi
