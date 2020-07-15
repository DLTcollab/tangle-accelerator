# Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
# All Rights Reserved.
# This is free software; you can redistribute it and/or modify it under the
# terms of the MIT license. A copy of the license can be found in the file
# "LICENSE" at the root of this distribution.

export LEGATO_ROOT := $(PWD)/legato
export PATH := $(LEGATO_ROOT)/bin:$(LEGATO_ROOT)build/localhost/framework/bin:$(PATH)
export LEGATO_TARGET := localhost

platform-build-command = \
	sh -c "endpoint/build-legato.sh"; \
	cd endpoint && mkapp -t $(LEGATO_TARGET) -C -DENABLE_ENDPOINT_TEST $(LEGATO_FLAGS) endpoint.adef;
