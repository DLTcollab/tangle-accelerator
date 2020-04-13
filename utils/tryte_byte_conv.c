/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "tryte_byte_conv.h"
#include <stdint.h>
#include <stdio.h>

void bytes_to_trytes(unsigned char const *const input, uint16_t input_len, char *output) {
  const char tryte_alphabet[] = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  uint8_t dec = 0, lower = 0, upper = 0;

  for (uint16_t i = 0; i < input_len; i++) {
    dec = input[i];
    upper = (dec >> 4) & 15;
    lower = dec & 15;
    output[2 * i] = tryte_alphabet[upper];
    output[2 * i + 1] = tryte_alphabet[lower];
  }
}

void trytes_to_bytes(unsigned char const *const input, uint32_t input_len, char *const output) {
  uint8_t upper = 0, lower = 0;

  for (uint16_t i = 0; i < input_len; i += 2) {
    if (input[i] == '9') {
      upper = 0;
    } else {
      upper = input[i] - 64;
    }
    if (input[i + 1] == '9') {
      lower = 0;
    } else {
      lower = input[i + 1] - 64;
    }

    output[i / 2] = (upper << 4) | lower;
  }
}
