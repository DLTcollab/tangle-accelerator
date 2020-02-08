# Building Options

## Build Docker Images

If you prefer building a docker image, tangle-accelerator also provides build rules for it. Note that you still have to edit configurations in `accelerator/config.h`.

```
$ make && bazel run //accelerator:ta_image
```

There's also an easier option to pull image from docker hub then simply run with default configs. Please do remember a redis-server is still required in this way.

```
$ docker run -d --net=host --name tangle-accelerator dltcollab/tangle-accelerator
```

## Build and Push Docker Image to Docker Hub

Before pushing the docker image to Docker Hub, you need to log in the docker registry:

```
$ docker login
```

Then you could push the docker image with the following command:

```
$ make && bazel run //accelerator:push_docker
```

If you get the following error message:

```
SyntaxError: invalid syntax
----------------
Note: The failure of target @containerregistry//:digester (with exit code 1) may have been caused by the fact that it is running under Python 3 instead of Python 2. Examine the error to determine if that appears to be the problem. Since this target is built in the host configuration, the only way to change its version is to set --host_force_python=PY2, which affects the entire build.

If this error started occurring in Bazel 0.27 and later, it may be because the Python toolchain now enforces that targets analyzed as PY2 and PY3 run under a Python 2 and Python 3 interpreter, respectively. See https://github.com/bazelbuild/bazel/issues/7899 for more information.
------------
```

Use the `--host_force_python=PY2` parameter to force the Bazel to use the Python2 in entire build.

```
$ make && bazel run //accelerator:push_docker --host_force_python=PY2
```

## Enable MQTT connectivity
MQTT connectivity is an optional feature allowing IoT endpoint devices to collaborate with `tangle-accelerator`.

```
make MQTT && bazel run --define mqtt=enable //accelerator
```

Note you may need to set up the `MQTT_HOST` and `TOPIC_ROOT` in `config.h` to connect to a MQTT broker, or you can use CLI option `--mqtt_host`, and  `--mqtt_root` to set MQTT broker address and MQTT topic root, respectively.
For more information for MQTT connectivity of `tangle-accelerator`, you could read `connectivity/mqtt/usage.md`.

## Enable external database for transaction reattachment
Transaction reattachment is an optional feature.

You can enable it in the build time with command :

```
make && bazel run --define db=enable //accelerator
```

When enabling reattachment, every transaction issues from the `tangle-accelerator` API called `Send Transfer Message` will be stored in the specific ScyllaDB host and response a UUID string for each transfer message as the identifier. With a promoting process that monitors the status of storing transactions, persistent pending transactions will be reattached to the Tangle.

Transaction reattachment relies on ScyllDB, you need to install the dependency by following commands.

For Ubuntu Linux 16.04/x86_64:

```
wget https://downloads.datastax.com/cpp-driver/ubuntu/16.04/cassandra/v2.14.1/cassandra-cpp-driver_2.14.1-1_amd64.deb
wget https://downloads.datastax.com/cpp-driver/ubuntu/16.04/cassandra/v2.14.1/cassandra-cpp-driver-dev_2.14.1-1_amd64.deb
sudo dpkg -i cassandra-cpp-driver_2.14.1-1_amd64.deb
sudo dpkg -i cassandra-cpp-driver-dev_2.14.1-1_amd64.deb
```

For Ubuntu Linux 18.04/x86_64:

```
wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/cassandra/v2.14.1/cassandra-cpp-driver_2.14.1-1_amd64.deb
wget https://downloads.datastax.com/cpp-driver/ubuntu/18.04/cassandra/v2.14.1/cassandra-cpp-driver-dev_2.14.1-1_amd64.deb
sudo dpkg -i cassandra-cpp-driver_2.14.1-1_amd64.deb
sudo dpkg -i cassandra-cpp-driver-dev_2.14.1-1_amd64.deb
```