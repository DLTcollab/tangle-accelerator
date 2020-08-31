/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common/ta_errors.h"
#include "endpoint/OBDComp/can-bus/can-utils.h"
#include "endpoint/OBDComp/obd_pid.h"
#include "obd_emulator.h"

int service_01_response(int fd, int pid);

void print_help(void) { printf("Usage: obd_emulator <can_interface>\n"); }

int main(int argc, char* argv[]) {
  char* default_can_device = OBD_EMULATOR_INTERFACE;
  if (argc >= 2 && strncmp(argv[1], "-h", strlen("-h")) == 0) {
    print_help();
    exit(EXIT_SUCCESS);
  }

  if (argc >= 2 && argv[1] != NULL) {
    default_can_device = argv[1];
  }

  int s;
  status_t ret;
  ret = can_open(default_can_device, &s);
  if (ret != SC_OK) {
    fprintf(stderr, "%s\n", ta_error_to_string(ret));
    exit(EXIT_FAILURE);
  }

  printf("OBDII emulator start!\n");

  struct can_frame frame;
  do {
    ret = can_recv(s, &frame);
    if (ret != SC_OK) {
      fprintf(stderr, "%s\n", ta_error_to_string(ret));
      continue;
    }
    printf("0x%03X [%d] ", frame.can_id, frame.can_dlc);
    for (int i = 0; i < frame.can_dlc; i++) printf("%02X ", frame.data[i]);
    printf("\r\n");

    if (frame.can_id == 0x7DF) {
      printf("Received OBD query\r\n");
      if (frame.can_dlc >= 2) {
        switch (frame.data[1]) {
          case 1:
            printf("Service 01: Show current data\r\n");
            service_01_response(s, frame.data[2]);
            break;
          default:
            printf("Unknown service 0x%02X", frame.data[1]);
            break;
        }
      } else {
        printf("Error, frame too short. DLC = %d bytes.\r\n", frame.can_dlc);
      }
    }
  } while (1);

  can_close(s);
  return 0;
}

static int rand_int(int max, int min) {
  srand(time(NULL));
  int x = rand() % (max - min + 1) + min;
  return x;
}

int service_01_response(int s, int pid) {
  struct can_frame frame;
  memset(&frame, 0, sizeof(frame));
  frame.can_id = 0x7E8;
  frame.can_dlc = 8;

  printf("Service 1 PID = 0x%02X\r\n", pid);

  frame.data[0] = 2;     // Number of additional bytes
  frame.data[1] = 0x41;  // Custom Service/Mode (Same as query + 0x40)
  frame.data[2] = pid;   // PID
  int max = 5, min = -5; // Set the noise ±5  

  // Generate random data
  frame.data[3] = 0x0F + rand_int(max, min);
  frame.data[4] = 0x0F + rand_int(max, min);
  frame.data[5] = 0x0F + rand_int(max, min);
  frame.data[6] = 0x0F + rand_int(max, min);
  can_send(s, &frame);
  return 0;
}
