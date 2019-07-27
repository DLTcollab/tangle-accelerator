/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "apis.h"
#include "map/mode.h"

#define APIS_LOGGER "apis"

static logger_id_t apis_logger_id;

void apis_logger_init() { apis_logger_id = logger_helper_enable(APIS_LOGGER, LOGGER_DEBUG, true); }

int apis_logger_release() {
  logger_helper_release(apis_logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(apis_logger_id, "[%s:%d] Destroying logger failed %s.\n", __func__, __LINE__, APIS_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t api_get_tips(const iota_client_service_t* const service, char** json_result) {
  status_t ret = SC_OK;
  get_tips_res_t* res = get_tips_res_new();
  char_buffer_t* res_buff = char_buffer_new();

  if (res == NULL || res_buff == NULL) {
    ret = SC_CCLIENT_OOM;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_OOM");
    goto done;
  }

  if (iota_client_get_tips(service, res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  if (service->serializer.vtable.get_tips_serialize_response(res, res_buff) != RC_OK) {
    ret = SC_CCLIENT_JSON_PARSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    goto done;
  }

  *json_result = (char*)malloc((res_buff->length + 1) * sizeof(char));
  if (*json_result == NULL) {
    ret = SC_CCLIENT_JSON_PARSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    goto done;
  }
  snprintf(*json_result, (res_buff->length + 1), "%s", res_buff->data);

done:
  get_tips_res_free(&res);
  char_buffer_free(res_buff);
  return ret;
}

status_t api_get_tips_pair(const iota_config_t* const tangle, const iota_client_service_t* const service,
                           char** json_result) {
  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* res = get_transactions_to_approve_res_new();
  char_buffer_t* res_buff = char_buffer_new();

  if (req == NULL || res == NULL || res_buff == NULL) {
    ret = SC_TA_OOM;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    goto done;
  }

  get_transactions_to_approve_req_set_depth(req, tangle->milestone_depth);
  if (iota_client_get_transactions_to_approve(service, req, res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  if (service->serializer.vtable.get_transactions_to_approve_serialize_response(res, res_buff) != RC_OK) {
    ret = SC_CCLIENT_JSON_PARSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    goto done;
  }

  *json_result = (char*)malloc((res_buff->length + 1) * sizeof(char));
  if (*json_result == NULL) {
    ret = SC_CCLIENT_JSON_PARSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    goto done;
  }
  snprintf(*json_result, (res_buff->length + 1), "%s", res_buff->data);

done:
  get_transactions_to_approve_req_free(&req);
  get_transactions_to_approve_res_free(&res);
  char_buffer_free(res_buff);
  return ret;
}

status_t api_generate_address(const iota_config_t* const tangle, const iota_client_service_t* const service,
                              char** json_result) {
  status_t ret = SC_OK;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    goto done;
  }

  ret = ta_generate_address(tangle, service, res);
  if (ret) {
    goto done;
  }

  ret = ta_generate_address_res_serialize(json_result, res);

done:
  ta_generate_address_res_free(&res);
  return ret;
}

status_t api_find_transactions(const iota_client_service_t* const service, const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  find_transactions_req_t* req = find_transactions_req_new();
  find_transactions_res_t* res = find_transactions_res_new();
  char_buffer_t* res_buff = char_buffer_new();
  if (req == NULL || res == NULL || res_buff == NULL) {
    ret = SC_TA_OOM;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    goto done;
  }
  if (service->serializer.vtable.find_transactions_deserialize_request(obj, req) != RC_OK) {
    ret = SC_CCLIENT_JSON_PARSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    goto done;
  }

  if (iota_client_find_transactions(service, req, res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  if (service->serializer.vtable.find_transactions_serialize_response(res, res_buff) != RC_OK) {
    ret = SC_CCLIENT_JSON_PARSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    goto done;
  }

  *json_result = (char*)malloc((res_buff->length + 1) * sizeof(char));
  if (*json_result == NULL) {
    ret = SC_CCLIENT_JSON_PARSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    goto done;
  }
  snprintf(*json_result, (res_buff->length + 1), "%s", res_buff->data);

done:
  find_transactions_req_free(&req);
  find_transactions_res_free(&res);
  char_buffer_free(res_buff);
  return ret;
}

status_t api_find_transaction_objects(const iota_client_service_t* const service, const char* const obj,
                                      char** json_result) {
  status_t ret = SC_OK;
  ta_find_transaction_objects_req_t* req = ta_find_transaction_objects_req_new();
  transaction_array_t* res = transaction_array_new();
  if (req == NULL || res == NULL) {
    ret = SC_TA_OOM;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    goto done;
  }

  ret = ta_find_transaction_objects_req_deserialize(obj, req);
  if (ret != SC_OK) {
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  ret = ta_find_transaction_objects(service, req, res);
  if (ret) {
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  ret = ta_find_transaction_objects_res_serialize(json_result, res);

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
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)obj, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  if (find_transactions_req_tag_add(req, tag_trits) != RC_OK) {
    ret = SC_CCLIENT_INVALID_FLEX_TRITS;
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  if (iota_client_find_transactions(service, req, res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  ret = ta_find_transactions_res_serialize(json_result, (ta_find_transactions_res_t*)res);

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
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)obj, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  if (find_transactions_req_tag_add(req, tag_trits) != RC_OK) {
    ret = SC_CCLIENT_INVALID_FLEX_TRITS;
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  ret = ta_find_transactions_obj_by_tag(service, req, res);
  if (ret) {
    log_error(apis_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  ret = ta_find_transaction_objects_res_serialize(json_result, res);

done:
  find_transactions_req_free(&req);
  transaction_array_free(res);
  return ret;
}

status_t api_receive_mam_message(const iota_client_service_t* const service, const char* const chid,
                                 char** json_result) {
  status_t ret = SC_OK;
  char* payload = NULL;
  mam_api_t mam;
  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);

  // Creating MAM API
  if (mam_api_init(&mam, (tryte_t*)SEED) != RC_OK) {
    ret = SC_MAM_FAILED_INIT;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_INIT");
    goto done;
  }

  flex_trit_t chid_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(chid_trits, NUM_TRITS_HASH, (const tryte_t*)chid, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  mam_api_add_trusted_channel_pk(&mam, chid_trits);
  ret = ta_get_bundle_by_addr(service, chid_trits, bundle);
  if (ret != SC_OK) {
    goto done;
  }

  if (map_api_bundle_read(&mam, bundle, &payload) != RC_OK) {
    ret = SC_MAM_FAILED_RESPONSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_RESPONSE");
    goto done;
  }

  ret = receive_mam_message_serialize(json_result, &payload);

done:
  // Destroying MAM API
  if (ret != SC_MAM_FAILED_INIT) {
    if (mam_api_destroy(&mam) != RC_OK) {
      ret = SC_MAM_FAILED_DESTROYED;
      log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_DESTROYED");
    }
  }
  bundle_transactions_free(&bundle);
  free(payload);
  return ret;
}

status_t api_mam_send_message(const iota_config_t* const tangle, const iota_client_service_t* const service,
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

  if (send_mam_req_deserialize(payload, req)) {
    ret = SC_MAM_FAILED_INIT;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_INIT");
    goto done;
  }

  // Creating MAM API
  prng = (req->prng[0]) ? req->prng : SEED;
  if (mam_api_init(&mam, prng) != RC_OK) {
    ret = SC_MAM_FAILED_INIT;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_INIT");
    goto done;
  }

  // Creating channel
  // TODO: TA only supports mss depth under 2. Larger than this would result in wasting too much time generate keys.
  // With depth of 2, user can send a bundle with  3 transaction messages filled (last one is for header).
  size_t payload_size = strlen(req->payload);
  size_t payload_depth = 1;
  if (payload_size > (NUM_TRYTES_SIGNATURE / 2) * 3) {
    ret = SC_MAM_NO_PAYLOAD;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_NO_PAYLOAD");
    goto done;
  } else if (payload_size > (NUM_TRYTES_SIGNATURE / 2)) {
    payload_depth = 2;
  }
  mam.channel_ord = req->channel_ord;
  if (map_channel_create(&mam, channel_id, payload_depth)) {
    ret = SC_MAM_NULL;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_NULL");
    goto done;
  }

  // Writing header to bundle
  if (map_write_header_on_channel(&mam, channel_id, bundle, msg_id)) {
    ret = SC_MAM_FAILED_WRITE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_WRITE");
    goto done;
  }

  // Writing packet to bundle
  if (map_write_packet(&mam, bundle, req->payload, msg_id, last_packet)) {
    ret = SC_MAM_FAILED_WRITE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_WRITE");
    goto done;
  }
  send_mam_res_set_channel_id(res, channel_id);

  // Sending bundle
  if (ta_send_bundle(tangle, service, bundle) != SC_OK) {
    ret = SC_MAM_FAILED_RESPONSE;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_RESPONSE");
    goto done;
  }
  send_mam_res_set_bundle_hash(res, transaction_bundle((iota_transaction_t*)utarray_front(bundle)));

  ret = send_mam_res_serialize(json_result, res);

done:
  // Destroying MAM API
  if (ret != SC_MAM_FAILED_INIT) {
    if (mam_api_destroy(&mam)) {
      ret = SC_MAM_FAILED_DESTROYED;
      log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_FAILED_DESTROYED");
    }
  }
  bundle_transactions_free(&bundle);
  send_mam_req_free(&req);
  send_mam_res_free(&res);
  return ret;
}

status_t api_send_transfer(const iota_config_t* const tangle, const iota_client_service_t* const service,
                           const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  ta_find_transaction_objects_req_t* txn_obj_req = ta_find_transaction_objects_req_new();
  transaction_array_t* res_txn_array = transaction_array_new();

  if (req == NULL || res == NULL || txn_obj_req == NULL || res_txn_array == NULL) {
    ret = SC_TA_OOM;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    goto done;
  }

  ret = ta_send_transfer_req_deserialize(obj, req);
  if (ret) {
    goto done;
  }

  ret = ta_send_transfer(tangle, service, req, res);
  if (ret) {
    goto done;
  }

  // return transaction object
  hash243_queue_push(&txn_obj_req->hashes, hash243_queue_peek(res->hash));

  ret = ta_find_transaction_objects(service, txn_obj_req, res_txn_array);
  if (ret) {
    goto done;
  }

  ret = ta_find_transaction_objects_res_serialize(json_result, res_txn_array);

done:
  ta_send_transfer_req_free(&req);
  ta_send_transfer_res_free(&res);
  ta_find_transaction_objects_req_free(&txn_obj_req);
  transaction_array_free(res_txn_array);
  return ret;
}

status_t api_send_trytes(const iota_config_t* const tangle, const iota_client_service_t* const service,
                         const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  hash8019_array_p trytes = hash8019_array_new();

  if (!trytes) {
    ret = SC_TA_OOM;
    log_error(apis_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    goto done;
  }

  ret = ta_send_trytes_req_deserialize(obj, trytes);
  if (ret != SC_OK) {
    goto done;
  }

  ret = ta_send_trytes(tangle, service, trytes);
  if (ret != SC_OK) {
    goto done;
  }

  ret = ta_send_trytes_res_serialize(trytes, json_result);

done:
  hash_array_free(trytes);
  return ret;
}
