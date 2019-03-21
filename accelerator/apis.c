#include "apis.h"

// TODO: Generate actual pre shared keys
static mam_psk_t const psk = {
    .id = {1,  0,  -1, -1, 0,  -1, -1, 0,  0,  1,  -1, 0,  1,  0,  0,  1,  1,
           1,  -1, 1,  1,  0,  1,  1,  0,  0,  -1, 1,  -1, -1, -1, -1, -1, -1,
           -1, 1,  -1, -1, 0,  -1, -1, 1,  0,  -1, -1, -1, 1,  1,  1,  0,  0,
           -1, 1,  -1, -1, -1, 0,  -1, 1,  -1, -1, -1, 1,  1,  -1, 1,  0,  0,
           1,  1,  1,  -1, -1, 0,  0,  -1, -1, 1,  0,  -1, 1},
    .key = {-1, 1,  -1, -1, 1,  -1, -1, 0,  0,  0,  -1, -1, 1,  1,  1,  -1, -1,
            -1, 0,  0,  0,  0,  -1, -1, 1,  1,  1,  0,  -1, -1, -1, 0,  0,  0,
            -1, -1, 1,  -1, 0,  0,  1,  0,  0,  -1, 1,  1,  0,  -1, 0,  0,  1,
            -1, 1,  0,  1,  0,  0,  -1, 1,  1,  -1, 1,  0,  -1, 0,  -1, 1,  -1,
            -1, -1, 0,  -1, -1, 0,  -1, -1, 0,  0,  -1, -1, 1,  -1, 0,  0,  -1,
            -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, 1,  1,  1,  1,  1,  0,  1,
            0,  1,  -1, 0,  0,  1,  0,  1,  0,  0,  1,  0,  -1, 0,  1,  1,  0,
            0,  -1, -1, 1,  1,  0,  0,  1,  -1, 1,  1,  1,  0,  1,  1,  1,  0,
            0,  -1, -1, -1, -1, 1,  1,  1,  0,  0,  -1, 0,  1,  -1, 1,  1,  1,
            0,  0,  1,  -1, -1, 0,  -1, 1,  -1, 1,  0,  0,  1,  -1, 0,  1,  -1,
            0,  0,  1,  1,  1,  1,  1,  0,  0,  1,  -1, 1,  -1, 1,  0,  1,  1,
            1,  -1, 0,  0,  -1, 1,  1,  0,  -1, -1, 0,  0,  -1, 1,  0,  1,  -1,
            0,  0,  -1, 1,  -1, 1,  1,  1,  -1, 0,  1,  1,  0,  0,  -1, -1, -1,
            0,  0,  1,  0,  1,  0,  -1, 1,  -1, 0,  1,  0,  -1, 1,  1,  -1, -1,
            0,  0,  -1, 0,  -1}};

status_t api_get_tips(const iota_client_service_t* const service,
                      char** json_result) {
  status_t ret = SC_OK;
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = cclient_get_tips(service, res);
  if (ret) {
    goto done;
  }

  ret = ta_get_tips_res_serialize(json_result, res);

done:
  ta_get_tips_res_free(&res);
  return ret;
}

status_t api_get_tips_pair(const iota_config_t* const config,
                           const iota_client_service_t* const service,
                           char** json_result) {
  status_t ret = SC_OK;
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = cclient_get_txn_to_approve(service, config->depth, res);
  if (ret) {
    goto done;
  }

  ret = ta_get_tips_res_serialize(json_result, res);

done:
  ta_get_tips_res_free(&res);
  return ret;
}

status_t api_generate_address(const iota_config_t* const config,
                              const iota_client_service_t* const service,
                              char** json_result) {
  status_t ret = SC_OK;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = ta_generate_address(config, service, res);
  if (ret) {
    goto done;
  }

  ret = ta_generate_address_res_serialize(json_result, res);

done:
  ta_generate_address_res_free(&res);
  return ret;
}

status_t api_get_transaction_object(const iota_client_service_t* const service,
                                    const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  ta_get_transaction_object_res_t* res = ta_get_transaction_object_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = ta_get_transaction_object(service, obj, res);
  if (ret) {
    goto done;
  }

  ret = ta_get_transaction_object_res_serialize(json_result, res);

done:
  ta_get_transaction_object_res_free(&res);
  return ret;
}

status_t api_find_transactions_by_tag(
    const iota_client_service_t* const service, const char* const obj,
    char** json_result) {
  status_t ret = SC_OK;
  ta_find_transactions_res_t* res = ta_find_transactions_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = ta_find_transactions_by_tag(service, obj, res);
  if (ret) {
    goto done;
  }

  ret = ta_find_transactions_res_serialize(json_result, res);

done:
  ta_find_transactions_res_free(&res);
  return ret;
}

status_t api_find_transactions_obj_by_tag(
    const iota_client_service_t* const service, const char* const obj,
    char** json_result) {
  status_t ret = SC_OK;
  ta_find_transactions_obj_res_t* res = ta_find_transactions_obj_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = ta_find_transactions_obj_by_tag(service, obj, res);
  if (ret) {
    goto done;
  }

  ret = ta_find_transactions_obj_res_serialize(json_result, res);

done:
  ta_find_transactions_obj_res_free(&res);
  return ret;
}

status_t api_receive_mam_message(const iota_client_service_t* const service,
                                 const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  mam_api_t mam;

  tryte_t* payload_trytes = NULL;
  size_t payload_size = 0;
  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);
  bool is_last_packet;

  // Creating MAM API
  if (mam_api_init(&mam, (tryte_t*)SEED)) {
    ret = SC_MAM_OOM;
    goto done;
  }

  // Get bundle which is find_transactions_by_bundle
  ret = ta_get_bundle(service, (tryte_t*)obj, bundle);
  if (ret) {
    goto done;
  }

  // Read MAM message from bundle
  mam_psk_t_set_add(&mam.psks, &psk);
  if (mam_api_bundle_read(&mam, bundle, &payload_trytes, &payload_size,
                          &is_last_packet) == RC_OK) {
    if (payload_trytes == NULL || payload_size == 0) {
      ret = SC_MAM_NULL;
    } else {
      char* payload = calloc(payload_size * 2 + 1, sizeof(char));

      trytes_to_ascii(payload_trytes, payload_size, payload);
      if (payload == NULL) {
        ret = SC_MAM_NOT_FOUND;
        goto done;
      }
      *json_result = payload;

      payload = NULL;
      free(payload_trytes);
    }
  } else {
    ret = SC_MAM_FAILED_RESPONSE;
  }

done:
  mam_api_destroy(&mam);
  bundle_transactions_free(&bundle);
  return ret;
}

status_t api_send_transfer(const iota_config_t* const config,
                           const iota_client_service_t* const service,
                           const char* const obj, char** json_result) {
  status_t ret = SC_OK;
  char hash_trytes[NUM_TRYTES_HASH + 1];
  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  ta_get_transaction_object_res_t* txn_obj_res =
      ta_get_transaction_object_res_new();

  if (req == NULL || res == NULL || txn_obj_res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = ta_send_transfer_req_deserialize(obj, req);
  if (ret) {
    goto done;
  }

  ret = ta_send_transfer(config, service, req, res);
  if (ret) {
    goto done;
  }

  // return transaction object
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH,
                       hash243_queue_peek(res->hash), NUM_TRITS_HASH,
                       NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  ret = ta_get_transaction_object(service, hash_trytes, txn_obj_res);
  if (ret) {
    goto done;
  }

  ret = ta_get_transaction_object_res_serialize(json_result, txn_obj_res);

done:
  ta_send_transfer_req_free(&req);
  ta_send_transfer_res_free(&res);
  ta_get_transaction_object_res_free(&txn_obj_res);
  return ret;
}
