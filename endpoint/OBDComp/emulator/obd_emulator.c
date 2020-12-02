/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common/logger.h"
#include "endpoint/OBDComp/can-bus/can-utils.h"
#include "obd_emulator.h"

static int service_01_response(int fd, uint8_t pid);
static int service_09_response(int fd, uint8_t pid);

static void print_help(void) { printf("Usage: obd_emulator <can_interface>\n"); }
void emulator_exit(int status, void* arg) {
  int can_fd = *(int*)arg;
  can_close(can_fd);
}

int main(int argc, char* argv[]) {
  char* default_can_device = OBD_EMULATOR_INTERFACE;
  if (argc < 2 || strncmp(argv[1], "-h", 2) == 0) {
    print_help();
    exit(EXIT_SUCCESS);
  }

  if (argc >= 2) {
    default_can_device = argv[1];
  }

  int can_fd;
  status_t ret;
  ret = can_open(default_can_device, &can_fd);
  if (ret != SC_OK) {
    fprintf(stderr, "%s\n", ta_error_to_string(ret));
    exit(EXIT_FAILURE);
  }

  printf("OBDII emulator start!\n");
  on_exit(emulator_exit, (void*)&can_fd);

  struct can_frame frame;
  do {
    ret = can_recv(can_fd, &frame);
    if (ret != SC_OK) {
      fprintf(stderr, "%s\n", ta_error_to_string(ret));
      continue;
    }
    printf("0x%03X [%d] ", frame.can_id, frame.can_dlc);
    for (int i = 0; i < frame.can_dlc; i++) printf("%02X ", frame.data[i]);
    printf("\r\n");

    if (frame.can_id == OBD_BROADCAST_ID) {
      printf("Received OBD query\r\n");
      if (frame.can_dlc >= 2) {
        switch (frame.data[1]) {
          case OBD2_SERVICE_01:
            printf("Service 01: Show current data\n");
            service_01_response(can_fd, frame.data[2]);
            break;
          case OBD2_SERVICE_09:
            printf("Service 09: Show current data\n");
            service_09_response(can_fd, frame.data[2]);
            break;
          default:
            printf("The service 0x%02X is not support\n", frame.data[1]);
            break;
        }
      } else {
        printf("Error, frame too short. DLC = %d bytes.\n", frame.can_dlc);
      }
    }
  } while (1);
  can_close(can_fd);
  return 0;
}

static int rand_int(int upper, int lower) {
  srand(time(NULL));
  int x = rand() % (upper - lower + 1) + lower;
  return x;
}

static int service_01_response(int can_fd, uint8_t pid) {
  struct can_frame frame;
  memset(&frame, 0, sizeof(frame));
  frame.can_id = OBD_FIRST_ECU_RESPONSE;
  frame.can_dlc = 8;

  printf("Service 1 PID = 0x%02X\r\n", pid);

  // Follow by the SAE standard
  frame.data[0] = 7;          // Number of additional bytes
  frame.data[1] = 0x41;       // Custom Service/Mode (Same as query + 0x40)
  frame.data[2] = pid;        // PID
  int upper = 5, lower = -5;  // Set the noise ±5

  frame.data[3] = 0x0F + rand_int(upper, lower);
  frame.data[4] = 0x0F + rand_int(upper, lower);
  frame.data[5] = 0x0F + rand_int(upper, lower);
  frame.data[6] = 0x0F + rand_int(upper, lower);
  frame.data[7] = 0x00;
  can_send(can_fd, &frame);
  return 0;
}

static void send_vin_message(int can_fd, uint8_t pid) {
  char* vin_array = OBD_EMULATOR_DEFAULT_VIN;
  size_t vin_array_len = strlen(vin_array);
  size_t byte_send = 0;
  uint8_t seq_num = 0;

  // The first can frame
  struct can_frame frame;
  memset(&frame, 0, sizeof(frame));
  frame.can_id = OBD_FIRST_ECU_RESPONSE;
  frame.can_dlc = 8;
  frame.data[0] = 0x10;
  frame.data[1] = 0x14;
  frame.data[2] = 0x49;

  // Generate random data
  frame.data[3] = pid;
  frame.data[4] = 0x01;
  frame.data[5] = vin_array[0];
  frame.data[6] = vin_array[1];
  frame.data[7] = vin_array[2];
  byte_send += 3;
  seq_num += 1;

  can_send(can_fd, &frame);

  // wait for flow control message
  struct can_frame frame_recv;
  memset(&frame_recv, 0, sizeof(struct can_frame));
  can_recv(can_fd, &frame_recv);

  if (frame_recv.data[0] == 0x30) {
    while (byte_send != vin_array_len) {
      memset(&frame, 0, sizeof(frame));
      frame.can_id = OBD_FIRST_ECU_RESPONSE;
      frame.can_dlc = 8;
      frame.data[0] = 0x20 | seq_num;
      memcpy(&frame.data[1], &vin_array[byte_send], 7);
      byte_send += 7;
      seq_num += 1;
      can_send(can_fd, &frame);
    }
  }
}

static int service_09_response(int can_fd, uint8_t pid) {
  switch (pid) {
    case 0x02:
      send_vin_message(can_fd, pid);
      break;
    default:
      printf("The pid 0x%02x is not support", pid);
      break;
  }
  return 0;
}
