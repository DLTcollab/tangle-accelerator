/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "serializer.h"
#include "ser_helper.h"
#ifdef MQTT_ENABLE
#include "connectivity/mqtt/mqtt_common.h"
#endif
#include "common/logger.h"
#include "time.h"

#define SERI_LOGGER "serializer"
#define logger_id ser_logger_id

void serializer_logger_init() { logger_id = logger_helper_enable(SERI_LOGGER, LOGGER_DEBUG, true); }

int serializer_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

status_t ta_get_info_serialize(char** obj, ta_config_t* const ta_config, iota_config_t* const tangle,
                               ta_cache_t* const cache) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_CREATE));
    return SC_SERIALIZER_JSON_CREATE;
  }

  cJSON_AddStringToObject(json_root, "name", "tangle-accelerator");
  cJSON_AddStringToObject(json_root, "host", ta_config->host);
  cJSON_AddStringToObject(json_root, "version", ta_config->version);
  cJSON_AddNumberToObject(json_root, "port", ta_config->port);
  cJSON_AddNumberToObject(json_root, "thread", ta_config->http_tpool_size);
  cJSON_AddStringToObject(json_root, "redis_host", cache->host);
  cJSON_AddNumberToObject(json_root, "redis_port", cache->port);
  cJSON_AddNumberToObject(json_root, "milestone_depth", tangle->milestone_depth);
  cJSON_AddNumberToObject(json_root, "mwm", tangle->mwm);
  cJSON_AddBoolToObject(json_root, "quiet", is_option_enabled(ta_config, CLI_QUIET_MODE));

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
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
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_CREATE));
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
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
    ret = SC_SERIALIZER_JSON_PARSE;
  }
  cJSON_Delete(json_root);
  return ret;
}
#endif

status_t ta_send_transfer_req_deserialize(const char* const obj, ta_send_transfer_req_t* req) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_result = NULL;
  flex_trit_t tag_trits[NUM_TRITS_TAG], address_trits[NUM_TRITS_HASH];
  bool raw_message = true;
  status_t ret = SC_OK;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "value");
  if ((json_result != NULL) && cJSON_IsNumber(json_result)) {
    req->value = json_result->valueint;
  } else {
    // 'value' does not exist or invalid, set to 0
    req->value = 0;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "address");
  if (json_result != NULL && json_result->valuestring != NULL) {
    int addr_len = strlen(json_result->valuestring);
    if (!valid_tryte(json_result->valuestring, addr_len)) {
      ret = SC_SERIALIZER_JSON_PARSE_NOT_TRYTE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    flex_trits_from_trytes(address_trits, NUM_TRITS_ADDRESS, (const tryte_t*)json_result->valuestring,
                           NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
  } else {
    // 'address' does not exists, set to DEFAULT_ADDRESS
    flex_trits_from_trytes(address_trits, NUM_TRITS_ADDRESS, (const tryte_t*)DEFAULT_ADDRESS, NUM_TRYTES_ADDRESS,
                           NUM_TRYTES_ADDRESS);
  }
  ret = hash243_queue_push(&req->address, address_trits);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  json_result = cJSON_GetObjectItemCaseSensitive(json_obj, "tag");
  if (json_result != NULL && json_result->valuestring != NULL) {
    int tag_len = strlen(json_result->valuestring);

    if (!valid_tryte(json_result->valuestring, tag_len)) {
      ret = SC_SERIALIZER_JSON_PARSE_NOT_TRYTE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
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
    ta_log_error("%s\n", ta_error_to_string(ret));
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
    int msg_len = strlen(json_result->valuestring);

    // In case the payload is unicode, the character whose ASCII code is beyond 128 result to an
    // error status_t code
    for (int i = 0; i < msg_len; i++) {
      if (json_result->valuestring[i] & 0x80) {
        ret = SC_SERIALIZER_JSON_PARSE_NOT_TRYTE;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }

    if (raw_message) {
      req->msg_len = msg_len * 2;
      if (req->msg_len > NUM_TRYTES_MESSAGE) {
        ret = SC_SERIALIZER_MESSAGE_OVERRUN;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }

      ascii_to_trytes(json_result->valuestring, req->message);
    } else {
      req->msg_len = msg_len;
      if (req->msg_len > NUM_TRYTES_MESSAGE) {
        ret = SC_SERIALIZER_MESSAGE_OVERRUN;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
      memcpy(req->message, json_result->valuestring, req->msg_len);
    }

  } else {
    // 'message' does not exists, set to DEFAULT_MSG
    req->msg_len = DEFAULT_MSG_LEN * 3;
    flex_trits_from_trytes(req->message, req->msg_len, (const tryte_t*)DEFAULT_MSG, DEFAULT_MSG_LEN, DEFAULT_MSG_LEN);
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_send_trytes_req_deserialize(const char* const obj, hash8019_array_p out_trytes) {
  if (obj == NULL || out_trytes == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
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
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }

  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();

  ret = ta_hash8019_array_to_json_array(trytes, "trytes", json_root);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
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
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }

  cJSON* json_obj = cJSON_Parse(obj);
  if (json_obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
    return SC_SERIALIZER_JSON_PARSE;
  }

  ret = ta_json_array_to_hash243_queue(json_obj, "hashes", &req->hashes);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
  }

  cJSON_Delete(json_obj);
  return ret;
}

status_t ta_find_transaction_object_single_res_serialize(transaction_array_t* res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_JSON_CREATE));
    return SC_CCLIENT_JSON_CREATE;
  }

  ret = ta_iota_transaction_to_json_object(transaction_array_at(res, 0), &json_root);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_find_transaction_objects_res_serialize(const transaction_array_t* const res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateArray();
  if (json_root == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CCLIENT_JSON_CREATE));
    return SC_CCLIENT_JSON_CREATE;
  }

  ret = ta_transaction_array_to_json_array(res, json_root);
  if (ret != SC_OK) {
    goto done;
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
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
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    goto done;
  }

  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_CREATE));
    goto done;
  }

  ret = ta_hash243_queue_to_json_array(res->hashes, json_root);
  if (ret) {
    goto done;
  }
  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t ta_send_transfer_res_serialize(ta_send_transfer_res_t* res, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (res->uuid) {
    cJSON_AddStringToObject(json_root, "uuid", res->uuid);
    cJSON_AddStringToObject(json_root, "address", (char*)res->address);
  } else {
    ret = ta_iota_transaction_to_json_object(transaction_array_at(res->txn_array, 0), &json_root);
    if (ret != SC_OK) {
      ta_log_error("%s\n", error_2_string(ret));
      goto done;
    }
#ifdef DB_ENABLE
    cJSON_AddStringToObject(json_root, "id", res->uuid_string);
#endif
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

#ifdef MQTT_ENABLE
status_t mqtt_device_id_deserialize(const char* const obj, char* device_id) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_json_get_string(json_obj, "device_id", device_id, ID_LEN + 1);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t mqtt_tag_req_deserialize(const char* const obj, char* tag) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_json_get_string(json_obj, "tag", tag, NUM_TRYTES_TAG + 1);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t mqtt_transaction_hash_req_deserialize(const char* const obj, char* hash) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_json_get_string(json_obj, "hash", hash, NUM_TRYTES_HASH + 1);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}
#endif

status_t proxy_apis_command_req_deserialize(const char* const obj, char* command) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  const uint8_t max_cmd_len = 30;
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_json_get_string(json_obj, "command", command, max_cmd_len);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t get_node_status_milestone_deserialize(char const* const obj, int* const latestMilestoneIndex,
                                               int* const latestSolidSubtangleMilestoneIndex) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_value = NULL;
  status_t ret = SC_OK;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_obj, "latestMilestoneIndex");
  if (cJSON_IsNumber(json_value)) {
    *latestMilestoneIndex = json_value->valueint;
  } else {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_obj, "latestSolidSubtangleMilestoneIndex");
  if (cJSON_IsNumber(json_value)) {
    *latestSolidSubtangleMilestoneIndex = json_value->valueint;
  } else {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t get_node_status_res_serialize(const status_t status, char** obj) {
  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (status == SC_OK) {
    cJSON_AddBoolToObject(json_root, "status", true);
  } else {
    cJSON_AddBoolToObject(json_root, "status", false);
  }

  if (status == SC_CCLIENT_FAILED_RESPONSE) {
    cJSON_AddStringToObject(json_root, "status_code", "SC_CCLIENT_FAILED_RESPONSE");
  } else if (status == SC_CORE_NODE_UNSYNC) {
    cJSON_AddStringToObject(json_root, "status_code", "SC_CORE_NODE_UNSYNC");
  }

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

done:
  cJSON_Delete(json_root);
  return ret;
}

status_t fetch_buffered_request_status_req_deserialize(char* obj, char* uuid) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_value = NULL;
  status_t ret = SC_OK;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_obj, "uuid");
  if (json_value != NULL) {
    strncpy(uuid, json_value->valuestring, UUID_STR_LEN - 1);
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t fetch_buffered_request_status_res_serialize(const ta_fetch_buffered_request_status_res_t* const res,
                                                     char** obj) {
  status_t ret = SC_OK;
  if (res == NULL) {
    ret = SC_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (res->status == SENT) {
    if (res->mam_result) {
      *obj = strdup(res->mam_result);
      // Goto the end of this function, since the buffered result of MAM request has been serialized.
      goto done;
    } else {
      ret = ta_bundle_transaction_to_json_array(res->bundle, "bundle", json_root);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }
  } else if (res->status == UNSENT) {
    cJSON_AddStringToObject(json_root, "status", "unsent");
  } else if (res->status == NOT_EXIST) {
    cJSON_AddStringToObject(json_root, "status", "not_exist");
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
