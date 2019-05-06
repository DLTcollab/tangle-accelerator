git fetch origin develop
git checkout -b pullrequest FETCH_HEAD

make

bazel run //accelerator &
pid=$!
sleep 60
kill "$pid"
