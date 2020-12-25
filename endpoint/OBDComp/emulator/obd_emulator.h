/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef OBD_EMULATOR_H
#define OBD_EMULATOR_H

#include <stdint.h>

/**
 * @file endpoint/OBDComp/emulator/obd_emulator.h
 */

// default can interface for obd emulator
#define OBD_EMULATOR_INTERFACE "vcan0"
#define OBD_EMULATOR_DEFAULT_VIN "1234567890ABCDEFG"
#define OBD_EMULATOR_DEFAULT_VIN_LEN 17

#define OBD_BROADCAST_ID 0x7DF
#define OBD_FIRST_ECU_RESPONSE 0x7E8
#define OBD_LAST_ECU_RESPONSE 0x7EF

// An ECU responds to a message with an ID 8 less than the request
// For example, the engine control unit responds with 0x7E0 (8 less than 0x7E8)
#define OBD_REQUEST_RESPONSE_OFFSET 8

enum OBD2_SERVICE { OBD2_SERVICE_01 = 0x01, OBD2_SERVICE_09 = 0x09 };

#endif  // OBD_EMULATOR_H
