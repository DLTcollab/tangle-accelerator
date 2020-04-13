/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ERROR_H
#define ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#define HTTP_OK 200 /**< HTTP status codes */

/* status code */
typedef enum {
  RET_OK,           /**< success */
  RET_WRITE_ERROR,  /**< write error */
  RET_OOM,          /**< out of memory */
  RET_HTTP_INIT,    /**< failed on HTTP init */
  RET_HTTP_CERT,    /**< failed on x509 cert parse */
  RET_HTTP_CONNECT, /**< failed on HTTP initial connection */
  RET_HTTP_SSL,     /**< failed on setting ssl config */
  RET_DEVICE_INIT,  /**< initialize device error */
  RET_DEVICE_FINI,  /**< finalize device error */
  RET_UART_INIT,    /**< initialize uart error */
  RET_NO_MEMORY,    /**< not enough memory error */
  RET_OVERFLOW,     /**< overflow error */
  RET_NOT_FOUND,    /**< item not found error */
  RET_UNAVAILABLE,  /**< item not available */
  RET_FAULT         /**< some other fault */
} retcode_t;

#ifdef __cplusplus
}
#endif

#endif  // ERROR_H
