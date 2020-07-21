/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "diagnose.h"

#include "le_gnss_interface.h"
#include "le_mrc_interface.h"

static void hardware_logging(le_timer_Ref_t timer) {
  static char time_str[30];
  static size_t copied_bytes = 0;
  static int32_t latitude = 0, longitude = 0, accuracy = 0;
  static uint32_t quality = 0;
  static le_gnss_SampleRef_t position_sample;
  static le_result_t result;

  // Signal strength
  result = le_mrc_GetSignalQual(&quality);
  switch (result) {
    case LE_OK:
      LE_INFO("Signal strength: %d\n", quality);
      break;
    default:
      LE_ERROR("Fail to get signal strength\n");
      break;
  }

  // GPS
  position_sample = le_gnss_GetLastSampleRef();
  result = le_gnss_GetLocation(position_sample, &latitude, &longitude, &accuracy);
  switch (result) {
    case LE_OK:
      LE_INFO("Latitude: %lf, Longitude: %lf, Horizontal accuracy: %d\n", (double)latitude / 1000000,
              (double)longitude / 1000000, accuracy);
      break;
    case LE_FAULT:
      LE_ERROR("Fail to get location data\n");
      break;
    case LE_OUT_OF_RANGE:
      LE_ERROR("At least one retrieved value is invalid (set to INT32_MAX)\n");
      LE_INFO("Latitude: %lf, Longitude: %lf, Horizontal accuracy: %d\n", (double)latitude / 1000000,
              (double)longitude / 1000000, accuracy);
      break;
    default:
      break;
  }

  // Time
  result = le_clk_GetLocalDateTimeString(LE_CLK_STRING_FORMAT_DATE_TIME, time_str, sizeof(time_str), &copied_bytes);
  switch (result) {
    case LE_OK:
      LE_INFO("Time: %s\n", time_str);
      break;
    case LE_OVERFLOW:
      LE_ERROR("Overflow of time string\n");
      break;
    default:
      break;
  }
}

COMPONENT_INIT {
  le_timer_Ref_t timer;
  le_clk_Time_t interval = {3 /*sec*/, 0 /*usec*/};
  le_gnss_State_t gnss_state;

  // GPS state checking and activation
  gnss_state = le_gnss_GetState();
  if (gnss_state != LE_GNSS_STATE_ACTIVE) {
    le_gnss_Start();
    le_gnss_Enable();
  }

  // Timer for hardware logging
  timer = le_timer_Create("diagnose_timer");
  le_timer_SetInterval(timer, interval);
  le_timer_SetRepeat(timer, 0);
  le_timer_SetHandler(timer, hardware_logging);
  le_timer_Start(timer);
}
