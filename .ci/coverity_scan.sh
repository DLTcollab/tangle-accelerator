#!/usr/bin/env bash

# Download Coverity Build Tool
wget -q https://scan.coverity.com/download/cxx/linux64 --post-data "token=$TOKEN&project=DLTcollab/tangle-accelerator" -O cov-analysis-linux64.tar.gz
mkdir cov-analysis-linux64
tar xzf cov-analysis-linux64.tar.gz --strip 1 -C cov-analysis-linux64

# Build with cov-build
export PATH=`pwd`/cov-analysis-linux64/bin:$PATH
cov-build --dir cov-int bash tests/coverity_analysis.sh

# Submit the result to Coverity Scan
tar czvf tangle-accelerator.tgz cov-int
curl \
    --form project=DLTcollab/tangle-accelerator \
    --form token=$TOKEN \
    --form email=yanghau@biilabs.io \
    --form file=@tangle-accelerator.tgz \
    --form version=trunk \
    --form description="Tangle-accelerator" \
    https://scan.coverity.com/builds?project=DLTcollab/tangle-accelerator
