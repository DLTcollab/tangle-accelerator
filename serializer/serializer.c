#include "serializer.h"

int hash243_stack_to_json_array(hash243_stack_t stack, cJSON* const json_root,
                                char const* const obj_name) {
  size_t array_count = 0;
  cJSON* array_obj = NULL;
  hash243_stack_entry_t* s_iter = NULL;
  tryte_t trytes_out[NUM_TRYTES_HASH + 1];
  size_t trits_count = 0;

  array_count = hash243_stack_count(stack);
  if (array_count > 0) {
    array_obj = cJSON_CreateArray();
    if (array_obj == NULL) {
      return -1;
    }
    cJSON_AddItemToObject(json_root, obj_name, array_obj);

    LL_FOREACH(stack, s_iter) {
      trits_count =
          flex_trits_to_trytes(trytes_out, NUM_TRYTES_HASH, s_iter->hash,
                               NUM_TRITS_HASH, NUM_TRITS_HASH);
      trytes_out[NUM_TRYTES_HASH] = '\0';
      if (trits_count != 0) {
        cJSON_AddItemToArray(array_obj,
                             cJSON_CreateString((const char*)trytes_out));
      } else {
        return -1;
      }
    }
  }
  return 0;
}

int hash243_queue_to_json_array(hash243_queue_t queue, cJSON* const json_root,
                                char const* const obj_name) {
  size_t array_count;
  cJSON* array_obj = NULL;
  hash243_queue_entry_t* q_iter = NULL;

  array_count = hash243_queue_count(queue);
  if (array_count > 0) {
    array_obj = cJSON_CreateArray();
    if (array_obj == NULL) {
      return -1;
    }
    cJSON_AddItemToObject(json_root, obj_name, array_obj);

    CDL_FOREACH(queue, q_iter) {
      tryte_t trytes_out[NUM_TRYTES_HASH + 1];
      size_t trits_count =
          flex_trits_to_trytes(trytes_out, NUM_TRYTES_HASH, q_iter->hash,
                               NUM_TRITS_HASH, NUM_TRITS_HASH);
      trytes_out[NUM_TRYTES_HASH] = '\0';
      if (trits_count != 0) {
        cJSON_AddItemToArray(array_obj,
                             cJSON_CreateString((const char*)trytes_out));
      } else {
        return -1;
      }
    }
  }
  return 0;
}

int json_array_to_hash243_queue(cJSON const* const obj,
                                char const* const obj_name,
                                hash243_queue_t* queue) {
  retcode_t ret_code = RC_OK;
  flex_trit_t hash[FLEX_TRIT_SIZE_243] = {};
  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (cJSON_IsArray(json_item)) {
    cJSON* current_obj = NULL;
    cJSON_ArrayForEach(current_obj, json_item) {
      if (current_obj->valuestring != NULL) {
        flex_trits_from_trytes(hash, NUM_TRITS_HASH,
                               (const tryte_t*)current_obj->valuestring,
                               NUM_TRYTES_HASH, NUM_TRYTES_HASH);
        ret_code = hash243_queue_push(queue, hash);
        if (ret_code) {
          return -1;
        }
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
  int ret = 0;
  if (json_root == NULL) {
    return -1;
  }
  ret = hash243_queue_to_json_array(res->addresses, json_root, "address");
  if (ret) {
    return ret;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return -1;
  }
  cJSON_Delete(json_root);
  return ret;
}

int ta_get_tips_req_deserialize(const char* const obj, ta_get_tips_req_t* req) {
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;
  int ret = 0;

  if (json_obj == NULL) {
    ret = -1;
    goto done;
  }
  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "opt");
  if ((json_result != NULL) && cJSON_IsNumber(json_result)) {
    req->opt = json_result->valueint;
  } else {
    ret = -1;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

int ta_get_tips_res_serialize(char** obj, const ta_get_tips_res_t* const res) {
  cJSON* json_root = cJSON_CreateObject();
  int ret = 0;
  if (json_root == NULL) {
    return -1;
  }
  ret = hash243_stack_to_json_array(res->tips, json_root, "tips");
  if (ret) {
    return ret;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return ret;
  }
  cJSON_Delete(json_root);
  return ret;
}

int ta_send_transfer_req_deserialize(const char* const obj,
                                     ta_send_transfer_req_t* req) {
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;
  flex_trit_t tag_trits[NUM_TRITS_TAG], address_trits[NUM_TRITS_HASH];
  int msg_len = 0, ret;

  if (json_obj == NULL) {
    ret = -1;
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "value");
  if ((json_result != NULL) && cJSON_IsNumber(json_result)) {
    req->value = json_result->valueint;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "tag");
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG,
                         (const tryte_t*)json_result->valuestring,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  ret = hash81_queue_push(&req->tag, tag_trits);
  if (ret) {
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message");
  msg_len = strlen(json_result->valuestring);
  flex_trits_from_trytes(req->message, msg_len * 3,
                         (const tryte_t*)json_result->valuestring, msg_len,
                         msg_len);

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "address");
  flex_trits_from_trytes(address_trits, NUM_TRITS_HASH,
                         (const tryte_t*)json_result->valuestring,
                         NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  ret = hash243_queue_push(&req->address, address_trits);
  if (ret) {
    goto done;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

int ta_send_transfer_res_serialize(char** obj,
                                   const ta_send_transfer_res_t* const res) {
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    return -1;
  }

  char trytes_out[NUM_TRYTES_HASH + 1];
  flex_trits_to_trytes((tryte_t*)trytes_out, NUM_TRYTES_HASH, res->hash->hash,
                       NUM_TRITS_HASH, NUM_TRITS_HASH);
  trytes_out[NUM_TRYTES_HASH] = '\0';

  cJSON_AddStringToObject(json_root, "hash", trytes_out);
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return -1;
  }
  cJSON_Delete(json_root);
  return 0;
}

int ta_get_transaction_msg_res_serialize(
    char** obj, const ta_get_transaction_msg_res_t* const res) {
  int ret = 0;
  char msg_trytes[NUM_TRYTES_SIGNATURE + 1];
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = -1;
    goto done;
  }

  flex_trits_to_trytes((tryte_t*)msg_trytes, NUM_TRYTES_SIGNATURE, res->msg,
                       NUM_TRITS_SIGNATURE, NUM_TRITS_SIGNATURE);
  msg_trytes[NUM_TRYTES_SIGNATURE] = '\0';

  cJSON_AddStringToObject(json_root, "message", msg_trytes);
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = -1;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

int ta_find_transactions_res_serialize(
    char** obj, const ta_find_transactions_res_t* const res) {
  int ret = 0;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = -1;
    goto done;
  }

  hash243_queue_to_json_array(res->hashes, json_root, "hashes");
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = -1;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}
