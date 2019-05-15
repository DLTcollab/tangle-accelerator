make

redis-server &

bazel run //accelerator &
TA_pid=$!
sleep 5

pip install --user -r tests/regression/requirements.txt

