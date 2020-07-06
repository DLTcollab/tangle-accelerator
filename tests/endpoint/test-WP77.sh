#!/usr/bin/env bash

set -uo pipefail

COMMON_FILE="tests/endpoint/common.sh"

if [ ! -f "$COMMON_FILE" ]
then
    echo "$COMMON_FILE is not exists."
    exit 1
fi
source $COMMON_FILE

# setup WP77 leaf shell
setup_leaf "swi-wp77_3.4.0"

make TESTS=true EP_TARGET=wp77xx EP_TA_HOST=node.deviceproof.org EP_TA_PORT=5566 legato && \
tar zcf endpoint.tgz endpoint/_build_endpoint/wp77xx/app/endpoint/staging/read-only/
