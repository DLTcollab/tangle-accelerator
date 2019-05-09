make

bazel run //accelerator &
TA_pid=$!
sleep 20

pip install --user -r tests/regression/requirements.txt

