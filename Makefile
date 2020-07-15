DCURL_DIR := third_party/dcurl
DCURL_LIB := $(DCURL_DIR)/build/libdcurl.so
MOSQUITTO_DIR := third_party/mosquitto
MOSQUITTO_LIB := $(MOSQUITTO_DIR)/lib/libmosquitto.so.1
PEM_DIR = pem
# Default pem file. See pem/README.md for more information
PEM := $(PEM_DIR)/cert.pem
OUTPUT_BASE_DIR := output_base
# Endpoint build target. The default intends to the platform of your development system.
EP_TARGET := simulator
# Build test suite
TESTS := false
# Enable endpoint HTTPS connection to tangle-accelerator. 
# The endpoint uses HTTP connection to transmit encrypted data by default.
# See "HTTPS Connection Support" in docs/endpoint.md for more information.
ENFORCE_EP_HTTPS := false
# The flags that will be preprocessed by mkapp program, part of Legato Application Framework. 
# Eventually, the processed flags are passed as compiler-time options.
LEGATO_FLAGS :=
DEPS += $(DCURL_LIB)

# Determine to enable HTTPS connection
ifeq ($(ENFORCE_EP_HTTPS), true)
	LEGATO_FLAGS += -DENDPOINT_HTTPS
endif

# Determine to build test suite
ifeq ($(TESTS), true)
	LEGATO_FLAGS += -DENABLE_ENDPOINT_TEST
endif

# The tangle-acclerator host for endpoint to connect
ifdef EP_TA_HOST
	LEGATO_FLAGS += -DEP_TA_HOST=${EP_TA_HOST}
endif
# The tangle-acclerator port for endpoint to connect
ifdef EP_TA_PORT
	LEGATO_FLAGS += -DEP_TA_PORT=${EP_TA_PORT}
endif
# The ssl seed for endpoint (optional)
ifdef EP_SSL_SEED
	LEGATO_FLAGS += -DEP_SSL_SEED=${EP_SSL_SEED}
endif

# Pass target into endpoint build process
LEGATO_FLAGS += -DEP_TARGET=$(EP_TARGET)

# Prepend the "-C" flag at the beginging for passing cflags into mkapp
LEGATO_FLAGS := $(foreach flags, $(LEGATO_FLAGS), -C $(flags))

# Include the build command from the specific target
ifeq ($(EP_TARGET), simulator)
    include endpoint/platform/simulator/build.mk
else 
    include endpoint/platform/default/build.mk
endif 

export EP_TARGET

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
