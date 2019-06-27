make

redis-server &

bazel run -- accelerator --ta_port=$1&
TA=$!
sleep $2 # TA takes time to be built

pip install --user -r tests/regression/requirements.txt
python3 tests/regression/runner.py $3 $4 $5
rc=$?

if [ $rc -ne 0 ]
then
    exit -1;
fi

wait $(kill -9 $TA)
trap 'exit 0' SIGTERM
