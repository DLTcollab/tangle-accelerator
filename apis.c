#include "apis.h"

int api_get_tips(const iota_client_service_t* const service,
                 const char* const obj, char* json_result) {
  int ret = 0;
  ta_get_tips_req_t* req = ta_get_tips_req_new();
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  if (req == NULL || res == NULL) {
    goto done;
  }

  ret = ta_get_tips_req_deserialize(obj, req);
  if (ret) {
    goto done;
  }

  ret = ta_get_tips(service, req, res);
  if (ret) {
    goto done;
  }

  ret = ta_get_tips_res_serialize(&json_result, res);

done:
  ta_get_tips_req_free(req);
  ta_get_tips_res_free(&res);
  return ret;
}

int api_generate_address(const iota_client_service_t* const service,
                         char* json_result) {
  int ret = 0;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  if (res == NULL) {
    goto done;
  }

  ret = ta_generate_address(service, res);
  if (ret) {
    goto done;
  }

  ret = ta_generate_address_res_serialize(&json_result, res);

done:
  ta_generate_address_res_free(&res);
  return ret;
}
