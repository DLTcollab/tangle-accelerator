/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "periodical_task.h"

#define BK_LOGGER "core"

static logger_id_t logger_id;

void bk_logger_init() { logger_id = logger_helper_enable(BK_LOGGER, LOGGER_DEBUG, true); }

int bk_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", BK_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

/**
 * @brief Update the binding IOTA full node to another valid host on priority list
 *
 * ta_update_full_node_connection would check the connection status of all the IOTA full node on priority list
 * iteratively. Once it connects to one of the IOTA full node on the priority list, it would return SC_OK.
 *
 * @param[in] ta_conf Tangle-accelerator configuration variables
 * @param[in] service service IOTA full node endpoint service
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static status_t ta_update_full_node_connection(ta_config_t* const ta_conf, iota_client_service_t* const service) {
  status_t ret = SC_OK;
  for (int i = 0; i < MAX_NODE_LIST_ELEMENTS && ta_conf->iota_host_list[i]; i++) {
    // update new IOTA full node

    strncpy(service->http.host, ta_conf->iota_host_list[i], HOST_MAX_LEN - 1);
    service->http.port = ta_conf->iota_port_list[i];
    ta_log_info("Try to connect to %s:%d\n", service->http.host, service->http.port);

    // Run from the first one until found a good one.
    if (ta_get_node_status(service) == SC_OK) {
      ta_log_info("Connect to %s:%d\n", service->http.host, service->http.port);
      goto done;
    }
  }
  if (ret) {
    ta_log_error("All IOTA full node on priority list failed.\n");
  }

done:
  return ret;
}

status_t broadcast_buffered_txn(const ta_core_t* const core) {
  status_t ret = SC_OK;
  int uuid_list_len = 0;
  hash8019_array_p txn_trytes_array = NULL;

  /*
   *There are 4 data structures used here.
   * 1. List: A list of unsent uuid which can be used to identify an unsent transaction object
   * 2. Key-value: UUID to unsent transaction object in `flex_trit_t`
   * 3. List: Store all the UUID of sent transaction objects. (We could use set after investigation in the future)
   * 4. Key-value: UUID to sent transaction object in `flex_trit_t`
   *
   * 'push_txn_to_buffer()':
   *    Push UUID to the unsent UUID list.
   *    Store UUID as key and unsent transaction `flex_trit_t` as value.
   *
   * 'broadcast_buffered_txn()':
   *    Pop an unsent UUID in the unsent UUID list.
   *    Delete UUID-unsent_transaction pair from key value storage
   *    Store UUID as key and sent transaction object in `flex_trit_t` as value.
   *    Push UUID of sent transaction into sent transaction list.
   *
   * 'ta_fetch_buffered_request_status()':
   *    Fetch transaction object with UUID in key-value storage.
   *    Delete UUID from sent transaction list
   *    Delete UUID-sent_transaction pair from key-value storage
   */

  get_trytes_req_t* req = NULL;
  get_trytes_res_t* res = NULL;
  do {
    char uuid[UUID_STR_LEN];
    txn_trytes_array = hash8019_array_new();

    ret = cache_list_size(core->cache.buffer_list_name, &uuid_list_len);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    if (uuid_list_len == 0) {
      ta_log_debug("No buffered requests\n");
      goto done;
    }

    ret = cache_list_peek(core->cache.buffer_list_name, UUID_STR_LEN, uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // TODO Now we assume every time we call `cache_get()`, we would get a transaction object. However, in the future,
    // the returned result may be a bundle.
    int trytes_array_len = 0;
    ret = cache_list_size(uuid, &trytes_array_len);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    for (int i = 0; i < trytes_array_len; ++i) {
      flex_trit_t req_txn_flex_trits[NUM_FLEX_TRITS_SERIALIZED_TRANSACTION + 1] = {};
      ret = cache_list_at(uuid, i, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION, (char*)req_txn_flex_trits);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
      hash_array_push(txn_trytes_array, req_txn_flex_trits);
    }

    ret = ta_send_trytes(&core->ta_conf, &core->iota_conf, &core->iota_service, txn_trytes_array);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // TODO Update the transaction object saved in redis, which allows `ta_fetch_buffered_request_status()` to
    // return the transaction object much faster.
    req = get_trytes_req_new();
    res = get_trytes_res_new();
    iota_transaction_t txn;
    for (int i = 0; i < trytes_array_len; ++i) {
      transaction_deserialize_from_trits(&txn, hash_array_at(txn_trytes_array, i), true);
      flex_trit_t* hash = transaction_hash(&txn);

      ret = hash243_queue_push(&req->hashes, hash);
      if (ret) {
        ret = SC_CCLIENT_HASH;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }
    hash_array_free(txn_trytes_array);
    txn_trytes_array = NULL;

    ret = iota_client_get_trytes(&core->iota_service, req, res);
    if (ret) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // Delete the old transaction object
    ret = cache_del(uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    trytes_array_len = hash8019_queue_count(res->trytes);
    for (int i = 0; i < trytes_array_len; ++i) {
      ret = cache_list_push(uuid, UUID_STR_LEN - 1, hash8019_queue_at(res->trytes, i),
                            NUM_FLEX_TRITS_SERIALIZED_TRANSACTION);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }

    if (pthread_rwlock_trywrlock(core->cache.rwlock)) {
      ret = SC_CACHE_LOCK_FAILURE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    // Pop transaction from buffered list
    ret = cache_list_pop(core->cache.buffer_list_name, (char*)uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // Transfer the transaction to another list in where we store all the successfully broadcasted transactions.
    ret =
        cache_list_push(core->cache.complete_list_name, strlen(core->cache.complete_list_name), uuid, UUID_STR_LEN - 1);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    if (pthread_rwlock_unlock(core->cache.rwlock)) {
      ret = SC_CACHE_LOCK_FAILURE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    get_trytes_req_free(&req);
    get_trytes_res_free(&res);
  } while (uuid_list_len);

done:
  hash_array_free(txn_trytes_array);
  get_trytes_req_free(&req);
  get_trytes_res_free(&res);
  return ret;
}

status_t broadcast_buffered_send_mam_request(const ta_core_t* const core) {
  status_t ret = SC_OK;
  int uuid_list_len = 0;
  char* json = NULL;

  ta_send_mam_req_t* req = NULL;
  ta_send_mam_res_t* res = NULL;
  do {
    char uuid[UUID_STR_LEN];

    ret = cache_list_size(core->cache.mam_buffer_list_name, &uuid_list_len);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    if (uuid_list_len == 0) {
      ta_log_debug("No buffered requests\n");
      goto done;
    }

    ret = cache_list_peek(core->cache.mam_buffer_list_name, UUID_STR_LEN, uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    ret = cache_get(uuid, &json);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    req = send_mam_req_new();
    res = send_mam_res_new();
    if (!req || !res) {
      ret = SC_OOM;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    ret = send_mam_message_req_deserialize(json, req);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    free(json);
    json = NULL;

    ret = ta_send_mam_message(&core->ta_conf, &core->iota_conf, &core->iota_service, req, res);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    ret = send_mam_message_res_serialize(res, NULL, &json);
    if (ret != SC_OK) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    if (pthread_rwlock_trywrlock(core->cache.rwlock)) {
      ret = SC_CACHE_LOCK_FAILURE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // Delete the old transaction object
    ret = cache_del(uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    ret = cache_set(uuid, UUID_STR_LEN - 1, json, strlen(json), core->cache.timeout);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // Pop transaction from buffered list
    ret = cache_list_pop(core->cache.mam_buffer_list_name, (char*)uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // Transfer the transaction to another list in where we store all the successfully broadcasted transactions.
    ret = cache_list_push(core->cache.mam_complete_list_name, strlen(core->cache.mam_complete_list_name), uuid,
                          UUID_STR_LEN - 1);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    if (pthread_rwlock_unlock(core->cache.rwlock)) {
      ret = SC_CACHE_LOCK_FAILURE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    send_mam_req_free(&req);
    send_mam_res_free(&res);
    free(json);
    json = NULL;
  } while (uuid_list_len);

done:
  send_mam_req_free(&req);
  send_mam_res_free(&res);
  free(json);
  return ret;
}

void* health_track(void* arg) {
  ta_core_t* core = (ta_core_t*)arg;
  char uuid[UUID_STR_LEN] = {};

  while (core->cache.state) {
    status_t ret = ta_get_node_status(&core->iota_service);
    if (ret == SC_CORE_NODE_UNSYNC || ret == SC_CCLIENT_FAILED_RESPONSE) {
      ta_log_error("IOTA full node status error %d. Try to connect to another IOTA full node on priority list\n", ret);
      ret = ta_update_full_node_connection(&core->ta_conf, &core->iota_service);
      if (ret) {
        ta_log_error("Update IOTA full node failed: %d\n", ret);
      }
    }

    // Broadcast buffered transactions
    if (ret == SC_OK) {
      ret = broadcast_buffered_txn(core);
      if (ret) {
        ta_log_error("Broadcast buffered transactions failed. %s\n", ta_error_to_string(ret));
      }
    }

    if (ret == SC_OK) {
      ret = broadcast_buffered_send_mam_request(core);
      if (ret) {
        ta_log_error("Broadcast buffered send MAM message requests failed. %s\n", ta_error_to_string(ret));
      }
    }

    // The usage exceeds the maximum redis capacity
    while (core->cache.capacity < cache_occupied_space()) {
      if (pthread_rwlock_trywrlock(core->cache.rwlock)) {
        ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
        break;
      }

      ret = cache_list_pop(core->cache.complete_list_name, uuid);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
      }
      ret = cache_del(uuid);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
      }

      if (pthread_rwlock_unlock(core->cache.rwlock)) {
        ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
      }
    }

    sleep(core->ta_conf.health_track_period);
  }
  return ((void*)NULL);
}
