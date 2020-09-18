/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ser_mam.h"
#include "cJSON.h"
#include "common/logger.h"
#include "ser_helper.h"

#define logger_id ser_logger_id

static status_t send_mam_message_mam_v1_req_deserialize(cJSON const* const json_obj, ta_send_mam_req_t* const req) {
  cJSON *json_key = NULL, *json_value = NULL;
  status_t ret = SC_OK;
  send_mam_data_mam_v1_t* data = (send_mam_data_mam_v1_t*)req->data;
  send_mam_key_mam_v1_t* key = (send_mam_key_mam_v1_t*)req->key;

  ret = ta_json_get_string(json_obj, "x-api-key", req->service_token, SERVICE_TOKEN_LEN);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (cJSON_HasObjectItem(json_obj, "key")) {
    json_key = cJSON_GetObjectItem(json_obj, "key");
    if (json_key == NULL) {
      ret = SC_SERIALIZER_JSON_PARSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    if (cJSON_HasObjectItem(json_key, "psk")) {
      ret = ta_json_string_array_to_string_utarray(json_key, "psk", key->psk_array);
      if (ret != SC_OK) {
        ret = SC_SERIALIZER_JSON_PARSE;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }

    if (cJSON_HasObjectItem(json_key, "ntru")) {
      ret = ta_json_string_array_to_string_utarray(json_key, "ntru", key->ntru_array);
      if (ret != SC_OK) {
        ret = SC_SERIALIZER_JSON_PARSE;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }
  }

  json_key = cJSON_GetObjectItem(json_obj, "data");
  if (json_key == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "seed");
  if (json_value && json_value->valuestring) {
    size_t seed_size = strnlen(json_value->valuestring, NUM_TRYTES_ADDRESS);

    if (seed_size != NUM_TRYTES_HASH) {
      ret = SC_SERIALIZER_INVALID_REQ;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    data->seed = (tryte_t*)malloc(sizeof(tryte_t) * (NUM_TRYTES_ADDRESS + 1));
    snprintf((char*)data->seed, seed_size + 1, "%s", json_value->valuestring);
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "chid");
  if (json_value && json_value->valuestring) {
    size_t chid_size = strnlen(json_value->valuestring, NUM_TRYTES_ADDRESS);

    if (chid_size != NUM_TRYTES_HASH) {
      ret = SC_SERIALIZER_INVALID_REQ;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    data->chid = (tryte_t*)malloc(sizeof(tryte_t) * (NUM_TRYTES_ADDRESS + 1));
    snprintf((char*)data->chid, chid_size + 1, "%s", json_value->valuestring);
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "message");
  if (json_value && json_value->valuestring) {
    size_t payload_size = strlen(json_value->valuestring);
    data->message = (char*)malloc((payload_size + 1) * sizeof(char));

    // In case the payload is unicode, the character whose ASCII code is beyond 128 result to an
    // error status_t code
    for (size_t i = 0; i < payload_size; i++) {
      if (json_value->valuestring[i] & 0x80) {
        ret = SC_SERIALIZER_JSON_PARSE_NOT_TRYTE;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }
    memcpy(data->message, json_value->valuestring, payload_size + 1);
  } else {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    ret = SC_SERIALIZER_NULL;
  }

  json_value = cJSON_GetObjectItemCaseSensitive(json_key, "ch_mss_depth");
  if ((json_value != NULL) && cJSON_IsNumber(json_value)) {
    data->ch_mss_depth = json_value->valueint;
  }

done:
  return ret;
}

static status_t recv_mam_message_mam_v1_req_deserialize(cJSON const* const json_obj, ta_recv_mam_req_t* const req) {
  cJSON *json_key = NULL, *json_value = NULL;
  status_t ret = SC_OK;
  char *bundle_hash = NULL, *chid = NULL, *msg_id = NULL, *psk = NULL, *ntru = NULL;

  recv_mam_key_mam_v1_t* key = (recv_mam_key_mam_v1_t*)req->key;
  if (cJSON_HasObjectItem(json_obj, "key")) {
    json_key = cJSON_GetObjectItem(json_obj, "key");
    if (json_key == NULL) {
      ret = SC_SERIALIZER_JSON_PARSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    if (cJSON_HasObjectItem(json_key, "psk")) {
      ret = ta_json_string_array_to_string_utarray(json_key, "psk", key->psk_array);
      if (ret != SC_OK) {
        ret = SC_SERIALIZER_JSON_PARSE;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }

    if (cJSON_HasObjectItem(json_key, "ntru")) {
      ret = ta_json_string_array_to_string_utarray(json_key, "ntru", key->ntru_array);
      if (ret != SC_OK) {
        ret = SC_SERIALIZER_JSON_PARSE;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }
  }

  json_key = cJSON_GetObjectItem(json_obj, "data_id");
  if (json_key == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (cJSON_HasObjectItem(json_key, "bundle_hash")) {
    json_value = cJSON_GetObjectItemCaseSensitive(json_key, "bundle_hash");
    if (json_value == NULL) {
      ret = SC_CCLIENT_JSON_KEY;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    if (cJSON_IsString(json_value) && (json_value->valuestring != NULL) &&
        (strlen(json_value->valuestring) == NUM_TRYTES_HASH)) {
      bundle_hash = (char*)malloc(sizeof(char) * (NUM_TRYTES_HASH + 1));
      strncpy(bundle_hash, json_value->valuestring, sizeof(char) * (NUM_TRYTES_HASH + 1));
    } else {
      ret = SC_CCLIENT_JSON_PARSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    goto set_data_id;
  }

  if (cJSON_HasObjectItem(json_key, "chid")) {
    json_value = cJSON_GetObjectItemCaseSensitive(json_key, "chid");
    if (json_value == NULL) {
      ret = SC_CCLIENT_JSON_KEY;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    if (cJSON_IsString(json_value) &&
        (json_value->valuestring != NULL && (strlen(json_value->valuestring) == NUM_TRYTES_HASH))) {
      chid = (char*)malloc(sizeof(char) * (NUM_TRYTES_HASH + 1));
      strncpy(chid, json_value->valuestring, sizeof(char) * (NUM_TRYTES_HASH + 1));
    } else {
      ret = SC_CCLIENT_JSON_PARSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  }

  if (cJSON_HasObjectItem(json_key, "msg_id")) {
    json_value = cJSON_GetObjectItemCaseSensitive(json_key, "msg_id");
    if (json_value == NULL) {
      ret = SC_CCLIENT_JSON_KEY;
      ta_log_error("%s\n", ta_error_to_string(ret));
      return ret;
    }
    if (cJSON_IsString(json_value) && (json_value->valuestring != NULL) &&
        (strlen(json_value->valuestring) == NUM_TRYTES_MAM_MSG_ID)) {
      msg_id = (char*)malloc(sizeof(char) * (NUM_TRYTES_MAM_MSG_ID + 1));
      strncpy(msg_id, json_value->valuestring, sizeof(char) * (NUM_TRYTES_MAM_MSG_ID + 1));
    } else {
      ret = SC_CCLIENT_JSON_PARSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  }

set_data_id:
  ret = recv_mam_set_mam_v1_data_id(req, bundle_hash, chid, msg_id);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

done:
  free(psk);
  free(ntru);
  free(bundle_hash);
  free(chid);
  free(msg_id);
  return ret;
}

status_t send_mam_message_req_deserialize(const char* const obj, ta_send_mam_req_t* const req) {
  if (obj == NULL || req == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  const int protocol_len = 20;
  char protocol_str[protocol_len];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_json_get_string(json_obj, "protocol", protocol_str, protocol_len);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  if (!strncmp(protocol_str, "MAM_V1", strlen("MAM_V1"))) {
    req->protocol = MAM_V1;
  }

  send_mam_req_v1_init(req);
  ret = send_mam_message_mam_v1_req_deserialize(json_obj, req);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t send_mam_message_res_deserialize(const char* const obj, ta_send_mam_res_t* const res) {
  if (obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);
  status_t ret = SC_OK;
  tryte_t addr[NUM_TRYTES_ADDRESS + 1], msg_id[MAM_MSG_ID_SIZE];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (ta_json_get_string(json_obj, "chid", (char*)addr, NUM_TRYTES_ADDRESS) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  send_mam_res_set_channel_id(res, addr);

  if (ta_json_get_string(json_obj, "msg_id", (char*)msg_id, NUM_TRYTES_MAM_MSG_ID) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  send_mam_res_set_msg_id(res, msg_id);

  if (ta_json_get_string(json_obj, "bundle_hash", (char*)addr, NUM_TRYTES_ADDRESS) != SC_OK) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  send_mam_res_set_bundle_hash(res, addr);

  if (cJSON_HasObjectItem(json_obj, "chid1")) {
    if (ta_json_get_string(json_obj, "chid1", (char*)addr, NUM_TRYTES_ADDRESS) != SC_OK) {
      ret = SC_SERIALIZER_NULL;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    send_mam_res_set_chid1(res, addr);
  }

  if (cJSON_HasObjectItem(json_obj, "announcement_bundle_hash")) {
    if (ta_json_get_string(json_obj, "announcement_bundle_hash", (char*)addr, NUM_TRYTES_ADDRESS) != SC_OK) {
      ret = SC_SERIALIZER_NULL;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    send_mam_res_set_announce_bundle_hash(res, addr);
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t send_mam_message_res_serialize(const ta_send_mam_res_t* const res, char const* const uuid, char** obj) {
  status_t ret = SC_OK;
  if ((!res && !uuid) || !obj) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    return ret;
  }

  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (uuid) {
    cJSON_AddStringToObject(json_root, "uuid", uuid);
  } else {
    cJSON_AddStringToObject(json_root, "bundle_hash", res->bundle_hash);

    cJSON_AddStringToObject(json_root, "chid", res->chid);

    cJSON_AddStringToObject(json_root, "msg_id", res->msg_id);

    if (res->announcement_bundle_hash[0]) {
      cJSON_AddStringToObject(json_root, "announcement_bundle_hash", res->announcement_bundle_hash);
    }

    if (res->chid1[0]) {
      cJSON_AddStringToObject(json_root, "chid1", res->chid1);
    }
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

status_t recv_mam_message_req_deserialize(const char* const obj, ta_recv_mam_req_t* const req) {
  if (obj == NULL || req == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);

  const int protocol_len = 20;
  char protocol_str[protocol_len];

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_json_get_string(json_obj, "protocol", protocol_str, protocol_len);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  if (!strncmp(protocol_str, "MAM_V1", strlen("MAM_V1"))) {
    req->protocol = MAM_V1;
  }

  switch (req->protocol) {
    case MAM_V1:
      ret = recv_mam_req_v1_init(req);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
      ret = recv_mam_message_mam_v1_req_deserialize(json_obj, req);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
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

status_t recv_mam_message_res_deserialize(const char* const obj, ta_recv_mam_res_t* const res) {
  status_t ret = SC_OK;
  if (!obj || !res) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  cJSON* json_obj = cJSON_Parse(obj);

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_json_string_array_to_string_utarray(json_obj, "payload", res->payload_array);
  if (ret) {
    ret = SC_CCLIENT_JSON_KEY;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (cJSON_HasObjectItem(json_obj, "chid1")) {
    ret = ta_json_get_string(json_obj, "chid1", res->chid1, NUM_TRYTES_ADDRESS);
    if (ret) {
      ret = SC_CCLIENT_JSON_KEY;
      ta_log_error("%s\n", ta_error_to_string(ret));
    }
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t recv_mam_message_res_serialize(ta_recv_mam_res_t* const res, char** obj) {
  status_t ret = SC_OK;
  if (!res || !obj) {
    ret = SC_SERIALIZER_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    return ret;
  }

  cJSON* json_root = cJSON_CreateObject();
  if (json_root == NULL) {
    ret = SC_SERIALIZER_JSON_CREATE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_string_utarray_to_json_array(res->payload_array, "payload", json_root);
  if (ret) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (res->chid1[0]) {
    cJSON_AddStringToObject(json_root, "chid1", res->chid1);
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

status_t register_mam_channel_req_deserialize(const char* const obj, ta_register_mam_channel_req_t* req) {
  if (obj == NULL || req == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_item = NULL;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  json_item = cJSON_GetObjectItemCaseSensitive(json_obj, "seed");
  if (json_item == NULL) {
    ret = SC_SERIALIZER_KEY_NOT_EXISTS;
    ta_log_error("%s\n", ta_error_to_string(ret));
  } else if (json_item->valuestring == NULL) {
    ret = SC_SERIALIZER_VALUE_EMPTY;
    ta_log_error("%s\n", ta_error_to_string(ret));
  } else if (strlen(json_item->valuestring) != NUM_TRYTES_ADDRESS) {
    ret = SC_SERIALIZER_VALUE_INVLID;
    ta_log_error("%s\n", ta_error_to_string(ret));
  } else {
    strncpy(req->seed, json_item->valuestring, NUM_TRYTES_ADDRESS);
    req->seed[NUM_TRYTES_ADDRESS] = 0;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}

status_t register_mam_channel_res_serialize(const char* const uuid, char** obj) {
  if (uuid == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }

  status_t ret = SC_OK;
  cJSON* json_root = cJSON_CreateObject();

  cJSON_AddStringToObject(json_root, "user-id", uuid);

  *obj = cJSON_PrintUnformatted(json_root);
  if (*obj == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_JSON_PARSE));
    ret = SC_SERIALIZER_JSON_PARSE;
  }

  cJSON_Delete(json_root);
  return ret;
}

status_t register_mam_channel_res_deserialize(const char* const obj, char* user_id) {
  if (obj == NULL || user_id == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_SERIALIZER_NULL));
    return SC_SERIALIZER_NULL;
  }
  status_t ret = SC_OK;
  cJSON* json_obj = cJSON_Parse(obj);
  cJSON* json_item = NULL;

  if (json_obj == NULL) {
    ret = SC_SERIALIZER_JSON_PARSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  json_item = cJSON_GetObjectItemCaseSensitive(json_obj, "user-id");
  if (json_item == NULL) {
    ret = SC_SERIALIZER_KEY_NOT_EXISTS;
    ta_log_error("%s\n", ta_error_to_string(ret));
  } else if (json_item->valuestring == NULL) {
    ret = SC_SERIALIZER_VALUE_EMPTY;
    ta_log_error("%s\n", ta_error_to_string(ret));
  } else if (strlen(json_item->valuestring) != UUID_STR_LEN - 1) {
    ret = SC_SERIALIZER_VALUE_INVLID;
    ta_log_error("%s\n", ta_error_to_string(ret));
  } else {
    strncpy(user_id, json_item->valuestring, UUID_STR_LEN - 1);
    user_id[UUID_STR_LEN - 1] = 0;
  }

done:
  cJSON_Delete(json_obj);
  return ret;
}
