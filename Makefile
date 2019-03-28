DCURL_DIR := third_party/dcurl
DCURL_LIB := $(DCURL_DIR)/build/libdcurl.so
HIREDIS_DIR := third_party/hiredis
HIREDIS_LIB := $(HIREDIS_DIR)/build/libdhiredis.a
DEPS += $(DCURL_LIB) $(HIREDIS_LIB)

all: $(DEPS)

.PHONY: $(DCURL_LIB)

$(DCURL_LIB): $(DCURL_DIR)
	git submodule update --init $^
	$(MAKE) -C $^ config
	@echo
	$(info Modify $^/build/local.mk for your environments.)
	$(MAKE) -C $^ all

$(HIREDIS_LIB): $(HIREDIS_DIR)
	git submodule update --init $^
	$(MAKE) -C $^ static

clean:
	$(MAKE) -C $(DCURL_DIR) clean
	$(MAKE) -C $(HIREDIS_DIR) clean

distclean: clean
	$(RM) -r $(DCURL_DIR) $(HIREDIS_DIR)
	git checkout $(DCURL_DIR) $(HIREDIS_DIR)
