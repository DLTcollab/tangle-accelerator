#ifndef REQUEST_TA_GET_TRANSACTION_OBJECT_H_
#define REQUEST_TA_GET_TRANSACTION_OBJECT_H_

#include "accelerator/errors.h"
#include "common/model/transaction.h"
#include "utarray.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file request/ta_get_transaction_object.h
 */

/** struct of ta_get_transaction_object_req_t */
typedef struct ta_get_transaction_object_req {
  /** Transaction hashes in ascii with UT_array. */
  UT_array* hashes;
} ta_get_transaction_object_req_t;

/**
 * Allocate memory of ta_get_transaction_object_req_t
 *
 * @return
 * - struct of ta_get_transaction_object_req_t on success
 * - NULL on error
 */
ta_get_transaction_object_req_t* ta_get_transaction_object_req_new();

/**
 * Push ascii address to ta_get_transaction_object_req_t
 *
 * @param req Data type of ta_get_transaction_object_req_t
 * @param addr queried address
 *
 * @return NULL
 */
status_t ta_get_transaction_object_req_push_address(ta_get_transaction_object_req_t* req, const char* const addr);

/**
 * Search address in ta_get_transaction_object_req_t with index
 *
 * @param req Data type of ta_get_transaction_object_req_t
 * @param index index of index
 *
 * @return NULL
 */
static inline char* ta_get_transaction_object_req_address_at(ta_get_transaction_object_req_t* req,
                                                             uint32_t const index) {
  return *(char**)utarray_eltptr(req->hashes, index);
};

/**
 * Free memory of ta_get_transaction_object_req_t
 *
 * @param res Data type of ta_get_transaction_object_req_t
 *
 * @return NULL
 */
void ta_get_transaction_object_req_free(ta_get_transaction_object_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_GET_TRANSACTION_OBJECT_H_
