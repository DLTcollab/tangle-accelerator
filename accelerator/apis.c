#include "apis.h"

int api_get_tips(const iota_client_service_t* const service,
                 char** json_result) {
  int ret = 0;
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  if (res == NULL) {
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

int api_get_tips_pair(const iota_client_service_t* const service,
                      char** json_result) {
  int ret = 0;
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  if (res == NULL) {
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

int api_generate_address(const iota_client_service_t* const service,
                         char** json_result) {
  int ret = 0;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  if (res == NULL) {
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

int api_get_transaction_object(const iota_client_service_t* const service,
                               const char* const obj, char** json_result) {
  int ret = 0;
  ta_get_transaction_object_res_t* res = ta_get_transaction_object_res_new();
  if (res == NULL) {
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

int api_find_transactions_by_tag(const iota_client_service_t* const service,
                                 const char* const obj, char** json_result) {
  int ret = 0;
  ta_find_transactions_res_t* res = ta_find_transactions_res_new();
  if (res == NULL) {
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
