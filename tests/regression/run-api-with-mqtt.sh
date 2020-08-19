#!/usr/bin/env bash

source tests/regression/common.sh

check_env
setup_build_opts

# Get command line arguments
# Current arguments parsed are <socket name> <remaining_args>
get_cli_args $@

# Install prerequisites
make MQTT
pip3 install --user -r tests/regression/requirements.txt

# FIXME: Check Redis status
redis-server &

# Iterate over all available build options
for ((i = 0; i < ${#OPTIONS[@]}; i++)); do
	option=${OPTIONS[${i}]}
	cli_arg=${option} | cut -d '|' -f 1
	build_arg=${option} | cut -d '|' -f 2

	bazel run accelerator --define mqtt=enable ${build_arg} -- --quiet --ta_port=${TA_PORT} ${cli_arg} --proxy_passthrough &
	TA=$!
	trap "kill -9 ${TA};" INT # Trap SIGINT from Ctrl-C to stop TA

	# Wait until tangle-accelerator has been initialized
	echo "==============Wait for TA starting=============="
	while read -r line; do
		if [[ "$line" == "$start_notification" ]]; then
			echo "$line"
		fi
	done <<<$(nc -U -l $socket | tr '\0' '\n')
	echo "==============TA has successfully started=============="

	python3 tests/regression/runner.py ${remaining_args} --url "localhost" --mqtt
	rc=$?

	if [ $rc -ne 0 ]; then
		echo "Build option '${option}' failed"
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
