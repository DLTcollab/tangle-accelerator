#include "serializer.h"

void fill_tag(char* new_tag, char* old_tag, size_t tag_len) {
  char nines[NUM_TRYTES_TAG] = "999999999999999999999999999";
  int pad_len = NUM_TRYTES_TAG - (int)tag_len;
  sprintf(new_tag, "%s%*.*s", old_tag, pad_len, pad_len, nines);
}

static status_t ta_hash243_stack_to_json_array(hash243_stack_t stack,
                                               cJSON* const json_root,
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
      return SC_SERIALIZER_JSON_CREATE;
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
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    return SC_CCLIENT_NOT_FOUND;
  }
  return SC_OK;
}

static status_t ta_hash243_queue_to_json_array(hash243_queue_t queue,
                                               cJSON* const json_root,
                                               char const* const obj_name) {
  size_t array_count;
  cJSON* array_obj = NULL;
  hash243_queue_entry_t* q_iter = NULL;

  array_count = hash243_queue_count(queue);
  if (array_count > 0) {
    array_obj = cJSON_CreateArray();
    if (array_obj == NULL) {
      return SC_SERIALIZER_JSON_CREATE;
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
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    return SC_CCLIENT_NOT_FOUND;
  }
  return SC_OK;
}

static status_t ta_json_get_string(cJSON const* const json_obj,
                                   char const* const obj_name,
                                   char* const text) {
  status_t ret = SC_OK;
  if (json_obj == NULL || obj_name == NULL || text == NULL) {
    return SC_SERIALIZER_NULL;
  }

  cJSON* json_value = cJSON_GetObjectItemCaseSensitive(json_obj, obj_name);
  if (json_value == NULL) {
    return SC_CCLIENT_JSON_KEY;
  }

  if (cJSON_IsString(json_value) && (json_value->valuestring != NULL)) {
    strcpy(text, json_value->valuestring);
  } else {
    return SC_CCLIENT_JSON_PARSE;
  }

  return ret;
}

status_t iota_transaction_to_json_object(iota_transaction_t const* const txn,
                                         cJSON** txn_json) {
  if (txn == NULL) {
    return SC_CCLIENT_NOT_FOUND;
  }
  char msg_trytes[NUM_TRYTES_SIGNATURE + 1], hash_trytes[NUM_TRYTES_HASH + 1],
      tag_trytes[NUM_TRYTES_TAG + 1];
  *txn_json = cJSON_CreateObject();
  if (txn_json == NULL) {
    return SC_SERIALIZER_JSON_CREATE;
  }

  // transaction hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH,
                       transaction_hash(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "hash", hash_trytes);

  // message
  flex_trits_to_trytes((tryte_t*)msg_trytes, NUM_TRYTES_SIGNATURE,
                       transaction_message(txn), NUM_TRITS_SIGNATURE,
                       NUM_TRITS_SIGNATURE);
  msg_trytes[NUM_TRYTES_SIGNATURE] = '\0';
  cJSON_AddStringToObject(*txn_json, "signature_and_message_fragment",
                          msg_trytes);

  // address
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH,
                       transaction_address(txn), NUM_TRITS_HASH,
                       NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "address", hash_trytes);
  // value
  cJSON_AddNumberToObject(*txn_json, "value", transaction_value(txn));
  // obsolete tag
  flex_trits_to_trytes((tryte_t*)tag_trytes, NUM_TRYTES_TAG,
                       transaction_obsolete_tag(txn), NUM_TRITS_TAG,
                       NUM_TRITS_TAG);
  tag_trytes[NUM_TRYTES_TAG] = '\0';
  cJSON_AddStringToObject(*txn_json, "obsolete_tag", tag_trytes);

  // timestamp
  cJSON_AddNumberToObject(*txn_json, "timestamp", transaction_timestamp(txn));

  // current index
  cJSON_AddNumberToObject(*txn_json, "current_index",
                          transaction_current_index(txn));

  // last index
  cJSON_AddNumberToObject(*txn_json, "last_index", transaction_last_index(txn));

  // bundle hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH,
                       transaction_bundle(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "bundle_hash", hash_trytes);

  // trunk transaction hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH,
                       transaction_trunk(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "trunk_transaction_hash", hash_trytes);

  // branch transaction hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH,
                       transaction_branch(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "branch_transaction_hash", hash_trytes);

  // tag
  flex_trits_to_trytes((tryte_t*)tag_trytes, NUM_TRYTES_TAG,
                       transaction_tag(txn), NUM_TRITS_TAG, NUM_TRITS_TAG);
  tag_trytes[NUM_TRYTES_TAG] = '\0';
  cJSON_AddStringToObject(*txn_json, "tag", tag_trytes);

  // attachment timestamp
  cJSON_AddNumberToObject(*txn_json, "attachment_timestamp",
                          transaction_attachment_timestamp(txn));

  // attachment lower timestamp
  cJSON_AddNumberToObject(*txn_json, "attachment_timestamp_lower_bound",
                          transaction_attachment_timestamp_lower(txn));

  // attachment upper timestamp
  cJSON_AddNumberToObject(*txn_json, "attachment_timestamp_upper_bound",
                          transaction_attachment_timestamp_upper(txn));

  // nonce
  flex_trits_to_trytes((tryte_t*)tag_trytes, NUM_TRYTES_NONCE,
                       transaction_nonce(txn), NUM_TRITS_NONCE,
                       NUM_TRITS_NONCE);
  tag_trytes[NUM_TRYTES_TAG] = '\0';
  cJSON_AddStringToObject(*txn_json, "nonce", tag_trytes);

  return SC_OK;
}

status_t ta_generate_address_res_serialize(
    char** obj, const ta_generate_address_res_t* const res) {
  cJSON* json_root = cJSON_CreateObject();
  status_t ret = SC_OK;
  if (json_root == NULL) {
    return SC_SERIALIZER_JSON_CREATE;
  }
  ret = ta_hash243_queue_to_json_array(res->addresses, json_root, "address");
  if (ret) {
    return ret;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return SC_SERIALIZER_JSON_PARSE;
  }
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_get_tips_res_serialize(char** obj,
                                   const ta_get_tips_res_t* const res) {
  cJSON* json_root = cJSON_CreateObject();
  status_t ret = SC_OK;
  if (json_root == NULL) {
    return SC_SERIALIZER_JSON_CREATE;
  }
  ret = ta_hash243_stack_to_json_array(res->tips, json_root, "tips");
  if (ret) {
    return ret;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    return SC_SERIALIZER_JSON_PARSE;
  }
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_send_transfer_req_deserialize(const char* const obj,
                                          ta_send_transfer_req_t* req) {
  if (obj == NULL) {
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;
  flex_trit_t tag_trits[NUM_TRITS_TAG], address_trits[NUM_TRITS_HASH];
  int msg_len = 0, tag_len = 0;
  bool raw_message = true;
  status_t ret = SC_OK;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "value");
  if ((json_result != NULL) && cJSON_IsNumber(json_result)) {
    req->value = json_result->valueint;
  } else {
    // 'value' does not exist or invalid, set to 0
    req->value = 0;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "tag");
  if (json_result != NULL) {
    tag_len = strnlen(json_result->valuestring, NUM_TRYTES_TAG);

    // If 'tag' is less than 27 trytes (NUM_TRYTES_TAG), expands it
    if (tag_len < NUM_TRYTES_TAG) {
      char new_tag[NUM_TRYTES_TAG + 1];
      // Fill in '9' to get valid tag (27 trytes)
      fill_tag(new_tag, json_result->valuestring, tag_len);
      new_tag[NUM_TRYTES_TAG] = '\0';
      flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)new_tag,
                             NUM_TRYTES_TAG, NUM_TRYTES_TAG);
    } else {
      // Valid tag from request, use it directly
      flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG,
                             (const tryte_t*)json_result->valuestring,
                             NUM_TRYTES_TAG, NUM_TRYTES_TAG);
    }
  } else {
    // 'tag' does not exists, set to DEFAULT_TAG
    flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG,
                           (const tryte_t*)DEFAULT_TAG, NUM_TRYTES_TAG,
                           NUM_TRYTES_TAG);
  }
  ret = hash81_queue_push(&req->tag, tag_trits);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message_format");
  if (json_result != NULL) {
    strncmp("trytes", json_result->valuestring, 6);
    raw_message = false;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message");
  if (json_result != NULL) {
    if (raw_message) {
      msg_len = strlen(json_result->valuestring) * 2;
      req->msg_len = msg_len * 3;
      tryte_t trytes_buffer[msg_len];

      ascii_to_trytes(json_result->valuestring, trytes_buffer);
      flex_trits_from_trytes(req->message, req->msg_len, trytes_buffer, msg_len,
                             msg_len);
    } else {
      msg_len = strlen(json_result->valuestring);
      req->msg_len = msg_len * 3;
      flex_trits_from_trytes(req->message, req->msg_len,
                             (const tryte_t*)json_result->valuestring, msg_len,
                             msg_len);
    }
  } else {
    // 'message' does not exists, set to DEFAULT_MSG
    req->msg_len = DEFAULT_MSG_LEN * 3;
    flex_trits_from_trytes(req->message, req->msg_len,
                           (const tryte_t*)DEFAULT_MSG, DEFAULT_MSG_LEN,
                           DEFAULT_MSG_LEN);
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "address");
  if (json_result != NULL && (strnlen(json_result->valuestring, 81) == 81)) {
    flex_trits_from_trytes(address_trits, NUM_TRITS_HASH,
                           (const tryte_t*)json_result->valuestring,
                           NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  } else {
    // 'address' does not exists, set to DEFAULT_ADDRESS
    flex_trits_from_trytes(address_trits, NUM_TRITS_HASH,
                           (const tryte_t*)DEFAULT_ADDRESS, NUM_TRYTES_HASH,
                           NUM_TRYTES_HASH);
  }
  ret = hash243_queue_push(&req->address, address_trits);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    goto done;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_get_transaction_object_res_serialize(
    char** obj, const ta_get_transaction_object_res_t* const res) {
  status_t ret = SC_OK;
  cJSON* json_root = NULL;

  ret = iota_transaction_to_json_object(res->txn, &json_root);
  if (ret) {
    goto done;
  }
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transactions_res_serialize(
    char** obj, const ta_find_transactions_res_t* const res) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    goto done;
  }

  ret = ta_hash243_queue_to_json_array(res->hashes, json_root, "hashes");
  if (ret) {
    goto done;
  }
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transactions_obj_res_serialize(
    char** obj, const ta_find_transactions_obj_res_t* const res) {
  status_t ret = SC_OK;
  iota_transaction_t* txn = NULL;
  cJSON* array_obj = NULL;
  cJSON* txn_obj = NULL;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    goto done;
  }

  // Create json array
  array_obj = cJSON_CreateArray();
  if (array_obj == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    goto done;
  }
  cJSON_AddItemToObject(json_root, "transactions", array_obj);

  // Serialize iota_transaction_t
  for (txn = (iota_transaction_t*)utarray_front(res->txn_obj); txn != NULL;
       txn = (iota_transaction_t*)utarray_next(res->txn_obj, txn)) {
    txn_obj = NULL;
    ret = iota_transaction_to_json_object(txn, &txn_obj);
    if (ret) {
      goto done;
    }
    cJSON_AddItemToArray(array_obj, txn_obj);
  }
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t receive_mam_message_serialize(char** obj, const char** res) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    goto done;
  }

  cJSON_AddStringToObject(json_root, "message", *res);

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t send_mam_res_serialize(char** obj, const send_mam_res_t* const res) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    goto done;
  }

  cJSON_AddStringToObject(json_root, "channel", res->channel_id);

  cJSON_AddStringToObject(json_root, "bundle_hash", res->bundle_hash);

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t send_mam_res_deserialize(const char* const obj,
                                  send_mam_res_t* const res) {
  if (obj == NULL) {
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  status_t ret = SC_OK;
  tryte_t addr[NUM_TRYTES_ADDRESS + 1];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

  if (ta_json_get_string(json_obj, "channel", (char*)addr) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    goto done;
  }
  send_mam_res_set_channel_id(res, addr);

  if (ta_json_get_string(json_obj, "bundle_hash", (char*)addr) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    goto done;
  }
  send_mam_res_set_bundle_hash(res, addr);

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t send_mam_req_deserialize(const char* const obj, send_mam_req_t* req) {
  if (obj == NULL) {
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;
  status_t ret = SC_OK;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message");
  if (json_result != NULL) {
    size_t payload_size = strlen(json_result->valuestring) * 2;
    tryte_t* payload_trytes = (tryte_t*)malloc(payload_size * sizeof(tryte_t));
    ascii_to_trytes(json_result->valuestring, payload_trytes);

    req->payload_trytes = payload_trytes;
    req->payload_trytes_size = payload_size;
  } else {
    ret = SC_SERIALIZER_NULL;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}
