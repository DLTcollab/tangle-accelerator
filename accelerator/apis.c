/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "apis.h"
#include "map/mode.h"
#include "utils/handles/lock.h"

#define APIS_LOGGER "apis"

static logger_id_t logger_id;
lock_handle_t cjson_lock;

void apis_logger_init() { logger_id = logger_helper_enable(APIS_LOGGER, LOGGER_DEBUG, true); }

int apis_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_critical("Destroying logger failed %s.\n", APIS_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t api_get_ta_info(ta_config_t* const info, iota_config_t* const tangle, ta_cache_t* const cache,
                         iota_client_service_t* const service, char** json_result) {
  status_t ret = SC_OK;

  ret = ta_get_info_serialize(json_result, info, tangle, cache, service);
  if (ret != SC_OK) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
  }

  return ret;
}

status_t apis_lock_init() {
  if (lock_handle_init(&cjson_lock)) {
    return SC_CONF_LOCK_INIT;
  }
  return SC_OK;
}

status_t apis_lock_destroy() {
  if (lock_handle_destroy(&cjson_lock)) {
    return SC_CONF_LOCK_DESTROY;
  }
  return SC_OK;
}

status_t api_get_tips(const iota_client_service_t* const service, char** json_result) {
  status_t ret = SC_OK;
  get_tips_res_t* res = get_tips_res_new();

  if (res == NULL) {
    ret = SC_CCLIENT_OOM;
    ta_log_error("%s\n", "SC_CCLIENT_OOM");
    goto done;
  }

  lock_handle_lock(&cjson_lock);
  if (iota_client_get_tips(service, res) != RC_OK) {
    lock_handle_unlock(&cjson_lock);
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  ret = ta_get_tips_res_serialize(res, json_result);
  if (ret != SC_OK) {
    ret = SC_CCLIENT_JSON_PARSE;
    ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
    goto done;
  }

done:
  get_tips_res_free(&res);
  return ret;
}

status_t api_get_tips_pair(const iota_config_t* const iconf, const iota_client_service_t* const service,
                           char** json_result) {
  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* res = get_transactions_to_approve_res_new();
  char_buffer_t* res_buff = char_buffer_new();

  if (req == NULL || res == NULL || res_buff == NULL) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
    goto done;
  }

  get_transactions_to_approve_req_set_depth(req, iconf->milestone_depth);
  lock_handle_lock(&cjson_lock);
  if (iota_client_get_transactions_to_approve(service, req, res) != RC_OK) {
    lock_handle_unlock(&cjson_lock);
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  if (service->serializer.vtable.get_transactions_to_approve_serialize_response(res, res_buff) != RC_OK) {
    ret = SC_CCLIENT_JSON_PARSE;
    ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
    goto done;
  }

  *json_result = (char*)malloc((res_buff->length + 1) * sizeof(char));
  if (*json_result == NULL) {
    ret = SC_CCLIENT_JSON_PARSE;
    ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
    goto done;
  }
  snprintf(*json_result, (res_buff->length + 1), "%s", res_buff->data);

done:
  get_transactions_to_approve_req_free(&req);
  get_transactions_to_approve_res_free(&res);
  char_buffer_free(res_buff);
  return ret;
}

status_t api_generate_address(const iota_config_t* const iconf, const iota_client_service_t* const service,
                              char** json_result) {
  status_t ret = SC_OK;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
    goto done;
  }

  lock_handle_lock(&cjson_lock);
  ret = ta_generate_address(iconf, service, res);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  ret = ta_generate_address_res_serialize(res, json_result);

done:
  ta_generate_address_res_free(&res);
  return ret;
}

status_t api_find_transaction_object_single(const iota_client_service_t* const service, const char* const obj,
                                            char** json_result) {
  status_t ret = SC_OK;
  flex_trit_t txn_hash[NUM_FLEX_TRITS_HASH];
  ta_find_transaction_objects_req_t* req = ta_find_transaction_objects_req_new();
  transaction_array_t* res = transaction_array_new();
  if (req == NULL || res == NULL) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
    goto done;
  }

  flex_trits_from_trytes(txn_hash, NUM_TRITS_HASH, (const tryte_t*)obj, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_queue_push(&req->hashes, txn_hash);

  lock_handle_lock(&cjson_lock);
  ret = ta_find_transaction_objects(service, req, res);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    ta_log_error("%d\n", ret);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

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
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
    goto done;
  }

  lock_handle_lock(&cjson_lock);
  ret = ta_find_transaction_objects_req_deserialize(obj, req);
  if (ret != SC_OK) {
    lock_handle_unlock(&cjson_lock);
    ta_log_error("%d\n", ret);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  lock_handle_lock(&cjson_lock);
  ret = ta_find_transaction_objects(service, req, res);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    ta_log_error("%d\n", ret);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

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
  if (req == NULL || res == NULL) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
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
    ta_log_error("%s\n", "SC_CCLIENT_INVALID_FLEX_TRITS");
    goto done;
  }

  lock_handle_lock(&cjson_lock);
  if (iota_client_find_transactions(service, req, res) != RC_OK) {
    lock_handle_unlock(&cjson_lock);
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

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
  if (req == NULL || res == NULL) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
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
    ta_log_error("%s\n", "SC_CCLIENT_INVALID_FLEX_TRITS");
    goto done;
  }

  lock_handle_lock(&cjson_lock);
  ret = ta_find_transactions_obj_by_tag(service, req, res);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    ta_log_error("%d\n", ret);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  ret = ta_find_transaction_objects_res_serialize(res, json_result);

done:
  find_transactions_req_free(&req);
  transaction_array_free(res);
  return ret;
}

status_t api_receive_mam_message(const iota_config_t* const iconf, const iota_client_service_t* const service,
                                 const char* const chid, char** json_result) {
  status_t ret = SC_OK;
  char* payload = NULL;
  mam_api_t mam;
  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);

  // TODO We may need to use encryption here
  // Creating MAM API
  retcode_t rc = mam_api_load(iconf->mam_file_path, &mam, NULL, 0);
  if (rc == RC_UTILS_FAILED_TO_OPEN_FILE) {
    if (mam_api_init(&mam, (tryte_t*)SEED) != RC_OK) {
      ret = SC_MAM_FAILED_INIT;
      ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
      goto done;
    }
  } else if (rc != RC_OK) {
    ret = SC_MAM_FAILED_INIT;
    ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
    goto done;
  }

  mam_api_add_trusted_channel_pk(&mam, (tryte_t*)chid);
  ret = ta_get_bundle_by_addr(service, (tryte_t*)chid, bundle);
  if (ret != SC_OK) {
    goto done;
  }

  if (map_api_bundle_read(&mam, bundle, &payload) != RC_OK) {
    ret = SC_MAM_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_MAM_FAILED_RESPONSE");
    goto done;
  }

  ret = receive_mam_message_res_serialize(payload, json_result);

done:
  // Destroying MAM API
  if (ret != SC_MAM_FAILED_INIT) {
    if (mam_api_save(&mam, iconf->mam_file_path, NULL, 0) != RC_OK) {
      ta_log_error("%s\n", "SC_MAM_FILE_SAVE");
    }
    if (mam_api_destroy(&mam) != RC_OK) {
      ret = SC_MAM_FAILED_DESTROYED;
      ta_log_error("%s\n", "SC_MAM_FAILED_DESTROYED");
    }
  }
  bundle_transactions_free(&bundle);
  free(payload);
  return ret;
}

status_t api_mam_send_message(const iota_config_t* const iconf, const iota_client_service_t* const service,
                              char const* const payload, char** json_result) {
  status_t ret = SC_OK;
  mam_api_t mam;
  const bool last_packet = true;
  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);
  tryte_t* prng = NULL;
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE];
  trit_t msg_id[MAM_MSG_ID_SIZE];
  ta_send_mam_req_t* req = send_mam_req_new();
  ta_send_mam_res_t* res = send_mam_res_new();

  lock_handle_lock(&cjson_lock);
  if (send_mam_req_deserialize(payload, req)) {
    lock_handle_unlock(&cjson_lock);
    ret = SC_MAM_FAILED_INIT;
    ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  // Creating MAM API
  prng = (req->prng[0]) ? req->prng : (tryte_t*)SEED;
  retcode_t rc = mam_api_load(iconf->mam_file_path, &mam, NULL, 0);
  if (rc == RC_UTILS_FAILED_TO_OPEN_FILE) {
    if (mam_api_init(&mam, prng) != RC_OK) {
      ret = SC_MAM_FAILED_INIT;
      ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
      goto done;
    }
  } else if (rc != RC_OK) {
    ret = SC_MAM_FAILED_INIT;
    ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
    goto done;
  }

  // Creating channel
  // TODO: TA only supports mss depth under 2. Larger than this would result in wasting too much time generate keys.
  // With depth of 2, user can send a bundle with  3 transaction messages filled (last one is for header).
  size_t payload_size = strlen(req->payload);
  size_t payload_depth = 1;
  if (payload_size > (NUM_TRYTES_SIGNATURE / 2) * 3) {
    ret = SC_MAM_NO_PAYLOAD;
    ta_log_error("%s\n", "SC_MAM_NO_PAYLOAD");
    goto done;
  } else if (payload_size > (NUM_TRYTES_SIGNATURE / 2)) {
    payload_depth = 2;
  }

  // TODO We needs to examine if this channel has been used or not with API `find_transactions`.
  // In order to avoid wasting address (channel), even when user's request assigns a much bigger channel_ord than mam
  // file, after sending this current MAM message, we need to iterate the next unused channel.
  if (req->channel_ord > mam.channel_ord) {
    mam.channel_ord = req->channel_ord;
  }

  if (map_channel_create(&mam, channel_id, payload_depth)) {
    ret = SC_MAM_NULL;
    ta_log_error("%s\n", "SC_MAM_NULL");
    goto done;
  }

  // Writing header to bundle
  if (map_write_header_on_channel(&mam, channel_id, bundle, msg_id)) {
    ret = SC_MAM_FAILED_WRITE;
    ta_log_error("%s\n", "SC_MAM_FAILED_WRITE");
    goto done;
  }

  // Writing packet to bundle
  if (map_write_packet(&mam, bundle, req->payload, msg_id, last_packet)) {
    ret = SC_MAM_FAILED_WRITE;
    ta_log_error("%s\n", "SC_MAM_FAILED_WRITE");
    goto done;
  }
  send_mam_res_set_channel_id(res, channel_id);

  // Sending bundle
  lock_handle_lock(&cjson_lock);
  if (ta_send_bundle(iconf, service, bundle) != SC_OK) {
    lock_handle_unlock(&cjson_lock);
    ret = SC_MAM_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_MAM_FAILED_RESPONSE");
    goto done;
  }
  lock_handle_unlock(&cjson_lock);
  ret = send_mam_res_set_bundle_hash(res, transaction_bundle((iota_transaction_t*)utarray_front(bundle)));
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }
  res->channel_ord = mam.channel_ord;
  ret = send_mam_res_serialize(res, json_result);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  // TODO Find the unused channel with the smallest channel_ord here.

done:
  // Destroying MAM API
  if (ret != SC_MAM_FAILED_INIT) {
    if (mam_api_save(&mam, iconf->mam_file_path, NULL, 0) != RC_OK) {
      ta_log_error("%s\n", "SC_MAM_FILE_SAVE");
    }
    if (mam_api_destroy(&mam)) {
      ret = SC_MAM_FAILED_DESTROYED;
      ta_log_error("%s\n", "SC_MAM_FAILED_DESTROYED");
    }
  }
  bundle_transactions_free(&bundle);
  send_mam_req_free(&req);
  send_mam_res_free(&res);
  return ret;
}

status_t api_send_transfer(const iota_config_t* const iconf, const iota_client_service_t* const service,
                           const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  ta_find_transaction_objects_req_t* txn_obj_req = ta_find_transaction_objects_req_new();
  transaction_array_t* res_txn_array = transaction_array_new();

  if (req == NULL || res == NULL || txn_obj_req == NULL || res_txn_array == NULL) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
    goto done;
  }

  lock_handle_lock(&cjson_lock);
  ret = ta_send_transfer_req_deserialize(obj, req);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }

  ret = ta_send_transfer(iconf, service, req, res);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  // return transaction object
  hash243_queue_push(&txn_obj_req->hashes, hash243_queue_peek(res->hash));

  lock_handle_lock(&cjson_lock);
  ret = ta_find_transaction_objects(service, txn_obj_req, res_txn_array);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  ret = ta_send_transfer_res_serialize(res_txn_array, json_result);

done:
  ta_send_transfer_req_free(&req);
  ta_send_transfer_res_free(&res);
  ta_find_transaction_objects_req_free(&txn_obj_req);
  transaction_array_free(res_txn_array);
  return ret;
}

status_t api_send_trytes(const iota_config_t* const iconf, const iota_client_service_t* const service,
                         const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  hash8019_array_p trytes = hash8019_array_new();

  if (!trytes) {
    ret = SC_TA_OOM;
    ta_log_error("%s\n", "SC_TA_OOM");
    goto done;
  }

  lock_handle_lock(&cjson_lock);
  ret = ta_send_trytes_req_deserialize(obj, trytes);
  if (ret != SC_OK) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }

  ret = ta_send_trytes(iconf, service, trytes);
  if (ret != SC_OK) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  ret = ta_send_trytes_res_serialize(trytes, json_result);

done:
  hash_array_free(trytes);
  return ret;
}
