# Building Options

## Build Docker Images

If you prefer building a docker image, tangle-accelerator also provides build rules for it. Note that you still have to edit configurations in `accelerator/config.h`.

```
$ make && bazel run //accelerator:docker_image
```

The docker image will be gernerated in local machine.

```bash
$ docker images
REPOSITORY                    TAG                 IMAGE ID
dltcollab/accelerator         docker_image        71ea01606000
```

## Push Docker Image to Docker Hub

Before pushing the docker image to Docker Hub, you need to log in the docker registry:

```
$ docker login
```

Then you could modify docker image `REPOSITORY` and `TAG` as you like.

```
$ docker tag dltcollab/accelerator:docker_image dltcollab/tangle-accelerator:v0.9.1
$ docker images
REPOSITORY                    TAG                 IMAGE ID
dltcollab/tangle-accelerator  v0.9.1              71ea01606000
dltcollab/accelerator         docker_image        71ea01606000
```

Then you could push the docker image to Docker Hub.

```
$ docker push dltcollab/tangle-accelerator:v0.9.1
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