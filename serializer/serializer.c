/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "serializer.h"
#include "utils/logger_helper.h"

#define SERI_LOGGER "serializer"

static logger_id_t seri_logger_id;

void serializer_logger_init() { seri_logger_id = logger_helper_enable(SERI_LOGGER, LOGGER_DEBUG, true); }

int serializer_logger_release() {
  logger_helper_release(seri_logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(seri_logger_id, "[%s:%d] Destroying logger failed %s.\n", __func__, __LINE__, SERI_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

static status_t ta_hash243_stack_to_json_array(hash243_stack_t stack, cJSON* json_root) {
  size_t array_count = 0;
  hash243_stack_entry_t* s_iter = NULL;
  tryte_t trytes_out[NUM_TRYTES_HASH + 1];
  size_t trits_count = 0;

  array_count = hash243_stack_count(stack);
  if (array_count > 0) {
    LL_FOREACH(stack, s_iter) {
      trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_HASH, s_iter->hash, NUM_TRITS_HASH, NUM_TRITS_HASH);
      trytes_out[NUM_TRYTES_HASH] = '\0';
      if (trits_count != 0) {
        cJSON_AddItemToArray(json_root, cJSON_CreateString((const char*)trytes_out));
      } else {
        log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_INVALID_FLEX_TRITS");
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_NOT_FOUND");
    return SC_CCLIENT_NOT_FOUND;
  }
  return SC_OK;
}

static status_t ta_json_array_to_hash243_queue(cJSON const* const obj, char const* const obj_name,
                                               hash243_queue_t* queue) {
  status_t ret_code = SC_OK;
  flex_trit_t hash[FLEX_TRIT_SIZE_243] = {};
  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (!json_item) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  if (cJSON_IsArray(json_item)) {
    cJSON* current_obj = NULL;
    cJSON_ArrayForEach(current_obj, json_item) {
      if (current_obj->valuestring != NULL) {
        flex_trits_from_trytes(hash, NUM_TRITS_HASH, (tryte_t const*)current_obj->valuestring, NUM_TRYTES_HASH,
                               NUM_TRYTES_HASH);
        if (hash243_queue_push(queue, hash) != RC_OK) {
          log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
          return SC_SERIALIZER_JSON_PARSE;
        }
      }
    }
  } else {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    return SC_SERIALIZER_JSON_PARSE;
  }
  return ret_code;
}

static status_t ta_hash243_queue_to_json_array(hash243_queue_t queue, cJSON* const json_root) {
  size_t array_count;
  hash243_queue_entry_t* q_iter = NULL;

  array_count = hash243_queue_count(queue);
  if (array_count > 0) {
    CDL_FOREACH(queue, q_iter) {
      tryte_t trytes_out[NUM_TRYTES_HASH + 1];
      size_t trits_count =
          flex_trits_to_trytes(trytes_out, NUM_TRYTES_HASH, q_iter->hash, NUM_TRITS_HASH, NUM_TRITS_HASH);
      trytes_out[NUM_TRYTES_HASH] = '\0';
      if (trits_count != 0) {
        cJSON_AddItemToArray(json_root, cJSON_CreateString((const char*)trytes_out));
      } else {
        log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_INVALID_FLEX_TRITS");
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_NOT_FOUND");
    return SC_CCLIENT_NOT_FOUND;
  }
  return SC_OK;
}

static status_t ta_json_array_to_hash8019_array(cJSON const* const obj, char const* const obj_name,
                                                hash8019_array_p array) {
  status_t ret = SC_OK;
  flex_trit_t hash[FLEX_TRIT_SIZE_8019] = {};
  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (!cJSON_IsArray(json_item)) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    return SC_CCLIENT_JSON_PARSE;
  }

  cJSON* current_obj = NULL;
  cJSON_ArrayForEach(current_obj, json_item) {
    if (current_obj->valuestring != NULL) {
      if (strlen(current_obj->valuestring) != NUM_TRYTES_SERIALIZED_TRANSACTION) {
        log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_INVALID_REQ");
        return SC_SERIALIZER_INVALID_REQ;
      }
      flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t const*)current_obj->valuestring,
                             NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
      hash_array_push(array, hash);
    }
  }
  return ret;
}

status_t ta_hash8019_array_to_json_array(hash8019_array_p array, cJSON* const json_root, char const* const obj_name) {
  size_t array_count = 0;
  cJSON* array_obj = NULL;
  tryte_t trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION + 1] = {};
  size_t trits_count = 0;
  flex_trit_t* elt = NULL;

  array_count = hash_array_len(array);
  if (array_count > 0) {
    array_obj = cJSON_CreateArray();
    if (array_obj == NULL) {
      log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
      return SC_SERIALIZER_JSON_CREATE;
    }
    cJSON_AddItemToObject(json_root, obj_name, array_obj);

    HASH_ARRAY_FOREACH(array, elt) {
      trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_SERIALIZED_TRANSACTION, elt,
                                         NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
      trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';
      if (trits_count == 0) {
        log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FLEX_TRITS");
        return SC_CCLIENT_FLEX_TRITS;
      }
      cJSON_AddItemToArray(array_obj, cJSON_CreateString((char const*)trytes_out));
    }
  }
  return SC_OK;
}

static status_t ta_json_get_string(cJSON const* const json_obj, char const* const obj_name, char* const text) {
  status_t ret = SC_OK;
  if (json_obj == NULL || obj_name == NULL || text == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  cJSON* json_value = cJSON_GetObjectItemCaseSensitive(json_obj, obj_name);
  if (json_value == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_KEY");
    return SC_CCLIENT_JSON_KEY;
  }

  if (cJSON_IsString(json_value) && (json_value->valuestring != NULL)) {
    strcpy(text, json_value->valuestring);
  } else {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_PARSE");
    return SC_CCLIENT_JSON_PARSE;
  }

  return ret;
}

status_t iota_transaction_to_json_object(iota_transaction_t const* const txn, cJSON** txn_json) {
  if (txn == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_NOT_FOUND");
    return SC_CCLIENT_NOT_FOUND;
  }
  char msg_trytes[NUM_TRYTES_SIGNATURE + 1], hash_trytes[NUM_TRYTES_HASH + 1], tag_trytes[NUM_TRYTES_TAG + 1];

  if (txn_json == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    return SC_SERIALIZER_JSON_CREATE;
  }

  // transaction hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, transaction_hash(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "hash", hash_trytes);

  // message
  flex_trits_to_trytes((tryte_t*)msg_trytes, NUM_TRYTES_SIGNATURE, transaction_message(txn), NUM_TRITS_SIGNATURE,
                       NUM_TRITS_SIGNATURE);
  msg_trytes[NUM_TRYTES_SIGNATURE] = '\0';
  cJSON_AddStringToObject(*txn_json, "signature_and_message_fragment", msg_trytes);

  // address
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, transaction_address(txn), NUM_TRITS_HASH,
                       NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "address", hash_trytes);
  // value
  cJSON_AddNumberToObject(*txn_json, "value", transaction_value(txn));
  // obsolete tag
  flex_trits_to_trytes((tryte_t*)tag_trytes, NUM_TRYTES_TAG, transaction_obsolete_tag(txn), NUM_TRITS_TAG,
                       NUM_TRITS_TAG);
  tag_trytes[NUM_TRYTES_TAG] = '\0';
  cJSON_AddStringToObject(*txn_json, "obsolete_tag", tag_trytes);

  // timestamp
  cJSON_AddNumberToObject(*txn_json, "timestamp", transaction_timestamp(txn));

  // current index
  cJSON_AddNumberToObject(*txn_json, "current_index", transaction_current_index(txn));

  // last index
  cJSON_AddNumberToObject(*txn_json, "last_index", transaction_last_index(txn));

  // bundle hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, transaction_bundle(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "bundle_hash", hash_trytes);

  // trunk transaction hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, transaction_trunk(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "trunk_transaction_hash", hash_trytes);

  // branch transaction hash
  flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, transaction_branch(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(*txn_json, "branch_transaction_hash", hash_trytes);

  // tag
  flex_trits_to_trytes((tryte_t*)tag_trytes, NUM_TRYTES_TAG, transaction_tag(txn), NUM_TRITS_TAG, NUM_TRITS_TAG);
  tag_trytes[NUM_TRYTES_TAG] = '\0';
  cJSON_AddStringToObject(*txn_json, "tag", tag_trytes);

  // attachment timestamp
  cJSON_AddNumberToObject(*txn_json, "attachment_timestamp", transaction_attachment_timestamp(txn));

  // attachment lower timestamp
  cJSON_AddNumberToObject(*txn_json, "attachment_timestamp_lower_bound", transaction_attachment_timestamp_lower(txn));

  // attachment upper timestamp
  cJSON_AddNumberToObject(*txn_json, "attachment_timestamp_upper_bound", transaction_attachment_timestamp_upper(txn));

  // nonce
  flex_trits_to_trytes((tryte_t*)tag_trytes, NUM_TRYTES_NONCE, transaction_nonce(txn), NUM_TRITS_NONCE,
                       NUM_TRITS_NONCE);
  tag_trytes[NUM_TRYTES_TAG] = '\0';
  cJSON_AddStringToObject(*txn_json, "nonce", tag_trytes);

  return SC_OK;
}

static status_t transaction_array_to_json_array(cJSON* json_root, const transaction_array_t* const txn_array) {
  status_t ret = SC_OK;
  iota_transaction_t* txn = NULL;

  TX_OBJS_FOREACH(txn_array, txn) {
    cJSON* txn_obj = cJSON_CreateObject();

    ret = iota_transaction_to_json_object(txn, &txn_obj);
    if (ret != SC_OK) {
      return ret;
    }
    cJSON_AddItemToArray(json_root, txn_obj);
  }
  return ret;
}

status_t ta_generate_address_res_serialize(const ta_generate_address_res_t* const res, char** obj) {
  cJSON* json_root = cJSON_CreateArray();
  status_t ret = SC_OK;
  if (json_root == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    return SC_SERIALIZER_JSON_CREATE;
  }
  ret = ta_hash243_queue_to_json_array(res->addresses, json_root);
  if (ret) {
    return ret;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    return SC_SERIALIZER_JSON_PARSE;
  }
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_get_tips_res_serialize(const get_tips_res_t* const res, char** obj) {
  status_t ret = SC_OK;

  cJSON* json_root = cJSON_CreateArray();
  if (json_root == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    return RC_CCLIENT_JSON_CREATE;
  }

  ret = ta_hash243_stack_to_json_array(res->hashes, json_root);
  if (ret) {
    goto err;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    return SC_SERIALIZER_JSON_PARSE;
  }

err:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_send_transfer_req_deserialize(const char* const obj, ta_send_transfer_req_t* req) {
  if (obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
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
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
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
  if (json_result != NULL && json_result->valuestring != NULL) {
    tag_len = strnlen(json_result->valuestring, NUM_TRYTES_TAG);

    for (int i = 0; i < tag_len; i++) {
      if (json_result->valuestring[i] & (unsigned)128) {
        ret = SC_SERIALIZER_JSON_PARSE_ASCII;
        log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE_ASCII");
        goto done;
      }
    }

    // If 'tag' is less than 27 trytes (NUM_TRYTES_TAG), expands it
    if (tag_len < NUM_TRYTES_TAG) {
      char new_tag[NUM_TRYTES_TAG + 1];
      // Fill in '9' to get valid tag (27 trytes)
      fill_nines(new_tag, json_result->valuestring, NUM_TRYTES_TAG);
      new_tag[NUM_TRYTES_TAG] = '\0';
      flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)new_tag, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
    } else {
      // Valid tag from request, use it directly
      flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)json_result->valuestring, NUM_TRYTES_TAG,
                             NUM_TRYTES_TAG);
    }
  } else {
    // 'tag' does not exists, set to DEFAULT_TAG
    flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)DEFAULT_TAG, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  }
  ret = hash81_queue_push(&req->tag, tag_trits);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_HASH");
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message_format");
  if (json_result != NULL) {
    if (!strncmp("trytes", json_result->valuestring, 6)) {
      raw_message = false;
    }
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message");
  if (json_result != NULL && json_result->valuestring != NULL) {
    msg_len = strlen(json_result->valuestring);

    // In case the payload is unicode, char more than 128 will result to an
    // error status_t code
    for (int i = 0; i < msg_len; i++) {
      if (json_result->valuestring[i] & 0x80) {
        ret = SC_SERIALIZER_JSON_PARSE_ASCII;
        log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE_ASCII");
        goto done;
      }
    }

    if (raw_message) {
      msg_len = msg_len * 2;
      req->msg_len = msg_len * 3;
      tryte_t trytes_buffer[msg_len];

      ascii_to_trytes(json_result->valuestring, trytes_buffer);
      flex_trits_from_trytes(req->message, req->msg_len, trytes_buffer, msg_len, msg_len);
    } else {
      req->msg_len = msg_len * 3;
      flex_trits_from_trytes(req->message, req->msg_len, (const tryte_t*)json_result->valuestring, msg_len, msg_len);
    }
  } else {
    // 'message' does not exists, set to DEFAULT_MSG
    req->msg_len = DEFAULT_MSG_LEN * 3;
    flex_trits_from_trytes(req->message, req->msg_len, (const tryte_t*)DEFAULT_MSG, DEFAULT_MSG_LEN, DEFAULT_MSG_LEN);
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "address");
  if (json_result != NULL && json_result->valuestring != NULL && (strnlen(json_result->valuestring, 81) == 81)) {
    flex_trits_from_trytes(address_trits, NUM_TRITS_HASH, (const tryte_t*)json_result->valuestring, NUM_TRYTES_HASH,
                           NUM_TRYTES_HASH);
  } else {
    // 'address' does not exists, set to DEFAULT_ADDRESS
    flex_trits_from_trytes(address_trits, NUM_TRITS_HASH, (const tryte_t*)DEFAULT_ADDRESS, NUM_TRYTES_HASH,
                           NUM_TRYTES_HASH);
  }
  ret = hash243_queue_push(&req->address, address_trits);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_HASH");
    goto done;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_send_trytes_req_deserialize(const char* const obj, hash8019_array_p out_trytes) {
  if (obj == NULL || out_trytes == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  ret = ta_json_array_to_hash8019_array(json_obj, "trytes", out_trytes);
  if (ret != SC_OK) {
    goto done;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_send_trytes_res_serialize(const hash8019_array_p trytes, char** obj) {
  if (trytes == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();

  ret = ta_hash8019_array_to_json_array(trytes, json_root, "trytes");
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    ret = SC_SERIALIZER_JSON_PARSE;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transaction_objects_req_deserialize(const char* const obj,
                                                     ta_find_transaction_objects_req_t* const req) {
  status_t ret = SC_OK;
  if (obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  cJSON* json_obj = cJSON_Parse(obj);
  if (json_obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    return SC_SERIALIZER_JSON_PARSE;
  }

  ret = ta_json_array_to_hash243_queue(json_obj, "hashes", &req->hashes);
  if (ret != SC_OK) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
  }

  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_find_transaction_object_single_res_serialize(transaction_array_t* res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_CREATE");
    return SC_CCLIENT_JSON_CREATE;
  }

  ret = iota_transaction_to_json_object(transaction_array_at(res, 0), &json_root);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transaction_objects_res_serialize(const transaction_array_t* const res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateArray();
  if (json_root == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_JSON_CREATE");
    return SC_CCLIENT_JSON_CREATE;
  }

  ret = transaction_array_to_json_array(json_root, res);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transactions_by_tag_res_serialize(const ta_find_transactions_by_tag_res_t* const res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateArray();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  ret = ta_hash243_queue_to_json_array(res->hashes, json_root);
  if (ret) {
    goto done;
  }
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_send_transfer_res_serialize(transaction_array_t* res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  ret = iota_transaction_to_json_object(transaction_array_at(res, 0), &json_root);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t receive_mam_message_res_serialize(char* const message, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  cJSON_AddStringToObject(json_root, "message", message);

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t send_mam_res_serialize(const ta_send_mam_res_t* const res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  cJSON_AddStringToObject(json_root, "channel", res->channel_id);

  cJSON_AddStringToObject(json_root, "bundle_hash", res->bundle_hash);

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t send_mam_res_deserialize(const char* const obj, ta_send_mam_res_t* const res) {
  if (obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  status_t ret = SC_OK;
  tryte_t addr[NUM_TRYTES_ADDRESS + 1];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  if (ta_json_get_string(json_obj, "channel", (char*)addr) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_channel_id(res, addr);

  if (ta_json_get_string(json_obj, "bundle_hash", (char*)addr) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_bundle_hash(res, addr);

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t send_mam_req_deserialize(const char* const obj, ta_send_mam_req_t* req) {
  if (obj == NULL) {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;
  status_t ret = SC_OK;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "prng");
  if (json_result != NULL) {
    size_t prng_size = strlen(json_result->valuestring);

    if (prng_size != NUM_TRYTES_HASH) {
      ret = SC_SERIALIZER_INVALID_REQ;
      log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_INVALID_REQ");
      goto done;
    }
    snprintf(req->prng, prng_size + 1, "%s", json_result->valuestring);
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "message");
  if (json_result != NULL) {
    size_t payload_size = strlen(json_result->valuestring);
    char* payload = (char*)malloc((payload_size + 1) * sizeof(char));

    // In case the payload is unicode, char more than 128 will result to an
    // error status_t code
    for (int i = 0; i < payload_size; i++) {
      if (json_result->valuestring[i] & 0x80) {
        ret = SC_SERIALIZER_JSON_PARSE_ASCII;
        log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_JSON_PARSE_ASCII");
        goto done;
      }
    }

    snprintf(payload, payload_size + 1, "%s", json_result->valuestring);
    req->payload = payload;
  } else {
    log_error(seri_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_SERIALIZER_NULL");
    ret = SC_SERIALIZER_NULL;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "order");
  if ((json_result != NULL) && cJSON_IsNumber(json_result)) {
    req->channel_ord = json_result->valueint;
  } else {
    // 'value' does not exist or invalid, set to 0
    req->channel_ord = 0;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}
