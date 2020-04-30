DCURL_DIR := third_party/dcurl
DCURL_LIB := $(DCURL_DIR)/build/libdcurl.so
MOSQUITTO_DIR := third_party/mosquitto
MOSQUITTO_LIB := $(MOSQUITTO_DIR)/lib/libmosquitto.so.1
MBEDTLS_DIR := third_party/mbedtls
MBEDTLS_LIB := $(MBEDTLS_PATH)/library/libmbedx509.so $(MBEDTLS_PATH)/library/libmbedtls.so $(MBEDTLS_PATH)/library/libmbedcrypto.so
HTTPPARSER_DIR := third_party/http-parser
HTTPPARSER_LIB := $(HTTPPARSER_DIR)/libhttp_parser.so
PEM_DIR = pem
DEPS += $(DCURL_LIB) $(HTTPPARSER_LIB) $(MBEDTLS_LIB)
PEM := $(PEM_DIR)/cert.pem

all: $(DEPS) cert

.PHONY: $(DCURL_LIB) $(MOSQUITTO_LIB) cert check_pem

$(DCURL_LIB): $(DCURL_DIR)
	git submodule update --init $^
	$(MAKE) -C $^ config
	@echo
	$(info Modify $^/build/local.mk for your environments.)
	$(MAKE) -C $^ all

MQTT: $(DCURL_LIB) $(MOSQUITTO_LIB)
$(MOSQUITTO_LIB): $(MOSQUITTO_DIR)
	git submodule update --init $^
	@echo
	$(MAKE) -C $^ WITH_DOCS=no

$(HTTPPARSER_LIB): $(HTTPPARSER_DIR)
	git submodule update --init $^
	$(MAKE) -C $^ library

$(MBEDTLS_LIB): $(MBEDTLS_DIR)
	git submodule update --init $^
	$(MAKE) -C $^ SHARED=1 lib

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
	$(MAKE) -C $(HTTPPARSER_DIR) clean
	$(MAKE) -C $(MBEDTLS_DIR) clean


distclean: clean
	$(RM) -r $(DCURL_DIR)
	git checkout $(DCURL_DIR)
