DIR_BUILD = build
MBEDTLS_LIBRARY := -L$(MBEDTLS_PATH)/library
MBEDTLS_LDFLAGS := -lmbedcrypto -lmbedtls -lmbedx509
UNITY_SRC := $(UNITY_PATH)/src
UNITY_INCLUDE := -I$(UNITY_PATH)/src

all: create_dir build/test_tryte_byte_conv build/test_crypto_utils build/test_serializer

create_dir:
	[ -d $(DIR_BUILD) ] || mkdir -p $(DIR_BUILD)

build/test_tryte_byte_conv: test_tryte_byte_conv.c $(UTILS_PATH)/tryte_byte_conv.c $(UNITY_SRC)/unity.c
	$(CC) -g -DDEBUG $(INCLUDES) $(UNITY_INCLUDE) -o $@ $^

build/test_crypto_utils: test_crypto_utils.c $(UTILS_PATH)/crypto_utils.c $(UNITY_SRC)/unity.c
	$(CC) -g -DDEBUG $(INCLUDES) $(UNITY_INCLUDE) -o $@ $^  $(MBEDTLS_LIBRARY) $(MBEDTLS_LDFLAGS)

build/test_serializer: test_serializer.c $(UTILS_PATH)/serializer.c $(UNITY_SRC)/unity.c
	$(CC) -g -DDEBUG $(INCLUDES) $(UNITY_INCLUDE) -o $@ $^

clean:
	rm -f *.o
	rm -f build/*
