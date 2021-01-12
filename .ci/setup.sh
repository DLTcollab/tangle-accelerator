#!/usr/bin/env bash

sudo apt update -y
sudo apt upgrade -y
echo -n | openssl s_client -connect https://scan.coverity.com:443 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' | sudo tee -a /etc/ssl/certs/ca-
# Add Bazel distribution URI as a package source
sudo apt install curl
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
# Install and update Bazel
sudo apt install bazel
# Install UUID library
sudo apt install uuid-dev
# Install Cassandra C client library
echo "deb https://downloads.apache.org/cassandra/debian 311x main" | sudo tee -a /etc/apt/sources.list.d/cassandra.sources.list
curl https://downloads.apache.org/cassandra/KEYS | sudo apt-key add -
sudo apt-get update
sudo apt-get install -y libuv1 libuv1-dev cassandra
