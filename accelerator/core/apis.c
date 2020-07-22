/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "apis.h"
#include <sys/time.h>
#include <uuid/uuid.h>
#include "mam_core.h"

#define APIS_LOGGER "apis"

static logger_id_t logger_id;

void apis_logger_init() { logger_id = logger_helper_enable(APIS_LOGGER, LOGGER_DEBUG, true); }

int apis_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

status_t api_get_ta_info(ta_config_t* const info, iota_config_t* const tangle, ta_cache_t* const cache,
                         char** json_result) {
  return ta_get_info_serialize(json_result, info, tangle, cache);
}

status_t api_find_transaction_object_single(const iota_client_service_t* const service, const char* const obj,
                                            char** json_result) {
  status_t ret = SC_OK;
  flex_trit_t txn_hash[NUM_FLEX_TRITS_HASH];
  ta_find_transaction_objects_req_t* req = ta_find_transaction_objects_req_new();
  transaction_array_t* res = transaction_array_new();
  if (req == NULL || res == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  flex_trits_from_trytes(txn_hash, NUM_TRITS_HASH, (const tryte_t*)obj, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_queue_push(&req->hashes, txn_hash);

  ret = ta_find_transaction_objects(service, req, res);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  ret = ta_find_transaction_object_single_res_serialize(res, json_result);

done:
  ta_find_transaction_objects_req_free(&req);
  transaction_array_free(res);
  return ret;
}

status_t api_find_transaction_objects(const iota_client_service_t* const service, const char* const obj,
                                      char** json_result) {
  status_t ret = SC_OK;
  ta_find_transaction_objects_req_t* req = ta_find_transaction_objects_req_new();
  transaction_array_t* res = transaction_array_new();
  if (req == NULL || res == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_find_transaction_objects_req_deserialize(obj, req);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  // Limit the returned transaction objects less equal than 'RESULT_SET_LIMIT'
  while (hash243_queue_count(req->hashes) > RESULT_SET_LIMIT) {
    hash243_queue_pop(&req->hashes);
  }

  ret = ta_find_transaction_objects(service, req, res);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  ret = ta_find_transaction_objects_res_serialize(res, json_result);

done:
  ta_find_transaction_objects_req_free(&req);
  transaction_array_free(res);
  return ret;
}

status_t api_find_transactions_by_tag(const iota_client_service_t* const service, const char* const obj,
                                      char** json_result) {
  status_t ret = SC_OK;
  flex_trit_t tag_trits[NUM_FLEX_TRITS_TAG];
  find_transactions_req_t* req = find_transactions_req_new();
  find_transactions_res_t* res = find_transactions_res_new();
  if (obj == NULL || req == NULL || res == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // If 'tag' is less than 27 trytes (NUM_TRYTES_TAG), expand it
  if (strnlen(obj, NUM_TRYTES_TAG) < NUM_TRYTES_TAG) {
    char new_tag[NUM_TRYTES_TAG + 1];
    // Fill in '9' to get valid tag (27 trytes)
    fill_nines(new_tag, obj, NUM_TRYTES_TAG);
    new_tag[NUM_TRYTES_TAG] = '\0';
    flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)new_tag, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  } else {
    // Valid tag from request, use it directly
    flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)obj, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  }

  if (find_transactions_req_tag_add(req, tag_trits) != RC_OK) {
    ret = SC_CCLIENT_INVALID_FLEX_TRITS;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (iota_client_find_transactions(service, req, res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_find_transactions_by_tag_res_serialize((ta_find_transactions_by_tag_res_t*)res, json_result);

done:
  find_transactions_req_free(&req);
  find_transactions_res_free(&res);
  return ret;
}

status_t api_find_transactions_obj_by_tag(const iota_client_service_t* const service, const char* const obj,
                                          char** json_result) {
  status_t ret = SC_OK;
  flex_trit_t tag_trits[NUM_FLEX_TRITS_TAG];
  find_transactions_req_t* req = find_transactions_req_new();
  transaction_array_t* res = transaction_array_new();
  if (req == NULL || res == NULL || obj == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // If 'tag' is less than 27 trytes (NUM_TRYTES_TAG), expands it
  if (strnlen(obj, NUM_TRYTES_TAG) < NUM_TRYTES_TAG) {
    char new_tag[NUM_TRYTES_TAG + 1];
    // Fill in '9' to get valid tag (27 trytes)
    fill_nines(new_tag, obj, NUM_TRYTES_TAG);
    new_tag[NUM_TRYTES_TAG] = '\0';
    flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)new_tag, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  } else {
    // Valid tag from request, use it directly
    flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)obj, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  }

  if (find_transactions_req_tag_add(req, tag_trits) != RC_OK) {
    ret = SC_CCLIENT_INVALID_FLEX_TRITS;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_find_transactions_obj_by_tag(service, req, res);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  ret = ta_find_transaction_objects_res_serialize(res, json_result);

done:
  find_transactions_req_free(&req);
  transaction_array_free(res);
  return ret;
}

status_t api_recv_mam_message(const iota_config_t* const iconf, const iota_client_service_t* const service,
                              const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  ta_recv_mam_req_t* req = recv_mam_req_new();
  ta_recv_mam_res_t* res = recv_mam_res_new();
  if (!req || !res) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = recv_mam_message_req_deserialize(obj, req);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_recv_mam_message(iconf, service, req, res);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = recv_mam_message_res_serialize(res, json_result);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  recv_mam_req_free(&req);
  recv_mam_res_free(&res);
  return ret;
}

status_t api_send_mam_message(const ta_cache_t* const cache, char const* const obj, char** json_result) {
  status_t ret = SC_OK;
  ta_send_mam_req_t* req = send_mam_req_new();

  ret = send_mam_message_req_deserialize(obj, req);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // Generate UUID. Each UUID would map to a processing request.
  char uuid[UUID_STR_LEN];
  uuid_t bin_uuid;
  uuid_generate_random(bin_uuid);
  uuid_unparse(bin_uuid, uuid);
  if (!uuid[0]) {
    ta_log_error("%s\n", "Failed to generate UUID");
    goto done;
  }

  // TODO Generate the address that TA can quickly generate from the SEED with given parameters.

  // Buffer send_mam_req_t object for publishing it later
  ret = cache_set(uuid, UUID_STR_LEN - 1, obj, strlen(obj), cache->timeout);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // Buffer the UUID to the list
  ret = cache_list_push(cache->mam_buffer_list_name, strlen(cache->mam_buffer_list_name), uuid, UUID_STR_LEN - 1);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = send_mam_message_res_serialize(NULL, uuid, json_result);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  send_mam_req_free(&req);
  return ret;
}

status_t api_send_transfer(const ta_core_t* const core, const iota_client_service_t* iota_service,
                           const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  ta_find_transaction_objects_req_t* txn_obj_req = ta_find_transaction_objects_req_new();
  transaction_array_t* res_txn_array = transaction_array_new();

  if (req == NULL || res == NULL || txn_obj_req == NULL || res_txn_array == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_send_transfer_req_deserialize(obj, req);
  if (ret) {
    ta_log_error("%s\n", "ta_send_transfer_req_deserialize failed");

    goto done;
  }

  ret = ta_send_transfer(&core->ta_conf, &core->iota_conf, iota_service, &core->cache, req, res);
  if (ret == SC_CCLIENT_FAILED_RESPONSE) {
    ta_log_info("%s\n", "Caching transaction");

    // Cache the request and serialize UUID as response directly
    goto serialize;
  } else if (ret) {
    ta_log_error("%s\n", "ta_send_transfer failed");
    goto done;
  }

  // return transaction object
  if (hash243_queue_count(res->hash) == 0 || hash243_queue_push(&txn_obj_req->hashes, hash243_queue_peek(res->hash))) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "hash243_queue_push failed");
    goto done;
  }

  ret = ta_find_transaction_objects(iota_service, txn_obj_req, res_txn_array);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  res->txn_array = res_txn_array;
#ifdef DB_ENABLE
  ret = db_insert_tx_into_identity(&core->db_service, res->hash, PENDING_TXN, res->uuid_string);
  if (ret != SC_OK) {
    ta_log_error("fail to insert new pending transaction for reattachment\n");
    goto done;
  }
#endif

serialize:
  ret = ta_send_transfer_res_serialize(res, json_result);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  ta_send_transfer_req_free(&req);
  ta_send_transfer_res_free(&res);
  ta_find_transaction_objects_req_free(&txn_obj_req);
  transaction_array_free(res_txn_array);
  return ret;
}

status_t api_send_trytes(const ta_config_t* const info, const iota_config_t* const iconf,
                         const iota_client_service_t* const service, const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  hash8019_array_p trytes = hash8019_array_new();

  if (!trytes) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_send_trytes_req_deserialize(obj, trytes);
  if (ret != SC_OK) {
    goto done;
  }

  ret = ta_send_trytes(info, iconf, service, trytes);
  if (ret != SC_OK) {
    goto done;
  }

  ret = ta_send_trytes_res_serialize(trytes, json_result);

done:
  hash_array_free(trytes);
  return ret;
}

status_t api_get_node_status(const iota_client_service_t* const service, char** json_result) {
  status_t ret = SC_OK;

  ret = ta_get_node_status(service);
  switch (ret) {
    /*
     * The values of each status_t are listed as the following. Not listed status code are unexpected errors which
     * would cause TA return error.
     *
     * SC_CCLIENT_FAILED_RESPONSE: Can't connect to IOTA full node
     * SC_CORE_NODE_UNSYNC: IOTA full node is not at the latest milestone
     * SC_OK: IOTA full node works fine.
     **/
    case SC_CCLIENT_FAILED_RESPONSE:
    case SC_CORE_NODE_UNSYNC:
    case SC_OK:
      ret = get_node_status_res_serialize(ret, json_result);
      if (ret) {
        ta_log_error("failed to serialize. status code: %d\n", ret);
      }
      break;

    default:
      ta_log_error("check IOTA full node connection failed. status code: %d\n", ret);
      break;
  }

  return ret;
}

status_t api_fetch_buffered_request_status(const ta_cache_t* const cache, const char* const uuid, char** json_result) {
  status_t ret = SC_OK;

  ta_fetch_buffered_request_status_res_t* res = ta_fetch_buffered_request_status_res_new();
  if (res == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    return ret;
  }

  ret = ta_fetch_txn_with_uuid(cache, uuid, res);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // Check if the UUID exists in MAM buffered list when tangle-accelerator can't find it in transaction buffered list.
  if (res->status == NOT_EXIST) {
    ret = ta_fetch_mam_with_uuid(cache, uuid, res);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  }

  ret = fetch_buffered_request_status_res_serialize(res, json_result);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  ta_fetch_buffered_request_status_res_free(&res);
  return ret;
}

status_t api_register_mam_channel(const ta_cache_t* const cache, const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  ta_register_mam_channel_req_t* req = ta_register_mam_channel_req_new();
  char uuid[UUID_STR_LEN];

  ret = register_mam_channel_req_deserialize(obj, req);
  if (ret) {
    ta_log_error("Failed to deserialize request.\n");
    goto done;
  }

  ret = ta_register_mam_channel(cache, req, uuid);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = register_mam_channel_res_serialize(uuid, json_result);
  if (ret) {
    ta_log_error("Failed to deserialize request.\n");
  }

done:
  ta_register_mam_channel_req_free(&req);
  return ret;
}

#ifdef DB_ENABLE
status_t api_find_transactions_by_id(const iota_client_service_t* const iota_service,
                                     const db_client_service_t* const db_service, const char* const obj,
                                     char** json_result) {
  if (obj == NULL) {
    ta_log_error("Invalid NULL pointer to uuid string\n");
    return SC_NULL;
  }
  status_t ret = SC_OK;
  ta_log_info("find transaction by uuid string: %s\n", obj);
  db_identity_array_t* db_identity_array = db_identity_array_new();
  ret = db_get_identity_objs_by_uuid_string(db_service, obj, db_identity_array);
  if (ret != SC_OK) {
    ta_log_error("fail to find transaction by uuid string\n");
    goto exit;
  }

  db_identity_t* itr = (db_identity_t*)utarray_front(db_identity_array);
  if (itr != NULL) {
    ret = api_find_transaction_object_single(iota_service, (const char* const)db_ret_identity_hash(itr), json_result);
  } else {
    ta_log_error("No corresponding transaction found by uuid string : %s\n", obj);
    ret = SC_TA_WRONG_REQUEST_OBJ;
  }

exit:
  db_identity_array_free(&db_identity_array);
  return ret;
}

status_t api_get_identity_info_by_hash(const db_client_service_t* const db_service, const char* const obj,
                                       char** json_result) {
  if (obj == NULL) {
    ta_log_error("Invalid NULL pointer to uuid string\n");
    return SC_NULL;
  }
  status_t ret = SC_OK;
  ta_log_info("get identity info by hash : %s\n", obj);
  db_identity_array_t* db_identity_array = db_identity_array_new();
  ret = db_get_identity_objs_by_hash(db_service, (const cass_byte_t*)obj, db_identity_array);
  if (ret != SC_OK) {
    ta_log_error("fail to get identity objs by transaction hash\n");
    goto exit;
  }

  db_identity_t* itr = (db_identity_t*)utarray_front(db_identity_array);
  if (itr != NULL) {
    ret = db_identity_serialize(json_result, itr);
  } else {
    ta_log_error("No corresponding identity info found by hash : %s\n", obj);
    ret = SC_TA_WRONG_REQUEST_OBJ;
  }
exit:
  db_identity_array_free(&db_identity_array);
  return ret;
}

status_t api_get_identity_info_by_id(const db_client_service_t* const db_service, const char* const obj,
                                     char** json_result) {
  if (obj == NULL) {
    ta_log_error("Invalid NULL pointer to uuid string\n");
    return SC_NULL;
  }

  status_t ret = SC_OK;
  ta_log_info("get identity info by uuid string : %s\n", obj);
  db_identity_array_t* db_identity_array = db_identity_array_new();
  ret = db_get_identity_objs_by_uuid_string(db_service, obj, db_identity_array);
  if (ret != SC_OK) {
    ta_log_error("fail to get identity objs by uuid string\n");
    goto exit;
  }

  db_identity_t* itr = (db_identity_t*)utarray_front(db_identity_array);
  if (itr != NULL) {
    ret = db_identity_serialize(json_result, itr);
  } else {
    ta_log_error("No corresponding identity info found by uuid string : %s\n", obj);
    ret = SC_TA_WRONG_REQUEST_OBJ;
  }

exit:
  db_identity_array_free(&db_identity_array);
  return ret;
}
#endif
