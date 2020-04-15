DCURL_DIR := third_party/dcurl
DCURL_LIB := $(DCURL_DIR)/build/libdcurl.so
MOSQUITTO_DIR := third_party/mosquitto
MOSQUITTO_LIB := $(MOSQUITTO_DIR)/lib/libmosquitto.so.1
DEPS += $(DCURL_LIB)

all: $(DEPS)

.PHONY: $(DCURL_LIB) $(MOSQUITTO_LIB)

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

clean:
	$(MAKE) -C $(DCURL_DIR) clean
	$(MAKE) -C $(MOSQUITTO_DIR) clean

distclean: clean
	$(RM) -r $(DCURL_DIR)
	git checkout $(DCURL_DIR)
