#include "apis.h"

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

status_t api_get_tips_pair(const iota_client_service_t* const service,
                           char** json_result) {
  status_t ret = SC_OK;
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = cclient_get_txn_to_approve(service, res);
  if (ret) {
    goto done;
  }

  ret = ta_get_tips_res_serialize(json_result, res);

done:
  ta_get_tips_res_free(&res);
  return ret;
}

status_t api_generate_address(const iota_client_service_t* const service,
                              char** json_result) {
  status_t ret = SC_OK;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  if (res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  ret = ta_generate_address(service, res);
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

status_t api_send_transfer(const iota_client_service_t* const service,
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

  ret = ta_send_transfer(service, req, res);
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
