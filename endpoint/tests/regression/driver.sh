#!/usr/bin/env bash

if [ "$#" -ne 3 ]; then
	echo "Usage: ./driver.sh [NODE_HOST] [NODE_PORT] [TEST_TA_PORT]" >&2
	exit 1
fi

IOTA_NODE=$1
IOTA_PORT=$2
TEST_TA_PORT=$3

set -uo pipefail
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
EP_TEST_CONFIG=(asan tsan ubsan)

function cleanup() {
	kill -9 "${TA}"
}

function success() {
	cleanup
	echo -e "${GREEN}ALL test passed${NC}"
	exit 0
}

function failed() {
	cleanup
	echo -e "${RED}Unit-test: ${TEST_CASE} not passed${NC}"
	exit 1
}

function run_test_suite() {
	bazel test -c dbg --config "$1" //endpoint/tests/regression/... --test_arg=localhost --test_arg="$TEST_TA_PORT"
	ret=$?
	if [[ ret -ne 0 ]]; then
		TEST_CASE="$1"
		failed
	fi
}

function start_ta() {
	# Create tangle-accelerator for unit-test
	bazel run //accelerator -- --ta_port="$TEST_TA_PORT" --node_host="$IOTA_NODE" --node_port="$IOTA_PORT" --cache=on &
	TA=$!
	# Wait until tangle-accelerator has been initialized
	echo "==============Wait for TA starting=============="
	while read -r line; do
		if [[ "$line" == "TA-START" ]]; then
			echo "$line"
		fi
	done <<<$(nc -U -l $socket | tr '\0' '\n')
	echo "==============TA has successfully started=============="
}

echo "Start unit-test for endpoint"
start_ta

# Run endpoint unit-test
for i in ${EP_TEST_CONFIG[*]}; do
	run_test_suite "$i"
done

# Finish
success
