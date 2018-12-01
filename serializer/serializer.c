#include "serializer.h"

/* This function is from cclient */
int flex_hash_array_to_json_array_p(const flex_hash_array_t* const head,
                                    cJSON* json_root, const char* obj_name) {
  flex_hash_array_t* elt;
  int array_count;
  cJSON* array_obj = NULL;

  LL_COUNT(head, elt, array_count);
  array_obj = cJSON_CreateArray();
  cJSON_AddItemToObject(json_root, obj_name, array_obj);
  if (array_count > 0) {
    LL_FOREACH(head, elt) {
      // flex to trytes;
      size_t len_trytes = elt->hash->num_trits / 3;
      char trytes_out[len_trytes + 1];
      flex_hash_to_trytes(elt->hash, trytes_out);
      trytes_out[len_trytes] = '\0';
      cJSON_AddItemToArray(array_obj,
                           cJSON_CreateString((const char*)trytes_out));
    }
  }
  return 0;
}

int flex_trit_array_to_json_array(const flex_trit_t* const head,
                                  cJSON* json_root, const char* obj_name) {
  cJSON* array_obj = cJSON_CreateArray();
  cJSON_AddItemToObject(json_root, obj_name, array_obj);
  // flex to trytes;
  size_t len_trytes = 243 / 3;
  trit_t trytes_out[len_trytes + 1];
  size_t trits_count =
      flex_trits_to_trytes(trytes_out, len_trytes, head, 243, 243);
  trytes_out[len_trytes] = '\0';
  if (trits_count != 0) {
    cJSON_AddItemToArray(array_obj,
                         cJSON_CreateString((const char*)trytes_out));
  }
  return 0;
}

int json_array_to_flex_hash_array_p(cJSON const* const obj,
                                    flex_hash_array_t* head) {
  if (cJSON_IsArray(obj)) {
    cJSON* current_obj = NULL;
    cJSON_ArrayForEach(current_obj, obj) {
      if (current_obj->valuestring != NULL) {
        head =
            flex_hash_array_append(head, (const char*)current_obj->valuestring);
      }
    }
  } else {
    return -1;
  }
  return 0;
}

int ta_generate_address_res_serialize(
    char** obj, const ta_generate_address_res_t* const res) {
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    return -1;
  }
  flex_hash_array_to_json_array_p(res, json_root, "address");
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return -1;
  }
  cJSON_Delete(json_root);
  return 0;
}

int ta_get_tips_req_deserialize(const char* const obj, ta_get_tips_req_t* req) {
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;

  if (json_obj == NULL) {
    return -1;
  }
  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "opt");
  if ((json_result != NULL) && cJSON_IsNumber(json_result)) {
    req->opt = json_result->valueint;
    return 0;
  }
  return -1;
}

int ta_get_tips_res_serialize(char** obj, const ta_get_tips_res_t* const res) {
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    return -1;
  }
  flex_hash_array_to_json_array_p(res, json_root, "tips");
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return -1;
  }
  cJSON_Delete(json_root);
  return 0;
}

int ta_send_transfer_req_deserialize(const char* const obj,
                                     ta_send_transfer_req_t* req) {
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;
  int string_len = 0;

  if (json_obj == NULL) {
    return -1;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "value");
  if ((json_result != NULL) && cJSON_IsNumber(json_result)) {
    req->value = json_result->valueint;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "tag");
  req->tag =
      trit_array_new_from_trytes((const tryte_t*)json_result->valuestring);

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message");
  req->message =
      trit_array_new_from_trytes((const tryte_t*)json_result->valuestring);

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "address");
  req->address =
      trit_array_new_from_trytes((const tryte_t*)json_result->valuestring);

  return 0;
}

int ta_send_transfer_res_serialize(char** obj,
                                   const ta_send_transfer_res_t* const res) {
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    return -1;
  }

  // flex_trit to trytes
  size_t len_trytes = res->bundle->num_trits / 3;
  char trytes_out[len_trytes + 1];
  flex_trits_to_trytes((tryte_t*)trytes_out, len_trytes, res->bundle->trits,
                       res->bundle->num_trits, res->bundle->num_trits);
  trytes_out[len_trytes] = '\0';

  cJSON_AddStringToObject(json_root, "bundle", trytes_out);
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return -1;
  }
  cJSON_Delete(json_root);
  return 0;
}
