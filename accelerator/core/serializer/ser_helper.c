/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ser_helper.h"
#include "time.h"

#define logger_id ser_logger_id

bool valid_tryte(char* trytes, int len) {
  for (int i = 0; i < len; i++)
    if ((trytes[i] < 'A' || trytes[i] > 'Z') && trytes[i] != '9') {
      return false;
    }
  return true;
}

status_t ta_string_utarray_to_json_array(UT_array const* const ut, char const* const obj_name, cJSON* const json_root) {
  char** p = NULL;
  if (!ut) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  cJSON* array_obj = cJSON_CreateArray();
  if (array_obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_CREATE));
    return SC_SERIALIZER_JSON_CREATE;
  }

  cJSON_AddItemToObject(json_root, obj_name, array_obj);

  while ((p = (char**)utarray_next(ut, p))) {
    cJSON_AddItemToArray(array_obj, cJSON_CreateString(*p));
  }
  return SC_OK;
}

status_t ta_json_string_array_to_string_utarray(cJSON const* const obj, char const* const obj_name,
                                                UT_array* const ut) {
  char* str = NULL;

  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (cJSON_IsArray(json_item)) {
    cJSON* array_elt = NULL;
    cJSON_ArrayForEach(array_elt, json_item) {
      str = cJSON_GetStringValue(array_elt);
      if (!str) {
        ta_log_error("%s\n", "Encountered non-string array member");
        return SC_SERIALIZER_JSON_PARSE;
      }
      utarray_push_back(ut, &str);
    }
  } else {
    ta_log_error("%s\n", "Not an array");
    return SC_SERIALIZER_JSON_PARSE;
  }

  return SC_OK;
}

status_t ta_hash243_stack_to_json_array(hash243_stack_t stack, cJSON* json_root) {
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
        ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_INVALID_FLEX_TRITS));
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_NOT_FOUND));
    return SC_CCLIENT_NOT_FOUND;
  }
  return SC_OK;
}

status_t ta_json_array_to_hash243_queue(cJSON const* const obj, char const* const obj_name, hash243_queue_t* queue) {
  status_t ret_code = SC_OK;
  flex_trit_t hash[FLEX_TRIT_SIZE_243] = {};
  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (!json_item) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }

  if (cJSON_IsArray(json_item)) {
    cJSON* current_obj = NULL;
    cJSON_ArrayForEach(current_obj, json_item) {
      if (current_obj->valuestring != NULL) {
        flex_trits_from_trytes(hash, NUM_TRITS_HASH, (tryte_t const*)current_obj->valuestring, NUM_TRYTES_HASH,
                               NUM_TRYTES_HASH);
        if (hash243_queue_push(queue, hash) != RC_OK) {
          ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
          return SC_SERIALIZER_JSON_PARSE;
        }
      }
    }
  } else {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
    return SC_SERIALIZER_JSON_PARSE;
  }
  return ret_code;
}

status_t ta_hash243_queue_to_json_array(hash243_queue_t queue, cJSON* const json_root) {
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
        ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_INVALID_FLEX_TRITS));
        return SC_CCLIENT_INVALID_FLEX_TRITS;
      }
    }
  } else {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_NOT_FOUND));
    return SC_CCLIENT_NOT_FOUND;
  }
  return SC_OK;
}

status_t ta_json_array_to_hash8019_array(cJSON const* const obj, char const* const obj_name, hash8019_array_p array) {
  status_t ret = SC_OK;
  flex_trit_t hash[FLEX_TRIT_SIZE_8019] = {};
  cJSON* json_item = cJSON_GetObjectItemCaseSensitive(obj, obj_name);
  if (!cJSON_IsArray(json_item)) {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_JSON_PARSE));
    return SC_CCLIENT_JSON_PARSE;
  }

  cJSON* current_obj = NULL;
  cJSON_ArrayForEach(current_obj, json_item) {
    if (current_obj->valuestring != NULL) {
      if (strlen(current_obj->valuestring) != NUM_TRYTES_SERIALIZED_TRANSACTION) {
        ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_INVALID_REQ));
        return SC_SERIALIZER_INVALID_REQ;
      }
      flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t const*)current_obj->valuestring,
                             NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
      hash_array_push(array, hash);
    }
  }
  return ret;
}

status_t ta_hash8019_array_to_json_array(hash8019_array_p array, char const* const obj_name, cJSON* const json_root) {
  size_t array_count = 0;
  cJSON* array_obj = NULL;
  tryte_t trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION + 1] = {};
  size_t trits_count = 0;
  flex_trit_t* elt = NULL;

  array_count = hash_array_len(array);
  if (array_count > 0) {
    array_obj = cJSON_CreateArray();
    if (array_obj == NULL) {
      ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_CREATE));
      return SC_SERIALIZER_JSON_CREATE;
    }
    cJSON_AddItemToObject(json_root, obj_name, array_obj);

    HASH_ARRAY_FOREACH(array, elt) {
      trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_SERIALIZED_TRANSACTION, elt,
                                         NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
      trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';
      if (trits_count == 0) {
        ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_FLEX_TRITS));
        return SC_CCLIENT_FLEX_TRITS;
      }
      cJSON_AddItemToArray(array_obj, cJSON_CreateString((char const*)trytes_out));
    }
  }
  return SC_OK;
}

status_t ta_iota_transaction_to_json_object(iota_transaction_t const* const txn, cJSON** txn_json) {
  if (txn == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_NOT_FOUND));
    return SC_CCLIENT_NOT_FOUND;
  }
  char msg_trytes[NUM_TRYTES_SIGNATURE + 1], hash_trytes[NUM_TRYTES_HASH + 1], tag_trytes[NUM_TRYTES_TAG + 1];

  if (txn_json == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_CREATE));
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

status_t ta_transaction_array_to_json_array(const transaction_array_t* const txn_array, cJSON* json_root) {
  status_t ret = SC_OK;
  iota_transaction_t* txn = NULL;

  TX_OBJS_FOREACH(txn_array, txn) {
    cJSON* txn_obj = cJSON_CreateObject();
    if (!txn_obj) {
      ret = SC_SERIALIZER_JSON_CREATE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      return ret;
    }

    ret = ta_iota_transaction_to_json_object(txn, &txn_obj);
    if (ret != SC_OK) {
      cJSON_Delete(txn_obj);
      ta_log_error("%s\n", ta_error_to_string(ret));
      return ret;
    }
    cJSON_AddItemToArray(json_root, txn_obj);
  }
  return ret;
}

status_t ta_bundle_transaction_to_json_array(const bundle_transactions_t* const bundle, char const* const obj_name,
                                             cJSON* json_root) {
  status_t ret = SC_OK;

  cJSON* json_array = cJSON_CreateArray();
  if (json_array == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  iota_transaction_t* txn = NULL;
  BUNDLE_FOREACH(bundle, txn) {
    cJSON* txn_obj = cJSON_CreateObject();
    if (!txn_obj) {
      ret = SC_SERIALIZER_JSON_CREATE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      return ret;
    }

    ret = ta_iota_transaction_to_json_object(txn, &txn_obj);
    if (ret != SC_OK) {
      cJSON_Delete(txn_obj);
      ta_log_error("%s\n", ta_error_to_string(ret));
      return ret;
    }
    cJSON_AddItemToArray(json_array, txn_obj);
  }

  cJSON_AddItemToObject(json_root, obj_name, json_array);

done:
  return ret;
}

status_t ta_json_get_string(cJSON const* const json_obj, char const* const obj_name, char* const text,
                            const size_t size) {
  status_t ret = SC_OK;
  if (json_obj == NULL || obj_name == NULL || text == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }

  cJSON* json_value = cJSON_GetObjectItemCaseSensitive(json_obj, obj_name);
  if (json_value == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_JSON_KEY));
    return SC_CCLIENT_JSON_KEY;
  }

  if (cJSON_IsString(json_value) && (json_value->valuestring != NULL)) {
    strncpy(text, json_value->valuestring, size);
  } else {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_JSON_PARSE));
    return SC_CCLIENT_JSON_PARSE;
  }

  return ret;
}
