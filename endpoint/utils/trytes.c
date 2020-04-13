/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "trytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQ_BODY                                                           \
  "{\"value\": 0, \"tag\": \"POWEREDBYTANGLEACCELERATOR9\", \"message\": " \
  "\"%s\", \"address\":\"%s\"}\r\n\r\n"

void gen_rand_trytes(const uint16_t len, uint8_t *out) {
  const char tryte_alphabet[] = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const int alphabet_array_length = sizeof(tryte_alphabet) / sizeof(char);
  for (int i = 0; i < len; i++) {
    uint8_t rand_index = rand() % alphabet_array_length;
    out[i] = tryte_alphabet[rand_index];
  }
}

void gen_trytes_message(const char *tryte_msg, const uint8_t *addr, char req_body[1024]) {
  memset(req_body, 0, sizeof(char) * 1024);
  snprintf(req_body, 1024, REQ_BODY, tryte_msg, addr);
}
