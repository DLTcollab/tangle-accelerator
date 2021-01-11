ROOT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../)

TARGETS := wp77xx

# Third party library
FLATCC_DIR=$(ROOT_DIR)/third_party/flatcc
MBEDTLS_DIR=$(ROOT_DIR)/third_party/mbedtls
BROTLI_DIR=$(ROOT_DIR)/third_party/brotli

OBD_SCHEMA=$(ROOT_DIR)/third_party/vehicle-scheme/vehicle_obd.fbs
ENDPOINT_SCHEMA=$(ROOT_DIR)/third_party/vehicle-scheme/endpoint.fbs
OUT=../build

FLATCC_EXE=flatcc
FLATCC_INCLUDE=$(FLATCC_DIR)/include
FLATCC_LIB=$(FLATCC_DIR)/lib
FLATCC_CMAKE_DIR=$(OUT)/flatcc

MBEDTLS_INCLUDE=$(MBEDTLS_DIR)/include
MBEDTLS_LIB=$(MBEDTLS_DIR)/library

BROTLI_INCLUDE=$(BROTLI_DIR)/c/include

INCLUDE_FLAGS=-I../ -I$(FLATCC_INCLUDE) -I$(MBEDTLS_INCLUDE) -I$(BROTLI_INCLUDE) 
LDFLAGS=-L$(FLATCC_LIB) -L$(MBEDTLS_LIB) -L$(BROTLI_DIR) -lbrotli -lmbedcrypto -lmbedx509 -lmbedtls -lflatccrt -lm

THIRD_PARTY_CFLAGS= -fPIC

# If the build target is not simulator
ifneq ($(EP_TARGET), simulator)
    UPPERCASE_TARGET = $(shell echo $(EP_TARGET) | tr '[a-z]' '[A-Z]')
    TOOLCHAINDIR := $($(UPPERCASE_TARGET)_TOOLCHAIN_DIR)
    TOOLCHAINPRE = $(TOOLCHAINDIR)/arm-poky-linux-gnueabi-
    SYSROOT = $($(UPPERCASE_TARGET)_SYSROOT)
    CC = $(TOOLCHAINPRE)gcc
    CXX = $(TOOLCHAINPRE)g++
    LD = $(TOOLCHAINPRE)ld
    AR = $(TOOLCHAINPRE)ar
    RANLIB = $(TOOLCHAINPRE)ranlib
	THIRD_PARTY_CFLAGS="--sysroot=$(SYSROOT) -fPIC"
endif

LEGATO_INCLUDE_FLAGS=$(addprefix -C , $(INCLUDE_FLAGS))
LEGATO_LDFLAGS=$(addprefix -L , $(LDFLAGS))

.PHONY: all test clean $(TARGETS)

obd_header:
	mkdir -p $(OUT)
	$(FLATCC_EXE) --outfile $(OUT)/obd_generated.h -a $(OBD_SCHEMA) || exit 1
	$(FLATCC_EXE) --outfile $(OUT)/endpoint_generated.h -a $(ENDPOINT_SCHEMA) || exit 1

mbedtls_lib:
	$(MAKE) -C $(MBEDTLS_DIR) CC=$(CC) CFLAGS=$(THIRD_PARTY_CFLAGS) lib

brotli_lib:
	$(MAKE) -C $(BROTLI_DIR) CC=$(CC) CFLAGS=$(THIRD_PARTY_CFLAGS) lib

flatcc_lib:
	mkdir -p $(FLATCC_CMAKE_DIR)
	cd $(FLATCC_CMAKE_DIR); \
	cmake $(FLATCC_DIR) \
	 -DFLATCC_TEST=off \
	 -DCMAKE_BUILD_TYPE=Release \
	 -DCMAKE_C_COMPILER=$(CC) \
	 -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	 -DCMAKE_FIND_ROOT_PATH=$(SYSROOT) \
	 -DCMAKE_SYSROOT=$(SYSROOT)
	$(MAKE) -C $(FLATCC_CMAKE_DIR)

clean:
	-$(MAKE) -C $(MBEDTLS_DIR) clean
	-$(MAKE) -C $(BROTLI_DIR) clean
	rm -rf $(OUT) 
	-rm -rf _build_* *.*.update

simulator: export LEGATO_ROOT=$(PWD)/legato
simulator: obd_header mbedtls_lib flatcc_lib brotli_lib
	export PATH=$(LEGATO_ROOT)/bin:$(LEGATO_ROOT)build/localhost/framework/bin:$(PATH) ; \
	export LEGATO_TARGET=localhost ; \
	mkapp -v -t localhost \
		  $(LEGATO_INCLUDE_FLAGS) \
		  $(LEGATO_LDFLAGS) \
		  $(LEGATO_FLAGS) \
		  -C -DENABLE_ENDPOINT_TEST \
		  -C -fPIC \
		  -C -DEP_TARGET=simulator \
          --interface-search=$(LEGATO_ROOT)/interfaces/modemServices \
          endpoint.adef  && \
	mkapp -t localhost \
		  $(LEGATO_INCLUDE_FLAGS) \
		  $(LEGATO_LDFLAGS) \
		  $(LEGATO_FLAGS) \
		  -C -DENABLE_ENDPOINT_TEST \
		  -C -fPIC \
		  -C -DEP_TARGET=simulator \
          --interface-search=$(LEGATO_ROOT)/interfaces/modemServices \
          OBDMonitor.adef

$(TARGETS): export TARGET=$@
$(TARGETS): obd_header mbedtls_lib flatcc_lib brotli_lib
	mkapp -v -t $@ \
		  $(LEGATO_INCLUDE_FLAGS) \
		  $(LEGATO_LDFLAGS) \
		  $(LEGATO_FLAGS) \
          --interface-search=$(LEGATO_ROOT)/interfaces/modemServices \
          endpoint.adef
	mkapp -t $@ \
		  $(LEGATO_INCLUDE_FLAGS) \
		  $(LEGATO_LDFLAGS) \
		  $(LEGATO_FLAGS) \
          --interface-search=$(LEGATO_ROOT)/interfaces/modemServices \
          OBDMonitor.adef
	mkapp -t $@ \
		  $(LEGATO_INCLUDE_FLAGS) \
		  $(LEGATO_LDFLAGS) \
		  $(LEGATO_FLAGS) \
          --interface-search=$(LEGATO_ROOT)/interfaces/modemServices \
          shell.adef
