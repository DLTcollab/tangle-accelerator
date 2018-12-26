FROM ubuntu:18.04

RUN apt-get update \
 && apt-get install  -y \
    automake \
    build-essential \
    ccache \
    clang-format \
    cmake \
    curl \
    doxygen \
    graphviz \
    libssl-dev \
    libtinfo5 \
    openssl \
    unzip \
    wget \
    xxd \
    zip \
    git \
    openjdk-8-jdk \
    python3-dev \
    python3-numpy \
    python-pyparsing\
    python3-pyparsing\
    python \
    gcc \
    g++ \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

ENV BAZEL_VERSION 0.20.0

RUN wget -q https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/bazel-${BAZEL_VERSION}-installer-linux-x86_64.sh -O bazel_installer.sh
RUN chmod a+x ./bazel_installer.sh && ./bazel_installer.sh --prefix=/usr \
      && rm bazel_installer.sh
COPY bazel.bazelrc /etc/bazel.bazelrc
