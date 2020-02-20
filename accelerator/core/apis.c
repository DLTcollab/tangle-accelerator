/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "apis.h"
#include <sys/time.h>
#include "mam_core.h"
#include "utils/handles/lock.h"

#define APIS_LOGGER "apis"

static logger_id_t logger_id;
lock_handle_t cjson_lock;

void apis_logger_init() { logger_id = logger_helper_enable(APIS_LOGGER, LOGGER_DEBUG, true); }

int apis_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", APIS_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t api_get_ta_info(ta_config_t* const info, iota_config_t* const tangle, ta_cache_t* const cache,
                         char** json_result) {
  return ta_get_info_serialize(json_result, info, tangle, cache);
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
    ret = SC_TA_ERROR;
    ta_log_error("%s\n", "SC_TA_ERROR");
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
  ret = ta_get_bundles_by_addr(service, (tryte_t*)chid, bundle);
  if (ret != SC_OK) {
    goto done;
  }

  if (ta_mam_api_bundle_read(&mam, bundle, &payload) != RC_OK) {
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

status_t api_mam_send_message(const ta_config_t* const info, const iota_config_t* const iconf,
                              const iota_client_service_t* const service, char const* const payload,
                              char** json_result) {
  status_t ret = SC_OK;
  mam_api_t mam;
  tryte_t chid[MAM_CHANNEL_ID_TRYTE_SIZE] = {}, epid[MAM_CHANNEL_ID_TRYTE_SIZE] = {},
          chid1[MAM_CHANNEL_ID_TRYTE_SIZE] = {}, msg_id[NUM_TRYTES_MAM_MSG_ID] = {};
  uint32_t ch_remain_sk, ep_remain_sk;
  mam_psk_t_set_t psks = NULL;
  mam_ntru_pk_t_set_t ntru_pks = NULL;

  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);

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
  ret = ta_mam_init(&mam, iconf, req->seed, req->channel_ord, &psks, &ntru_pks, (tryte_t*)req->psk,
                    (tryte_t*)req->ntru_pk);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  // Create epid merkle tree and find the smallest unused secret key.
  // Write both Header and Pakcet into one single bundle.
  ret = ta_mam_written_msg_to_bundle(service, &mam, req->ch_mss_depth, req->ep_mss_depth, chid, epid, psks, ntru_pks,
                                     req->message, &bundle, msg_id, &ch_remain_sk, &ep_remain_sk);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  // Sending bundle
  lock_handle_lock(&cjson_lock);
  if (ta_send_bundle(info, iconf, service, bundle) != SC_OK) {
    lock_handle_unlock(&cjson_lock);
    ret = SC_MAM_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_MAM_FAILED_RESPONSE");
    goto done;
  }
  lock_handle_unlock(&cjson_lock);

  ret = send_mam_res_set_channel_id(res, chid);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }
  ret = send_mam_res_set_endpoint_id(res, epid);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }
  ret = send_mam_res_set_msg_id(res, msg_id);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }
  ret = send_mam_res_set_bundle_hash(res, transaction_bundle((iota_transaction_t*)utarray_front(bundle)));
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  bundle_transactions_free(&bundle);
  bundle_transactions_new(&bundle);

  // Sending bundle
  if (ch_remain_sk == 1 || ep_remain_sk == 1) {
    // Send announcement for the next endpoint or channel
    ret = ta_mam_announce_next_mss_private_key_to_bundle(&mam, req->ch_mss_depth, req->ep_mss_depth, chid,
                                                         &ch_remain_sk, &ep_remain_sk, psks, ntru_pks, chid1, &bundle);
    if (ret) {
      ta_log_error("%d\n", ret);
      goto done;
    }

    lock_handle_lock(&cjson_lock);
    if (ta_send_bundle(info, iconf, service, bundle) != SC_OK) {
      lock_handle_unlock(&cjson_lock);
      ret = SC_MAM_FAILED_RESPONSE;
      ta_log_error("%s\n", "SC_MAM_FAILED_RESPONSE");
      goto done;
    }
    lock_handle_unlock(&cjson_lock);

    ret =
        send_mam_res_set_announcement_bundle_hash(res, transaction_bundle((iota_transaction_t*)utarray_front(bundle)));
    if (ret) {
      ta_log_error("%d\n", ret);
      goto done;
    }

    ret = send_mam_res_set_chid1(res, chid1);
    if (ret) {
      ta_log_error("%d\n", ret);
      goto done;
    }
  }

  ret = send_mam_res_serialize(res, json_result);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

done:
  // Destroying MAM API
  if (ret != SC_MAM_FAILED_INIT) {
    // If `seed` is not assigned, then the local MAM file will be used. Therefore, we need to close the MAM file.
    if (!req->seed && mam_api_save(&mam, iconf->mam_file_path, NULL, 0) != RC_OK) {
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
  mam_psk_t_set_free(&psks);
  mam_ntru_pk_t_set_free(&ntru_pks);
  return ret;
}

status_t api_send_transfer(const ta_core_t* const core, const char* const obj, char** json_result) {
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

  ret = ta_send_transfer(&core->ta_conf, &core->iota_conf, &core->iota_service, req, res);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);
  // return transaction object
  hash243_queue_push(&txn_obj_req->hashes, hash243_queue_peek(res->hash));

  lock_handle_lock(&cjson_lock);
  ret = ta_find_transaction_objects(&core->iota_service, txn_obj_req, res_txn_array);
  if (ret) {
    lock_handle_unlock(&cjson_lock);
    goto done;
  }
  lock_handle_unlock(&cjson_lock);
  res->txn_array = res_txn_array;
#ifdef DB_ENABLE
  ret = db_insert_tx_into_identity(&core->db_service, res->hash, PENDING_TXN, res->uuid_string);
  if (ret != SC_OK) {
    ta_log_error("fail to insert new pending transaction for reattachement\n");
    goto done;
  }
#endif
  ret = ta_send_transfer_res_serialize(res, json_result);

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

  ret = ta_send_trytes(info, iconf, service, trytes);
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

#ifdef DB_ENABLE
status_t api_find_transactions_by_id(const iota_client_service_t* const iota_service,
                                     const db_client_service_t* const db_service, const char* const obj,
                                     char** json_result) {
  if (obj == NULL) {
    ta_log_error("Invalid NULL pointer to uuid string\n");
    return SC_TA_NULL;
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
    return SC_TA_NULL;
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
    return SC_TA_NULL;
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
