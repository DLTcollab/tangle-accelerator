DCURL_DIR := third_party/dcurl
DCURL_LIB := $(DCURL_DIR)/build/libdcurl.so
DEPS += $(DCURL_LIB)

all: $(DEPS)

.PHONY: $(DCURL_LIB)

$(DCURL_LIB): $(DCURL_DIR)
	git submodule update --init $^
	$(MAKE) -C $^ config
	@echo
	$(info Modify $^/build/local.mk for your environments.)
	$(MAKE) -C $^ all

clean:
	$(MAKE) -C $(DCURL_DIR) clean

distclean: clean
	$(RM) -r $(DCURL_DIR)
	git checkout $(DCURL_DIR)
