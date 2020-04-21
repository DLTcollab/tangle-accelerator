/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "serializer.h"
#ifdef MQTT_ENABLE
#include "connectivity/mqtt/mqtt_common.h"
#endif
#include "common/logger.h"
#include "time.h"
#define SERI_LOGGER "serializer"

static logger_id_t logger_id;

void serializer_logger_init() { logger_id = logger_helper_enable(SERI_LOGGER, LOGGER_DEBUG, true); }

int serializer_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", SERI_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t ta_get_info_serialize(char** obj, ta_config_t* const ta_config, iota_config_t* const tangle,
                               ta_cache_t* const cache) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    return SC_SERIALIZER_JSON_CREATE;
  }

  cJSON_AddStringToObject(json_root, "name", "tangle-accelerator");
  cJSON_AddStringToObject(json_root, "host", ta_config->host);
  cJSON_AddStringToObject(json_root, "version", ta_config->version);
  cJSON_AddNumberToObject(json_root, "port", ta_config->port);
  cJSON_AddNumberToObject(json_root, "thread", ta_config->thread_count);
  cJSON_AddStringToObject(json_root, "redis_host", cache->host);
  cJSON_AddNumberToObject(json_root, "redis_port", cache->port);
  cJSON_AddNumberToObject(json_root, "milestone_depth", tangle->milestone_depth);
  cJSON_AddNumberToObject(json_root, "mwm", tangle->mwm);
  cJSON_AddBoolToObject(json_root, "quiet", quiet_mode);

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    ret = SC_SERIALIZER_JSON_PARSE;
  }

  cJSON_Delete(json_root);
  return ret;
}
#ifdef DB_ENABLE
status_t db_identity_serialize(char** obj, db_identity_t* id_obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    return SC_SERIALIZER_JSON_CREATE;
  }
  // uuid
  char uuid_str[DB_UUID_STRING_LENGTH];
  db_get_identity_uuid_string(id_obj, uuid_str);
  cJSON_AddStringToObject(json_root, "id", uuid_str);

  // transaction hash
  char hash_trytes[NUM_TRYTES_HASH + 1];
  memcpy(hash_trytes, db_ret_identity_hash(id_obj), NUM_TRYTES_HASH);
  hash_trytes[NUM_TRYTES_HASH] = '\0';
  cJSON_AddStringToObject(json_root, "hash", hash_trytes);

  // status
  if (db_ret_identity_status(id_obj) == CONFIRMED_TXN) {
    cJSON_AddStringToObject(json_root, "status", "CONFIRMED");
  } else {
    cJSON_AddStringToObject(json_root, "status", "PENDING");
  }

  // timestamp
  time_t raw_time = db_ret_identity_timestamp(id_obj);
  struct tm* ts = localtime(&raw_time);
  char buf[40];
  strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
  cJSON_AddStringToObject(json_root, "timestamp", buf);
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    ret = SC_SERIALIZER_JSON_PARSE;
  }
  cJSON_Delete(json_root);
  return ret;
}
#endif

status_t string_utarray_to_json_array(UT_array const* const ut, cJSON* const json_root, char const* const obj_name) {
  cJSON* array_obj = cJSON_CreateArray();
  char** p = NULL;

  if (!ut) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  if (array_obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    return SC_SERIALIZER_JSON_CREATE;
  }

  cJSON_AddItemToObject(json_root, obj_name, array_obj);

  while ((p = (char**)utarray_next(ut, p))) {
    cJSON_AddItemToArray(array_obj, cJSON_CreateString(*p));
  }
  return SC_OK;
}

status_t json_string_array_to_string_utarray(cJSON const* const obj, char const* const obj_name, UT_array* const ut) {
  char* str = NULL;

  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (cJSON_IsArray(json_item)) {
    cJSON* array_elt = NULL;
    cJSON_ArrayForEach(array_elt, json_item) {
      str = cJSON_GetStringValue(array_elt);
      if (!str) {
        ta_log_error("%s\n", "encountered non-string array member");
        return SC_SERIALIZER_JSON_PARSE;
      }
      utarray_push_back(ut, &str);
    }
  } else {
    ta_log_error("%s\n", "not an array");
    return SC_SERIALIZER_JSON_PARSE;
  }

  return SC_OK;
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
        ta_log_error("%s\n", "SC_CCLIENT_INVALID_FLEX_TRITS");
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    ta_log_error("%s\n", "SC_CCLIENT_NOT_FOUND");
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
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  if (cJSON_IsArray(json_item)) {
    cJSON* current_obj = NULL;
    cJSON_ArrayForEach(current_obj, json_item) {
      if (current_obj->valuestring != NULL) {
        flex_trits_from_trytes(hash, NUM_TRITS_HASH, (tryte_t const*)current_obj->valuestring, NUM_TRYTES_HASH,
                               NUM_TRYTES_HASH);
        if (hash243_queue_push(queue, hash) != RC_OK) {
          ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
          return SC_SERIALIZER_JSON_PARSE;
        }
      }
    }
  } else {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
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
        ta_log_error("%s\n", "SC_CCLIENT_INVALID_FLEX_TRITS");
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    ta_log_error("%s\n", "SC_CCLIENT_NOT_FOUND");
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
    ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
    return SC_CCLIENT_JSON_PARSE;
  }

  cJSON* current_obj = NULL;
  cJSON_ArrayForEach(current_obj, json_item) {
    if (current_obj->valuestring != NULL) {
      if (strlen(current_obj->valuestring) != NUM_TRYTES_SERIALIZED_TRANSACTION) {
        ta_log_error("%s\n", "SC_SERIALIZER_INVALID_REQ");
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
      ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
      return SC_SERIALIZER_JSON_CREATE;
    }
    cJSON_AddItemToObject(json_root, obj_name, array_obj);

    HASH_ARRAY_FOREACH(array, elt) {
      trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_SERIALIZED_TRANSACTION, elt,
                                         NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
      trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';
      if (trits_count == 0) {
        ta_log_error("%s\n", "SC_CCLIENT_FLEX_TRITS");
        return SC_CCLIENT_FLEX_TRITS;
      }
      cJSON_AddItemToArray(array_obj, cJSON_CreateString((char const*)trytes_out));
    }
  }
  return SC_OK;
}

static status_t ta_json_get_string(cJSON const* const json_obj, char const* const obj_name, char* const text,
                                   const size_t size) {
  status_t ret = SC_OK;
  if (json_obj == NULL || obj_name == NULL || text == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  cJSON* json_value = cJSON_GetObjectItemCaseSensitive(json_obj, obj_name);
  if (json_value == NULL) {
    ta_log_error("%s\n", "SC_CCLIENT_JSON_KEY");
    return SC_CCLIENT_JSON_KEY;
  }

  if (cJSON_IsString(json_value) && (json_value->valuestring != NULL)) {
    strncpy(text, json_value->valuestring, size);
  } else {
    ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
    return SC_CCLIENT_JSON_PARSE;
  }

  return ret;
}

status_t iota_transaction_to_json_object(iota_transaction_t const* const txn, cJSON** txn_json) {
  if (txn == NULL) {
    ta_log_error("%s\n", "SC_CCLIENT_NOT_FOUND");
    return SC_CCLIENT_NOT_FOUND;
  }
  char msg_trytes[NUM_TRYTES_SIGNATURE + 1], hash_trytes[NUM_TRYTES_HASH + 1], tag_trytes[NUM_TRYTES_TAG + 1];

  if (txn_json == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
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
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    return SC_SERIALIZER_JSON_CREATE;
  }
  ret = ta_hash243_queue_to_json_array(res->addresses, json_root);
  if (ret) {
    return ret;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    return SC_SERIALIZER_JSON_PARSE;
  }
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_get_tips_res_serialize(const get_tips_res_t* const res, char** obj) {
  status_t ret = SC_OK;

  cJSON* json_root = cJSON_CreateArray();
  if (json_root == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    return RC_CCLIENT_JSON_CREATE;
  }

  ret = ta_hash243_stack_to_json_array(res->hashes, json_root);
  if (ret) {
    goto err;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    return SC_SERIALIZER_JSON_PARSE;
  }

err:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_send_transfer_req_deserialize(const char* const obj, ta_send_transfer_req_t* req) {
  if (obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
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
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
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
        ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE_ASCII");
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
    ta_log_error("%s\n", "SC_CCLIENT_HASH");
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

    // In case the payload is unicode, the character whose ASCII code is beyond 128 result to an
    // error status_t code
    for (int i = 0; i < msg_len; i++) {
      if (json_result->valuestring[i] & 0x80) {
        ret = SC_SERIALIZER_JSON_PARSE_ASCII;
        ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE_ASCII");
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

    if (req->msg_len > NUM_TRYTES_MESSAGE) {
      ret = SC_SERIALIZER_MESSAGE_OVERRUN;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
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
    ta_log_error("%s\n", "SC_CCLIENT_HASH");
    goto done;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_send_trytes_req_deserialize(const char* const obj, hash8019_array_p out_trytes) {
  if (obj == NULL || out_trytes == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
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
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
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
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
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
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }

  cJSON* json_obj = cJSON_Parse(obj);
  if (json_obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    return SC_SERIALIZER_JSON_PARSE;
  }

  ret = ta_json_array_to_hash243_queue(json_obj, "hashes", &req->hashes);
  if (ret != SC_OK) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
  }

  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_find_transaction_object_single_res_serialize(transaction_array_t* res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ta_log_error("%s\n", "SC_CCLIENT_JSON_CREATE");
    return SC_CCLIENT_JSON_CREATE;
  }

  ret = iota_transaction_to_json_object(transaction_array_at(res, 0), &json_root);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transaction_objects_res_serialize(const transaction_array_t* const res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateArray();
  if (json_root == NULL) {
    ta_log_error("%s\n", "SC_CCLIENT_JSON_CREATE");
    return SC_CCLIENT_JSON_CREATE;
  }

  ret = transaction_array_to_json_array(json_root, res);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transactions_by_tag_res_serialize(const ta_find_transactions_by_tag_res_t* const res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateArray();

  if (res == NULL || res->hashes == NULL) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }

  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  ret = ta_hash243_queue_to_json_array(res->hashes, json_root);
  if (ret) {
    goto done;
  }
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    ret = SC_SERIALIZER_JSON_PARSE;
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

static status_t send_mam_message_mam_v1_req_deserialize(cJSON const* const json_obj, ta_send_mam_req_t* const req) {
  cJSON *json_key = NULL, *json_value = NULL;
  status_t ret = SC_OK;
  send_mam_data_mam_v1_t* data = (send_mam_data_mam_v1_t*)req->data;
  send_mam_key_mam_v1_t* key = (send_mam_key_mam_v1_t*)req->key;

  ret = ta_json_get_string(json_obj, "x-api-key", req->service_token, SERVICE_TOKEN_LEN);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  if (cJSON_HasObjectItem(json_obj, "key")) {
    json_key = cJSON_GetObjectItem(json_obj, "key");
    if (json_key == NULL) {
      ret = SC_SERIALIZER_JSON_PARSE;
      ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
      goto done;
    }

    if (cJSON_HasObjectItem(json_key, "psk")) {
      ret = json_string_array_to_string_utarray(json_key, "psk", key->psk_array);
      if (ret != SC_OK) {
        ret = SC_SERIALIZER_JSON_PARSE;
        ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
        goto done;
      }
    }

    if (cJSON_HasObjectItem(json_key, "ntru")) {
      ret = json_string_array_to_string_utarray(json_key, "ntru", key->ntru_array);
      if (ret != SC_OK) {
        ret = SC_SERIALIZER_JSON_PARSE;
        ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
        goto done;
      }
    }
  }

  json_key = cJSON_GetObjectItem(json_obj, "data");
  if (json_key == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "seed");
  if (json_value != NULL) {
    size_t seed_size = strnlen(json_value->valuestring, NUM_TRYTES_ADDRESS);

    if (seed_size != NUM_TRYTES_HASH) {
      ret = SC_SERIALIZER_INVALID_REQ;
      ta_log_error("%s\n", "SC_SERIALIZER_INVALID_REQ");
      goto done;
    }
    data->seed = (tryte_t*)malloc(sizeof(tryte_t) * (NUM_TRYTES_ADDRESS + 1));
    snprintf((char*)data->seed, seed_size + 1, "%s", json_value->valuestring);
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "message");
  if (json_value != NULL) {
    size_t payload_size = strlen(json_value->valuestring);
    char* payload = (char*)malloc((payload_size + 1) * sizeof(char));

    // In case the payload is unicode, the character whose ASCII code is beyond 128 result to an
    // error status_t code
    for (size_t i = 0; i < payload_size; i++) {
      if (json_value->valuestring[i] & 0x80) {
        ret = SC_SERIALIZER_JSON_PARSE_ASCII;
        ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE_ASCII");
        goto done;
      }
    }

    snprintf(payload, payload_size + 1, "%s", json_value->valuestring);
    data->message = payload;
  } else {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    ret = SC_SERIALIZER_NULL;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "ch_mss_depth");
  if ((json_value != NULL) && cJSON_IsNumber(json_value)) {
    data->ch_mss_depth = json_value->valueint;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "ep_mss_depth");
  if ((json_value != NULL) && cJSON_IsNumber(json_value)) {
    data->ep_mss_depth = json_value->valueint;
  }

done:
  return ret;
}

status_t ta_send_transfer_res_serialize(ta_send_transfer_res_t* res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  if (res->uuid) {
    cJSON_AddStringToObject(json_root, "uuid", res->uuid);
  } else {
    ret = iota_transaction_to_json_object(transaction_array_at(res->txn_array, 0), &json_root);
    if (ret != SC_OK) {
      goto done;
    }
#ifdef DB_ENABLE
    cJSON_AddStringToObject(json_root, "id", res->uuid_string);
#endif
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t send_mam_req_deserialize(const char* const obj, ta_send_mam_req_t* const req) {
  if (obj == NULL || req == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  const int protocol_len = 20;
  char protocol_str[protocol_len];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  ret = ta_json_get_string(json_obj, "protocol", protocol_str, protocol_len);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }
  if (!strncmp(protocol_str, "MAM_V1", strlen("MAM_V1"))) {
    req->protocol = MAM_V1;
  }

  send_mam_req_v1_init(req);
  ret = send_mam_message_mam_v1_req_deserialize(json_obj, req);
  if (ret) {
    ta_log_error("%d\n", ta_error_to_string(ret));
    goto done;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

static status_t recv_mam_message_mam_v1_req_deserialize(cJSON const* const json_obj, ta_recv_mam_req_t* const req) {
  cJSON *json_key = NULL, *json_value = NULL;
  status_t ret = SC_OK;
  char *bundle_hash = NULL, *chid = NULL, *epid = NULL, *msg_id = NULL, *psk = NULL, *ntru = NULL;

  json_value = cJSON_GetObjectItemCaseSensitive(json_obj, "key");
  if (json_value == NULL) {
    ta_log_error("%s\n", "SC_CCLIENT_JSON_KEY");
    return SC_CCLIENT_JSON_KEY;
  }
  if (cJSON_IsString(json_value) && (json_value->valuestring != NULL)) {
    if (strlen(json_value->valuestring) == NUM_TRYTES_MAM_PSK_KEY_SIZE) {
      psk = (char*)malloc(sizeof(char) * (NUM_TRYTES_MAM_PSK_KEY_SIZE + 1));
      strncpy(psk, json_value->valuestring, sizeof(char) * (NUM_TRYTES_MAM_PSK_KEY_SIZE + 1));
    } else if (strlen(json_value->valuestring) == NUM_TRYTES_MAM_NTRU_PK_SIZE) {
      ntru = (char*)malloc(sizeof(char) * (NUM_TRYTES_MAM_NTRU_PK_SIZE + 1));
      strncpy(ntru, json_value->valuestring, sizeof(char) * (NUM_TRYTES_MAM_NTRU_PK_SIZE + 1));
    }

  } else {
    ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
    return SC_CCLIENT_JSON_PARSE;
  }

  ret = recv_mam_set_mam_v1_key(req, (tryte_t*)psk, (tryte_t*)ntru);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  json_key = cJSON_GetObjectItem(json_obj, "data_id");
  if (json_key == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  if (cJSON_HasObjectItem(json_key, "bundle_hash")) {
    json_value = cJSON_GetObjectItemCaseSensitive(json_key, "bundle_hash");
    if (json_value == NULL) {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_KEY");
      return SC_CCLIENT_JSON_KEY;
    }
    if (cJSON_IsString(json_value) && (json_value->valuestring != NULL) &&
        (strlen(json_value->valuestring) == NUM_TRYTES_HASH)) {
      bundle_hash = (char*)malloc(sizeof(char) * (NUM_TRYTES_HASH + 1));
      strncpy(bundle_hash, json_value->valuestring, sizeof(char) * (NUM_TRYTES_HASH + 1));
    } else {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
      return SC_CCLIENT_JSON_PARSE;
    }
    goto set_data_id;
  }

  if (cJSON_HasObjectItem(json_key, "chid")) {
    json_value = cJSON_GetObjectItemCaseSensitive(json_key, "chid");
    if (json_value == NULL) {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_KEY");
      return SC_CCLIENT_JSON_KEY;
    }
    if (cJSON_IsString(json_value) &&
        (json_value->valuestring != NULL && (strlen(json_value->valuestring) == NUM_TRYTES_HASH))) {
      chid = (char*)malloc(sizeof(char) * (NUM_TRYTES_HASH + 1));
      strncpy(chid, json_value->valuestring, sizeof(char) * (NUM_TRYTES_HASH + 1));
    } else {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
      return SC_CCLIENT_JSON_PARSE;
    }
  }

  if (cJSON_HasObjectItem(json_key, "epid")) {
    json_value = cJSON_GetObjectItemCaseSensitive(json_key, "epid");
    if (json_value == NULL) {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_KEY");
      return SC_CCLIENT_JSON_KEY;
    }
    if (cJSON_IsString(json_value) && (json_value->valuestring != NULL) &&
        (strlen(json_value->valuestring) == NUM_TRYTES_HASH)) {
      epid = (char*)malloc(sizeof(char) * (NUM_TRYTES_HASH + 1));
      strncpy(epid, json_value->valuestring, sizeof(char) * (NUM_TRYTES_HASH + 1));
    } else {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
      return SC_CCLIENT_JSON_PARSE;
    }
  }

  if (cJSON_HasObjectItem(json_key, "msg_id")) {
    json_value = cJSON_GetObjectItemCaseSensitive(json_key, "msg_id");
    if (json_value == NULL) {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_KEY");
      return SC_CCLIENT_JSON_KEY;
    }
    if (cJSON_IsString(json_value) && (json_value->valuestring != NULL) &&
        (strlen(json_value->valuestring) == NUM_TRYTES_MAM_MSG_ID)) {
      msg_id = (char*)malloc(sizeof(char) * (NUM_TRYTES_MAM_MSG_ID + 1));
      strncpy(msg_id, json_value->valuestring, sizeof(char) * (NUM_TRYTES_MAM_MSG_ID + 1));
    } else {
      ta_log_error("%s\n", "SC_CCLIENT_JSON_PARSE");
      return SC_CCLIENT_JSON_PARSE;
    }
  }

set_data_id:
  ret = recv_mam_set_mam_v1_data_id(req, bundle_hash, chid, epid, msg_id);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

done:
  free(psk);
  free(ntru);
  free(bundle_hash);
  free(chid);
  free(epid);
  free(msg_id);
  return ret;
}

status_t recv_mam_message_req_deserialize(const char* const obj, ta_recv_mam_req_t* const req) {
  if (obj == NULL || req == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  const int protocol_len = 20;
  char protocol_str[protocol_len];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  ret = ta_json_get_string(json_obj, "protocol", protocol_str, protocol_len);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }
  if (!strncmp(protocol_str, "MAM_V1", strlen("MAM_V1"))) {
    req->protocol = MAM_V1;
  }

  switch (req->protocol) {
    case MAM_V1:
      ret = recv_mam_message_mam_v1_req_deserialize(json_obj, req);
      if (ret) {
        ta_log_error("%d\n", ret);
        goto done;
      }
      break;

    default:
      break;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t recv_mam_message_res_serialize(UT_array* const payload_array, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  ret = string_utarray_to_json_array(payload_array, json_root, "payload");
  if (ret) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
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
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  cJSON_AddStringToObject(json_root, "bundle_hash", res->bundle_hash);

  cJSON_AddStringToObject(json_root, "chid", res->chid);

  cJSON_AddStringToObject(json_root, "epid", res->epid);

  cJSON_AddStringToObject(json_root, "msg_id", res->msg_id);

  if (res->announcement_bundle_hash[0]) {
    cJSON_AddStringToObject(json_root, "announcement_bundle_hash", res->announcement_bundle_hash);
  }

  if (res->chid1[0]) {
    cJSON_AddStringToObject(json_root, "chid1", res->chid1);
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t send_mam_res_deserialize(const char* const obj, ta_send_mam_res_t* const res) {
  if (obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  status_t ret = SC_OK;
  tryte_t addr[NUM_TRYTES_ADDRESS + 1], msg_id[MAM_MSG_ID_SIZE];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  if (ta_json_get_string(json_obj, "chid", (char*)addr, NUM_TRYTES_ADDRESS + 1) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_channel_id(res, addr);

  if (ta_json_get_string(json_obj, "epid", (char*)addr, NUM_TRYTES_ADDRESS + 1) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_endpoint_id(res, addr);

  if (ta_json_get_string(json_obj, "msg_id", (char*)msg_id, MAM_MSG_ID_SIZE / 3 + 1) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_msg_id(res, msg_id);

  if (ta_json_get_string(json_obj, "bundle_hash", (char*)addr, NUM_TRYTES_ADDRESS + 1) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_bundle_hash(res, addr);

  if (ta_json_get_string(json_obj, "chid1", (char*)addr, NUM_TRYTES_ADDRESS + 1) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_chid1(res, addr);

  if (ta_json_get_string(json_obj, "announcement_bundle_hash", (char*)addr, NUM_TRYTES_ADDRESS + 1) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }
  send_mam_res_set_announcement_bundle_hash(res, addr);

done:
  cJSON_Delete(json_obj);
  return ret;
}

#ifdef MQTT_ENABLE
status_t mqtt_device_id_deserialize(const char* const obj, char* device_id) {
  if (obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  ret = ta_json_get_string(json_obj, "device_id", device_id, ID_LEN + 1);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t mqtt_tag_req_deserialize(const char* const obj, char* tag) {
  if (obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  ret = ta_json_get_string(json_obj, "tag", tag, NUM_TRYTES_TAG + 1);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t mqtt_transaction_hash_req_deserialize(const char* const obj, char* hash) {
  if (obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  ret = ta_json_get_string(json_obj, "hash", hash, NUM_TRYTES_HASH + 1);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}
#endif

status_t proxy_apis_command_req_deserialize(const char* const obj, char* command) {
  if (obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  const uint8_t max_cmd_len = 30;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  ret = ta_json_get_string(json_obj, "command", command, max_cmd_len);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t get_iri_status_milestone_deserialize(char const* const obj, int* const latestMilestoneIndex,
                                              int* const latestSolidSubtangleMilestoneIndex) {
  if (obj == NULL) {
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_value = NULL;
  status_t ret = SC_OK;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_obj, "latestMilestoneIndex");
  if (cJSON_IsNumber(json_value)) {
    *latestMilestoneIndex = json_value->valueint;
  } else {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
    goto done;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_obj, "latestSolidSubtangleMilestoneIndex");
  if (cJSON_IsNumber(json_value)) {
    *latestSolidSubtangleMilestoneIndex = json_value->valueint;
  } else {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", "SC_SERIALIZER_NULL");
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t get_iri_status_res_serialize(const status_t status, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_CREATE");
    goto done;
  }

  if (status == SC_OK) {
    cJSON_AddBoolToObject(json_root, "status", true);
  } else {
    cJSON_AddBoolToObject(json_root, "status", false);
  }

  if (status == SC_CCLIENT_FAILED_RESPONSE) {
    cJSON_AddStringToObject(json_root, "status_code", "SC_CCLIENT_FAILED_RESPONSE");
  } else if (status == SC_CORE_IRI_UNSYNC) {
    cJSON_AddStringToObject(json_root, "status_code", "SC_CORE_IRI_UNSYNC");
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", "SC_SERIALIZER_JSON_PARSE");
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}
