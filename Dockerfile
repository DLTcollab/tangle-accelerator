# docker build -t mytest . --no-cache -f Dockerfile

FROM ubuntu:18.04

# Install Dependencies
RUN apt update && apt install -y curl gnupg
RUN curl https://bazel.build/bazel-release.pub.gpg | apt-key add -
RUN echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
RUN apt update && apt install -y bazel cmake git build-essential xxd uuid-dev
RUN apt upgrade -y

WORKDIR /workdir
COPY . /workdir

EXPOSE 8000
RUN make && bazel build //accelerator --incompatible_restrict_string_escapes=false