#!/usr/bin/env bash

set -euo pipefail

# Create endpoint app
make EP_TA_HOST=node.deviceproof.org EP_TA_PORT=5566 legato

# Run endpoint app test here
endpoint/_build_endpoint/localhost/app/endpoint/staging/read-only/bin/endpoint
