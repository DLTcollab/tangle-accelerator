/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef OBD_PID_H
#define OBD_PID_H

#include <stdint.h>

/**
 * @file endpoint/OBDComp/obd_pid.h
 */

#define OBD_BROADCAST_ID 0x7DF
#define OBD_FIRST_ECU_RESPONSE 0x7E8
#define OBD_LAST_ECU_RESPONSE 0x7EF

// An ECU responds to a message with an ID 8 less than the request
// For example, the engine control unit responds with 0x7E0 (8 less than 0x7E8)
#define OBD_REQUEST_RESPONSE_OFFSET 8

/**
 * @brief Struct of OBDII PID
 *
 */
typedef struct pid {
  uint8_t service;  //< Service
  uint16_t pid;     //< Query PID
} PID;

typedef struct OBD_data {
  uint8_t* data;
  size_t data_len;
} OBD_data_t;

/**
 * @brief List of OBDII PID
 *
 */
#define PIDS_INIT_LIST                                                                      \
  X(VEHICLE_IDENTIFICATION_NUMBER, 0x09, 0x02, "VEHICLE_IDENTIFICATION_NUMBER")             \
  X(CALCULATED_ENGINE_LOAD, 0x01, 0x04, "CALCULATED_ENGINE_LOAD")                           \
  X(ENGINE_COOLANT_TEMPERATURE, 0x01, 0x05, "ENGINE_COOLANT_TEMPERATURE")                   \
  X(FUEL_PRESSURE, 0x01, 0x0A, "FUEL_PRESSURE")                                             \
  X(ENGINE_RPM, 0x01, 0x0C, "ENGINE_RPM")                                                   \
  X(VEHICLE_SPEED, 0x01, 0x0D, "VEHICLE_SPEED")                                             \
  X(INTAKE_AIR_TEMPERATURE, 0x01, 0x0F, "INTAKE_AIR_TEMPERATURE")                           \
  X(MASS_AIR_FLOW, 0x01, 0x10, "MASS_AIR_FLOW")                                             \
  X(FUEL_TANK_LEVEL_INPUT, 0x01, 0x01F, "FUEL_TANK_LEVEL_INPUT")                            \
  X(ABSOLUTE_BAROMETRIC_PRESSURE, 0x01, 0x33, "ABSOLUTE_BAROMETRIC_PRESSURE")               \
  X(CONTROL_MODULE_VOLTAGE, 0x01, 0x42, "CONTROL_MODULE_VOLTAGE")                           \
  X(THROTTLE_POSITION, 0x01, 0x45, "THROTTLE_POSITION")                                     \
  X(AMBIENT_AIR_TEMPERATURE, 0x01, 0x46, "AMBIENT_AIR_TEMPERATURE")                         \
  X(RELATIVE_ACCELERATOR_PEDAL_POSITION, 0x01, 0x5A, "RELATIVE_ACCELERATOR_PEDAL_POSITION") \
  X(ENGINE_OIL_TEMPERATURE, 0x01, 0x5C, "ENGINE_OIL_TEMPERATURE")                           \
  X(ENGINE_FUEL_RATE, 0x01, 0x5E, "ENGINE_FUEL_RATE")                                       \
  X(SERVICE_DISTANCE, 0x01, 0xE1, "SERVICE_DISTANCE")                                       \
  X(ANTI_LOCK_BRAKING_ACTIVE, 0x01, 0xE2, "ANTI_LOCK_BRAKING_ACTIVE")                       \
  X(STEERING_WHEEL_ANGLE, 0x01, 0xE3, "STEERING_WHEEL_ANGLE")                               \
  X(POSITION_OF_DOORS, 0x01, 0xE5, "POSITION_OF_DOORS")                                     \
  X(RIGHT_LEFT_TURN_SIGNAL_LIGHT, 0x01, 0xE6, "RIGHT_LEFT_TURN_SIGNAL_LIGHT")               \
  X(ALTERNATE_BEAM_HEAD_LIGHT, 0x01, 0xE7, "ALTERNATE_BEAM_HEAD_LIGHT")                     \
  X(HIGH_BEAM_HEAD_LIGHT, 0x01, 0xE8, "HIGH_BEAM_HEAD_LIGHT")

#define X(a, b, c, d) a,
enum ENUM_PID { PIDS_INIT_LIST };
#undef X

static PID PIDS[] = {
#define X(a, b, c, d) {.service = b, .pid = c},
    PIDS_INIT_LIST
#undef X
};

#define LENGTH_OF_PIDS (sizeof(PIDS) / sizeof(PID))

static const char* pid_to_string(int value) {
#define X(a, b, c, d) d,
  static char* table[] = {PIDS_INIT_LIST};
#undef X
  return table[value];
}

#endif  // OBD_PID_H
