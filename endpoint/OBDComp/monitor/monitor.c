/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "interfaces.h"
#include "legato.h"

#include "brotli/decode.h"
#include "brotli/encode.h"
#include "build/obd_generated.h"
#include "cipher.h"
#include "endpoint/OBDComp/can-bus/can-utils.h"
#include "endpoint/OBDComp/obd_pid.h"
#include "hal/device.h"

#include "legato.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int string_compress(uint8_t *origin, size_t len, uint8_t **out_buffer, size_t *out_size) {
  int lgwin = BROTLI_DEFAULT_WINDOW;
  int bufferSize = len;
  uint8_t *buffer = malloc(bufferSize);
  uint8_t *inBuffer = malloc(bufferSize);

  memcpy(inBuffer, origin, len);

  size_t encodedSize = bufferSize;

  bool brotliStatus = BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, lgwin, BROTLI_MODE_GENERIC, len,
                                            (const uint8_t *)inBuffer, &encodedSize, buffer);

  *out_buffer = buffer;
  *out_size = encodedSize;

  free(inBuffer);
  return brotliStatus;
}

#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(OBD, x)  // Specified in the schema.

static uint32_t hex_to_obd_data(uint8_t *arr, int offset, size_t len) {
  uint32_t result = 0;

  for (size_t i = offset; i < len; i++) {
    result |= arr[i];
    result = result << 4;
  }
  return result;
}

static char *vin_hex_to_string(uint8_t *arr, size_t len) {
  char *str = malloc(len);
  memset(str, 0, len);
  memcpy(str, &arr[3], len - 3);
  return str;
}

int create_obd2_flatbuffer(uint8_t **flat_msg, size_t *flat_buf_len, le_hashmap_Ref_t OBD_datasets) {
  flatcc_builder_t builder;

  // Initialize the builder object.
  flatcc_builder_init(&builder);

  OBD_data_t *vin_obd_data = le_hashmap_Get(OBD_datasets, "VEHICLE_IDENTIFICATION_NUMBER");
  char *vin_id = vin_hex_to_string(vin_obd_data->data, vin_obd_data->data_len);

  flatbuffers_string_ref_t vin = flatbuffers_string_create_str(&builder, vin_id);

  uint32_t engine_load = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "CALCULATED_ENGINE_LOAD"), 4, 8);
  uint32_t engine_coolant_temperature =
      hex_to_obd_data(le_hashmap_Get(OBD_datasets, "ENGINE_COOLANT_TEMPERATURE"), 4, 8);
  uint32_t fuel_pressure = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "FUEL_PRESSURE"), 4, 8);
  uint32_t engine_speed = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "ENGINE_RPM"), 4, 8);
  uint32_t vehicle_speed = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "VEHICLE_SPEED"), 4, 8);
  uint32_t intake_air_temperature = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "INTAKE_AIR_TEMPERATURE"), 4, 8);
  uint32_t mass_air_flow = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "MASS_AIR_FLOW"), 4, 8);
  uint32_t fuel_tank_level_input = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "FUEL_TANK_LEVEL_INPUT"), 4, 8);
  uint32_t absolute_barometric_pressure =
      hex_to_obd_data(le_hashmap_Get(OBD_datasets, "ABSOLUTE_BAROMETRIC_PRESSURE"), 4, 8);
  uint32_t control_module_voltage = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "CONTROL_MODULE_VOLTAGE"), 4, 8);
  uint32_t throttle_position = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "THROTTLE_POSITION"), 4, 8);
  uint32_t ambient_air_temperature = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "AMBIENT_AIR_TEMPERATURE"), 4, 8);
  uint32_t relative_accelerator_pedal_position =
      hex_to_obd_data(le_hashmap_Get(OBD_datasets, "RELATIVE_ACCELERATOR_PEDAL_POSITION"), 4, 8);
  uint32_t engine_oil_temperature = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "ENGINE_OIL_TEMPERATURE"), 4, 8);
  uint32_t engine_fuel_rate = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "ENGINE_FUEL_RATE"), 4, 8);
  uint32_t service_distance = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "SERVICE_DISTANCE"), 4, 8);
  uint32_t anti_lock_barking_active = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "ANTI_LOCK_BRAKING_ACTIVE"), 4, 8);
  uint32_t steering_wheel_angle = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "STEERING_WHEEL_ANGLE"), 4, 8);
  uint32_t position_of_doors = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "POSITION_OF_DOORS"), 4, 8);
  uint32_t right_left_turn_signal_light =
      hex_to_obd_data(le_hashmap_Get(OBD_datasets, "RIGHT_LEFT_TURN_SIGNAL_LIGHT"), 4, 8);
  uint32_t alternate_beam_head_light = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "ALTERNATE_BEAM_HEAD_LIGHT"), 4, 8);
  uint32_t high_beam_head_light = hex_to_obd_data(le_hashmap_Get(OBD_datasets, "HIGH_BEAM_HEAD_LIGHT"), 4, 8);

  ns(OBD2_data_ref_t) obd2_flatbuffer = ns(OBD2_data_create(
      &builder, vin, engine_load, engine_coolant_temperature, fuel_pressure, engine_speed, vehicle_speed,
      intake_air_temperature, mass_air_flow, fuel_tank_level_input, absolute_barometric_pressure,
      control_module_voltage, throttle_position, ambient_air_temperature, relative_accelerator_pedal_position,
      engine_coolant_temperature, engine_fuel_rate, service_distance, anti_lock_barking_active, steering_wheel_angle,
      position_of_doors, right_left_turn_signal_light, alternate_beam_head_light, high_beam_head_light));

  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);

  device_t *device = ta_device(STRINGIZE(EP_TARGET));
  char device_id[16] = {0};
  device->op->get_device_id(device_id);

  flatbuffers_string_ref_t deviceID = flatbuffers_string_create_str(&builder, device_id);
  ns(OBD2Meta_ref_t) obd2_meta = ns(OBD2Meta_create(&builder, deviceID, t.tv_sec, obd2_flatbuffer));

  uint8_t *buf;
  size_t buf_size;

  buf = flatcc_builder_get_direct_buffer(&builder, &buf_size);

  size_t encodeSize = 0;
  uint8_t *encode_result;
  string_compress(buf, buf_size, &encode_result, &encodeSize);

  *flat_msg = encode_result;
  *flat_buf_len = encodeSize;

  free(vin_id);
  flatcc_builder_clear(&builder);
  return 0;
}

enum MESSAGE_TYPE { SINGLE, FIRST, CONSECUTIVE, FLOW };

static int check_message_type(struct can_frame *frame) {
  uint8_t header_byte = frame->data[0];
  return header_byte >> 4;
}

static le_result_t obd_query(int fd, canid_t can_id, uint8_t service, uint8_t pid, struct can_frame *frame_recv) {
  if (frame_recv == NULL) {
    LE_ERROR("Input frame should not be none");
    return LE_FAULT;
  }

  struct can_frame frame;
  memset(&frame, 0, sizeof(struct can_frame));
  memset(frame_recv, 0, sizeof(struct can_frame));
  frame.can_id = can_id;
  frame.can_dlc = 8;

  frame.data[0] = 2;        // Number of additional bytes
  frame.data[1] = service;  // Service/Mode
  frame.data[2] = pid;      // PID

  can_send(fd, &frame);
  can_recv(fd, frame_recv);
  return LE_OK;
}

static le_result_t obd_message(int fd, struct can_frame *frame_recv, OBD_data_t *obd_data) {
  static int append_data = 0;

  size_t _size = 0;
  uint8_t *buffer = NULL;

  bool need_flow_control = false;
  switch (check_message_type(frame_recv)) {
    case SINGLE:
      _size = 8;
      buffer = malloc(_size);
      memset(buffer, 0, _size);
      memcpy(buffer, frame_recv->data, _size);
      obd_data->data = buffer;
      obd_data->data_len = _size;
      return LE_OK;
    case FIRST:
      // allocate data buffer for consecutive data
      _size = ((uint16_t)(frame_recv->data[0] & 0x0F) << 8) | frame_recv->data[1];
      buffer = malloc(_size);
      memset(buffer, 0, _size);
      // copy other 6 bytes
      memcpy(buffer, &frame_recv->data[2], 6);
      obd_data->data = buffer;
      obd_data->data_len = _size;
      append_data += 6;
      need_flow_control = true;
      break;
    case CONSECUTIVE:
      // copy other 7 bytes
      memcpy(&obd_data->data[append_data], &frame_recv->data[1], 7);
      append_data += 7;
      break;
  }

  if (need_flow_control) {
    struct can_frame flow_control_frame;
    memset(&flow_control_frame, 0, sizeof(struct can_frame));
    flow_control_frame.can_id = frame_recv->can_id - OBD_REQUEST_RESPONSE_OFFSET;
    flow_control_frame.can_dlc = 8;
    flow_control_frame.data[0] = FLOW << 4;

    can_send(fd, &flow_control_frame);
  }

  // check consecutive message is complete
  if (obd_data->data_len == append_data) {
    return LE_OK;
  }
  can_recv(fd, frame_recv);
  return obd_message(fd, frame_recv, obd_data);
}

static void free_obd_hashmap_datasets(le_hashmap_Ref_t OBD_datasets) {
  const char *next_key;
  OBD_data_t *obd_data_ptr;

  le_hashmap_It_Ref_t myIter = le_hashmap_GetIterator(OBD_datasets);

  while (LE_OK == le_hashmap_NextNode(myIter)) {
    next_key = le_hashmap_GetKey(myIter);
    obd_data_ptr = le_hashmap_Remove(OBD_datasets, next_key);
    free(obd_data_ptr->data);
    free(obd_data_ptr);
  }
  le_hashmap_RemoveAll(OBD_datasets);
}

static void get_obd_datasets(int fd, const char *host, const char *port, const char *ssl_seed, const char *mam_seed,
                             const char *private_key, const char *device_id) {
  PID *tmp_pid = PIDS;
  struct can_frame frame_recv;

  le_hashmap_Ref_t obd2_data_table =
      le_hashmap_Create("obd2 data table", LENGTH_OF_PIDS, le_hashmap_HashString, le_hashmap_EqualsString);

  for (size_t i = 0; i < LENGTH_OF_PIDS; ++i) {
    OBD_data_t *obd_data = malloc(sizeof(OBD_data_t));
    uint32_t pid = tmp_pid[i].pid;
    memset(&frame_recv, 0, sizeof(frame_recv));
    memset(obd_data, 0, sizeof(OBD_data_t));
    obd_query(fd, OBD_BROADCAST_ID, tmp_pid[i].service, pid, &frame_recv);
    obd_message(fd, &frame_recv, obd_data);
    le_hashmap_Put(obd2_data_table, pid_to_string(i), obd_data);
  }

  uint8_t *flatbuffer_msg = NULL;
  size_t flatbuffer_msg_len = 0;
  flatcc_builder_t builder;
  create_obd2_flatbuffer(&flatbuffer_msg, &flatbuffer_msg_len, obd2_data_table);
  free_obd_hashmap_datasets(obd2_data_table);
  LE_INFO("Success to create flatbuffer for obd2 datasets");
  LE_INFO("Flatbuffer length: %zu", flatbuffer_msg_len);

  // Send flatbuffer message via endpointService
  int ret = endpoint_Send_mam_message(host, port, ssl_seed, mam_seed, flatbuffer_msg, flatbuffer_msg_len,
                                      (uint8_t *)private_key, AES_CBC_KEY_SIZE, device_id);
  LE_INFO("endpoint service return: %d", ret);
  free(flatbuffer_msg);
}

static void print_help(void) {
  puts(
      "NAME\n"
      "OBDService - The obd2 service for sending OBDII message to Tangle-accelerator.\n"
      "\n"
      "SYNOPSIS\n"
      "  OBDService [-h]\n"
      "  OBDService [--help]\n"
      "             [--host=\"host\"]\n"
      "             [--port=\"port\"]\n"
      "             [--ssl-seed=\"ssl-seed\"]\n"
      "             [--mam-seed=\"seed\"]\n"
      "             [--private-key=\"private-key\"]\n"
      "\n"
      "OPTIONS\n"
      "  -h\n"
      "  --help\n"
      "    Print the information for helping the users. Ignore other arguments.\n"
      "\n"
      "  -i\n"
      "  --can-interface\n"
      "    Set the can interface.\n"
      "\n"
      "  --mam-seed=\"seed\"\n"
      "    Channel root seed. It is an 81-trytes string.\n"
      "\n"
      "  --msg=\"message\"\n"
      "    Assign the message value with string for send_transaction_information.\n"
      "\n"
      "  --host=\"host\"\n"
      "    Assign the host of the tangle-accelerator for send_transaction_information.\n"
      "    The default value is NULL.\n"
      "\n"
      "  --port=\"port\"\n"
      "    Assign the port of the tangle-accelerator for send_transaction_information.\n"
      "    The default value is NULL.\n"
      "\n"
      "  --ssl-seed=\"ssl-seed\"\n"
      "    Assign the random seed of the SSL configuration for send_transaction_information.\n"
      "    The default value is NULL.\n"
      "\n"
      "  --private-key=\"private-key\"\n"
      "    The private key for encrypting message.\n"
      "\n");

  exit(EXIT_SUCCESS);
}

COMPONENT_INIT {
  const char *interface = NULL;
  const char *host = NULL;
  const char *port = NULL;
  const char *ssl_seed = "";
  const char *mam_seed = NULL;
  const char *private_key = NULL;

  char device_id[16] = {0};
  const char *device_id_ptr = device_id;

  le_arg_SetStringVar(&interface, "i", "can-interface");
  le_arg_SetFlagCallback(print_help, "h", "help");
  le_arg_SetStringVar(&host, NULL, "host");
  le_arg_SetStringVar(&port, NULL, "port");
  le_arg_SetStringVar(&ssl_seed, NULL, "ssl-seed");
  le_arg_SetStringVar(&mam_seed, NULL, "mam-seed");
  le_arg_SetStringVar(&private_key, NULL, "private-key");
  le_arg_Scan();

  device_t *device = ta_device(STRINGIZE(EP_TARGET));
  if (device == NULL) {
    LE_ERROR("Can not get specific device");
    exit(EXIT_FAILURE);
  }
  device->op->get_device_id(device_id);

  if (interface == NULL) {
    print_help();
  }

  if (host == NULL || port == NULL || mam_seed == NULL || private_key == NULL) {
    LE_ERROR("The host, port, mam seed and the private key should not be NULL");
    exit(EXIT_FAILURE);
  }

  int fd;
  can_open((char *)interface, &fd);
  get_obd_datasets(fd, host, port, ssl_seed, mam_seed, private_key, device_id);

  can_close(fd);
  exit(EXIT_SUCCESS);
}
