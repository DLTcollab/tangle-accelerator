DCURL_DIR := third_party/dcurl
DCURL_LIB := $(DCURL_DIR)/build/libdcurl.so
MOSQUITTO_DIR := third_party/mosquitto
MOSQUITTO_LIB := $(MOSQUITTO_DIR)/lib/libmosquitto.so.1
PEM_DIR = pem
# Default pem file. See pem/README.md for more information
PEM := $(PEM_DIR)/cert.pem
NAMESERVER := 8.8.8.8
RESOLV_CONF_DIR := endpoint/endpointComp
OUTPUT_BASE_DIR := output_base
# Endpoint build target. The default intends to the platform of your development system.
TARGET := simulator
# Build test suite
TESTS := false
# Enable endpoint HTTPS connection to tangle-accelerator. 
# The endpoint uses HTTP connection to transmit encrypted data by default.
# See "HTTPS Connection Support" in docs/endpoint.md for more information.
ENFORCE_EP_HTTPS := false
# The CFLAGS to pass during endpoint app build
ENDPOINT_CFLAGS :=
DEPS += $(DCURL_LIB)

# Determine to enable HTTPS connection
ifeq ($(ENFORCE_EP_HTTPS), true)
	ENDPOINT_CFLAGS += -C -DENDPOINT_HTTPS
endif

# Determine to build test suite
ifeq ($(TESTS), true)
	ENDPOINT_CFLAGS += -C -DENABLE_ENDPOINT_TEST
endif

ifdef EP_TA_HOST
	ENDPOINT_CFLAGS += -C -DEP_TA_HOST=${EP_TA_HOST}
endif

ifdef EP_TA_PORT
	ENDPOINT_CFLAGS += -C -DEP_TA_PORT=${EP_TA_PORT}
endif

ifdef EP_SSL_SEED
	ENDPOINT_CFLAGS += -C -DEP_SSL_SEED=${EP_SSL_SEED}
endif

include endpoint/platform/$(TARGET)/build.mk

all: $(DEPS) cert

.PHONY: $(DCURL_LIB) $(MOSQUITTO_LIB) legato cert check_pem

$(DCURL_LIB): $(DCURL_DIR)
	git submodule update --init $^
	$(MAKE) -C $^ config
	@echo
	$(info Modify $^/build/local.mk for your environments.)
	$(MAKE) -C $^ all

MQTT: $(DCURL_LIB) $(MOSQUITTO_LIB) cert
$(MOSQUITTO_LIB): $(MOSQUITTO_DIR)
	git submodule update --init $^
	@echo
	$(MAKE) -C $^ WITH_DOCS=no

# Build endpoint Legato app
legato: cert
	# Generate resolv.conf
	echo "nameserver $(NAMESERVER)" > $(RESOLV_CONF_DIR)/resolv.conf
	# Fetch the required external source code
	# FIXME: Use 'fetch' instead of 'build' to avoid extra building actions.
	#        The 'build' option is for getting the header file like 'mam/mam/mam_endpoint_t_set.h',
	#        which can not be downloaded when the 'fetch' option is used.
	bazel --output_base=$(OUTPUT_BASE_DIR) build //endpoint:libendpoint.so
	# Generate endpoint Legato app
	$(call platform-build-command)

cert: check_pem
	@xxd -i $(PEM) > $(PEM_DIR)/ca_crt.inc
	@sed -E \
		-e 's/(unsigned char)(.*)(\[\])(.*)/echo "\1 ca_crt_pem\3\4"/ge' 	    \
		-e 's/(unsigned int)(.*)(=)(.*)/echo "\1 ca_crt_pem_len \3\4"/ge' 	    \
		 -e 's/^unsigned/static &/' \
		 -e 's/(.*)(pem_len = )([0-9]+)(.*)/echo "\1\2$$((\3+1))\4"/ge' \
		 -e 's/(0[xX][[[:xdigit:]]+)$$/\1, 0x0/g' \
	     -i $(PEM_DIR)/ca_crt.inc
check_pem:
ifndef PEM
	$(error PEM is not set)
endif

clean:
	$(MAKE) -C $(DCURL_DIR) clean
	$(MAKE) -C $(MOSQUITTO_DIR) clean

distclean: clean
	$(RM) -r $(DCURL_DIR)
	git checkout $(DCURL_DIR)
