/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef DIAGNOSE_H
#define DIAGNOSE_H

#include "legato.h"

/**
 * @file endpoint/diagnoseComp/diagnose.h
 */

/**
 * @brief Log the information from hardware
 *
 * @param[in] timer The timer of the function. It would be passed by the timer API le_timer_SetHandler().
 */
static void hardware_logging(le_timer_Ref_t timer);

#endif  // DIAGNOSE_H
