#!/usr/bin/env bash

set -uo pipefail
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
EP_TEST_CONFIG=(asan tsan ubsan)

function cleanup(){
    kill -9 "${TA}"
}

function success(){
    cleanup
    echo -e "${GREEN}ALL test passed${NC}"
    exit 0
}

function failed(){
    cleanup
    echo -e "${RED}Unit-test: ${TEST_CASE} not passed${NC}"
    exit 1
}

function run_test_suite(){
    bazel test -c dbg --config "$1" //endpoint/unit-test/...
    ret=$?
    if [[ ret -ne 0 ]]
    then
        TEST_CASE="$1"
        failed 
    fi 
}

function start_ta(){
    # Create tangle-accelerator for unit-test
    bazel run accelerator &
    TA=$!
    # Wait for tangle-acclerator build finish
    sleep 20 
}

echo "Start unit-test for endpoint"
start_ta

# Run endpoint unit-test
for i in ${EP_TEST_CONFIG[*]}; do 
    run_test_suite "$i"
done

# Finish 
success
