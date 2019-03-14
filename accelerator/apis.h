#ifndef ACCELERATOR_APIS_H_
#define ACCELERATOR_APIS_H_

#include "accelerator/common_core.h"
#include "accelerator/errors.h"
#include "cclient/types/types.h"
#include "common/trinary/trit_tryte.h"
#include "common/trinary/tryte_ascii.h"
#include "mam/api/api.h"
#include "serializer/serializer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file apis.h
 * @brief General tangle-accelerator APIs
 *
 * tangle-accelerator APIs provide major IOTA APIs wrapper for public usage.
 * The arguments and return strings are all in json format. There can be
 * different server or protocol integration with these APIs.
 */

/**
 * @brief Generate an unused address.
 *
 * Generate and return an unused address from the seed. An unused address means
 * the address does not have any transaction with it yet.
 *
 * @param[in] service IRI node end point service
 * @param[out] json_result Result containing an unused address in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_generate_address(const iota_client_service_t* const service,
                              char** json_result);

/**
 * @brief Get trunk and branch transactions
 *
 * Get a tips pair as trunk/branch transactions for transaction construction.
 * The result is char array in json format:
 *
 * @param[in] service IRI node end point service
 * @param[out] json_result Result containing a tips pair in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_tips_pair(const iota_client_service_t* const service,
                           char** json_result);

/**
 * @brief Get list of all tips from IRI node.
 *
 * Get list of all tips from IRI node which usually hash thousands of tips in
 * its queue.
 *
 * @param[in] service IRI node end point service
 * @param[out] json_result Result containing list of all tips in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_tips(const iota_client_service_t* const service,
                      char** json_result);

status_t api_receive_mam_message(const iota_client_service_t* const service,
                                 const char* const obj, char** json_result);

/**
 * @brief Send transfer to tangle.
 *
 * Build the transfer bundle from request and broadcast to the tangle. Input
 * fields include address, value, tag, and message. This API would also try to
 * find the transactions after bundle sent.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj Input data in JSON
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_send_transfer(const iota_client_service_t* const service,
                           const char* const obj, char** json_result);

/**
 * @brief Return transaction object with given transaction hash.
 *
 * Explore transaction hash information with given transaction haash. This would
 * return whole transaction object details in json format instead of raw trytes.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj transaction hash in trytes
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_transaction_object(const iota_client_service_t* const service,
                                    const char* const obj, char** json_result);

/**
 * @brief Return list of transaction hash with given tag hash.
 *
 * Retreive all transactions that have same given tag. The result is a list of
 * transaction hash in json format.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj tag in trytes
 * @param[out] json_result Result containing list of transaction hash in json
 *             format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transactions_by_tag(
    const iota_client_service_t* const service, const char* const obj,
    char** json_result);

/**
 * @brief Return list of transaction object with given tag hash.
 *
 * Retreive all transactions that have same given tag. The result is a list of
 * transaction objects in json format.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj tag in trytes
 * @param[out] json_result Result containing list of transaction object in json
 * format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transactions_obj_by_tag(
    const iota_client_service_t* const service, const char* const obj,
    char** json_result);

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_APIS_H_
