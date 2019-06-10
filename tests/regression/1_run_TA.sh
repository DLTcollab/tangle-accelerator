make

redis-server &

bazel run -- accelerator --ta_port=$1&
sleep 5

pip install --user -r tests/regression/requirements.txt

