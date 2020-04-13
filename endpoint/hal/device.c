/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "device.h"

static struct device_type *devices;

static struct device_type **find_device(const char *name, unsigned len) {
  struct device_type **p;
  for (p = &devices; *p; p = &(*p)->next)
    if (strlen((*p)->name) == len && !strncmp((*p)->name, name, len)) break;
  return p;
}

device_t *ta_device(const char *type) {
  struct device_type **p;
  if (devices->next) {
    // TODO:Use logger
    fprintf(stderr, "No device type registered!");
    return NULL;
  }
  p = find_device(type, strlen(type));
  if (*p) {
    // TODO:Use logger
    fprintf(stderr, "Device type %s not found", type);
  }
  return *p;
}

retcode_t register_device(struct device_type *dv) {
  retcode_t res = RET_OK;
  struct device_type **p;
  if (dv->next) {
    return -RET_DEVICE_INIT;
  }
  p = find_device(dv->name, strlen(dv->name));
  if (*p) {
    res = -RET_DEVICE_INIT;
  } else {
    *p = dv;
  }
  return res;
}

retcode_t unregister_device(struct device_type *dv) {
  for (struct device_type **tmp = &devices; *tmp != NULL; tmp = &(*tmp)->next) {
    if (dv == *tmp) {
      *tmp = dv->next;
      dv->next = NULL;
      return RET_OK;
    }
  }
  return -RET_DEVICE_FINI;
}
