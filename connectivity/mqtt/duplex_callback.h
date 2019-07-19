/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef DUPLEX_CALLBACK_H
#define DUPLEX_CALLBACK_H

#include "duplex_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/duplex_callback.h
 */

/**
 * @brief Interface for functions setting callback functions.
 *
 * @param[in] mosq `struct mosquitto` object
 */
void duplex_callback_func_set(struct mosquitto *mosq);

#ifdef __cplusplus
}
#endif

#endif  // DUPLEX_CALLBACK_H
