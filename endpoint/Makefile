ifeq ($(ARM), y)
CROSS = arm-linux-gnueabi-
CC = $(CROSS)gcc
CXX = $(CROSS)g++
AR = $(CROSS)ar
RANLIB = $(CROSS)ranlib
LD = $(CROSS)ld
STRIP = $(CROSS)strip
export CC CXX AR RANLIB
endif

ROOT_DIR = $(CURDIR)
THIRD_PARTY_PATH = $(ROOT_DIR)/third_party
MBEDTLS_PATH = $(THIRD_PARTY_PATH)/mbedtls
HTTP_PARSER_PATH = $(THIRD_PARTY_PATH)/http-parser
UNITY_PATH = $(THIRD_PARTY_PATH)/Unity
TEST_PATH = $(ROOT_DIR)/tests
UTILS_PATH = $(ROOT_DIR)/utils
CONNECTIVITY_PATH = $(ROOT_DIR)/connectivity
HAL_PATH = $(ROOT_DIR)/hal
export THIRD_PARTY_PATH ROOT_DIR UTILS_PATH MBEDTLS_PATH UNITY_PATH HAL_PATH

ifeq ($(DEBUG), n)
CFLAGS = -Wall -Werror -fPIC -DHAVE_CONFIG_H -D_U_="__attribute__((unused))" -O2
else
CFLAGS = -Wall -fPIC -DHAVE_CONFIG_H -D_U_="__attribute__((unused))" -g3 -DDEBUG
endif
export CFLAGS

INCLUDES = -I$(THIRD_PARTY_PATH)/http-parser -I$(MBEDTLS_PATH)/include -I$(ROOT_DIR)/connectivity -I$(ROOT_DIR)/utils -I$(HAL_PATH)
LIBS = $(MBEDTLS_PATH)/library/libmbedx509.a $(MBEDTLS_PATH)/library/libmbedtls.a $(MBEDTLS_PATH)/library/libmbedcrypto.a
export INCLUDES

DEPS :=
OBJS := main.o $(HTTP_PARSER_PATH)/http_parser.o
SUBDIR = $(UTILS_PATH) $(CONNECTIVITY_PATH)

OBJS += $(foreach x,$(SUBDIR),$(patsubst %.c,%.o,$(wildcard $(x)/*.c)))
DEPS += $(foreach x,$(SUBDIR),$(patsubst %.c,%.o.d,$(wildcard $(x)/*.c)))
export OBJS

.SUFFIXES:.c .o
.PHONY: all clean test git-hook utils

all: git-hook $(OBJS) ta_client mbedtls_make utils

ta_client: mbedtls_make utils
	@echo Linking: $@ ....
	$(CC) -o $@ $(OBJS) $(LIBS)
mbedtls_make:
	$(MAKE) -C $(MBEDTLS_PATH) lib
utils:
	$(MAKE) -C $(HTTP_PARSER_PATH) http_parser.o
test: utils
	$(MAKE) -C $(TEST_PATH)
%.o:%.c
	$(CC) -v -c $(CFLAGS) $(INCLUDES) -MMD -MF $@.d -o $@ $<

hal:
	$(MAKE) -C $(HAL_PATH)

clean: clean_client clean_third_party clean_test clean_devices

clean_test:
	$(MAKE) -C $(TEST_PATH) clean

clean_client:
	rm -f $(OBJS) ta_client *.o *.c.d

clean_devices:
	$(MAKE) -C $(HAL_PATH) clean

clean_third_party: clean_mbedtls clean_http_parser

clean_mbedtls:
	@for dir in $(MBEDTLS_PATH); do \
		$(MAKE) -C $$dir clean; \
		if [ $$? != 0 ]; then exit 1; fi; \
	done

clean_http_parser:
	$(MAKE) -C $(HTTP_PARSER_PATH) clean

git-hook:
	git config core.hooksPath hooks
