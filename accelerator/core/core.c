/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "core.h"
#include <sys/time.h>

#define CC_LOGGER "core"

static logger_id_t logger_id;

void cc_logger_init() { logger_id = logger_helper_enable(CC_LOGGER, LOGGER_DEBUG, true); }

int cc_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", CC_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t ta_attach_to_tangle(const attach_to_tangle_req_t* const req, attach_to_tangle_res_t* res) {
  status_t ret = SC_OK;
  bundle_transactions_t* bundle = NULL;
  iota_transaction_t tx;
  flex_trit_t* elt = NULL;

  // create bundle
  bundle_transactions_new(&bundle);
  HASH_ARRAY_FOREACH(req->trytes, elt) {
    transaction_deserialize_from_trits(&tx, elt, true);
    bundle_transactions_add(bundle, &tx);
  }

  // PoW to bundle
  ret = ta_pow(bundle, req->trunk, req->branch, req->mwm);
  if (ret) {
    goto done;
  }

  // bundle to trytes
  iota_transaction_t* tx_iter = NULL;
  BUNDLE_FOREACH(bundle, tx_iter) {
    flex_trit_t* tx_trytes = transaction_serialize(tx_iter);
    if (tx_trytes) {
      hash_array_push(res->trytes, tx_trytes);
      free(tx_trytes);
      tx_trytes = NULL;
    } else {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
  }

done:
  bundle_transactions_free(&bundle);
  return ret;
}

status_t ta_send_trytes(const ta_config_t* const info, const iota_config_t* const iconf,
                        const iota_client_service_t* const service, hash8019_array_p trytes) {
  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* tx_approve_req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* tx_approve_res = get_transactions_to_approve_res_new();
  attach_to_tangle_req_t* attach_req = attach_to_tangle_req_new();
  attach_to_tangle_res_t* attach_res = attach_to_tangle_res_new();
  if (!tx_approve_req || !tx_approve_res || !attach_req || !attach_res) {
    ret = SC_CCLIENT_OOM;
    ta_log_error("%s\n", "SC_CCLIENT_OOM");
    goto done;
  }
  if (info->gtta) {
    get_transactions_to_approve_req_set_depth(tx_approve_req, iconf->milestone_depth);
    if (iota_client_get_transactions_to_approve(service, tx_approve_req, tx_approve_res)) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
  }

  // copy trytes to attach_req->trytes
  flex_trit_t* elt = NULL;
  HASH_ARRAY_FOREACH(trytes, elt) { attach_to_tangle_req_trytes_add(attach_req, elt); }
  attach_to_tangle_req_init(attach_req, get_transactions_to_approve_res_trunk(tx_approve_res),
                            get_transactions_to_approve_res_branch(tx_approve_res), iconf->mwm);
  ret = ta_attach_to_tangle(attach_req, attach_res);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // store and broadcast
  if (iota_client_store_and_broadcast(service, (store_transactions_req_t*)attach_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  // set the value of attach_res->trytes as output trytes result
  memcpy(trytes, attach_res->trytes, hash_array_len(attach_res->trytes) * sizeof(flex_trit_t));

done:
  get_transactions_to_approve_req_free(&tx_approve_req);
  get_transactions_to_approve_res_free(&tx_approve_res);
  attach_to_tangle_req_free(&attach_req);
  attach_to_tangle_res_free(&attach_res);
  return ret;
}

static status_t ta_generate_address_helper(const iota_config_t* const iconf, const iota_client_service_t* const service,
                                           ta_generate_address_res_t* res) {
  if (res == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  hash243_queue_t out_address = NULL;
  flex_trit_t seed_trits[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(seed_trits, NUM_TRITS_HASH, (const tryte_t*)iconf->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  address_opt_t opt = {.security = 3, .start = 0, .total = 0};

  ret = iota_client_get_new_address(service, seed_trits, opt, &out_address);

  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
  } else {
    res->addresses = out_address;
  }
  return ret;
}

static status_t ta_generate_address_thread(void* args) {
  ta_generate_address_args_t* arg = (ta_generate_address_args_t*)args;
  return ta_generate_address_helper(arg->iconf, arg->service, arg->res);
}

status_t ta_generate_address(const iota_config_t* const iconf, const iota_client_service_t* const service,
                             ta_generate_address_res_t* res) {
  ta_generate_address_args_t args = {.iconf = iconf, .service = service, .res = res};
  int* rval;
  const struct itimerspec timeout = {.it_interval = {.tv_sec = 0, .tv_nsec = 0},
                                     .it_value = {.tv_sec = 50, .tv_nsec = 0}};

  ta_timer_t* timer_id = ta_timer_start(&timeout, ta_generate_address_thread, (void*)&args);
  if (timer_id != NULL) {
    if (ta_timer_stop(timer_id, (void**)&rval) != SC_OK) {
      ta_log_error("%s\n", "SC_UTILS_TIMER_ERROR");
    }
  } else {
    ta_log_error("%s\n", "SC_TA_OOM");
    return SC_TA_OOM;
  }

  return (status_t)rval;
}

status_t ta_send_transfer(const ta_config_t* const info, const iota_config_t* const iconf,
                          const iota_client_service_t* const service, const ta_cache_t* const cache,
                          const ta_send_transfer_req_t* const req, ta_send_transfer_res_t* res) {
  if (req == NULL || res == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  flex_trit_t* serialized_txn;
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  find_transactions_res_t* find_tx_res = find_transactions_res_new();
  hash8019_array_p raw_tx = hash8019_array_new();
  iota_transaction_t* txn = NULL;
  bundle_transactions_t* out_bundle = NULL;
  bundle_transactions_new(&out_bundle);
  transfer_array_t* transfers = transfer_array_new();
  if (find_tx_req == NULL || find_tx_res == NULL || transfers == NULL) {
    ret = SC_CCLIENT_OOM;
    ta_log_error("%s\n", "SC_CCLIENT_OOM");
    goto done;
  }

  // TODO Maybe we can replace the variable type of message from flex_trit_t to
  // tryte_t
  tryte_t msg_tryte[NUM_TRYTES_SERIALIZED_TRANSACTION];
  flex_trits_to_trytes(msg_tryte, req->msg_len / 3, req->message, req->msg_len, req->msg_len);

  transfer_t transfer = {.value = 0, .timestamp = current_timestamp_ms(), .msg_len = req->msg_len / 3};

  if (transfer_message_set_trytes(&transfer, msg_tryte, transfer.msg_len) != RC_OK) {
    ret = SC_CCLIENT_OOM;
    ta_log_error("%s\n", "SC_CCLIENT_OOM");
    goto done;
  }
  memcpy(transfer.address, hash243_queue_peek(req->address), FLEX_TRIT_SIZE_243);
  memcpy(transfer.tag, hash81_queue_peek(req->tag), FLEX_TRIT_SIZE_81);
  transfer_array_add(transfers, &transfer);

  // TODO we may need args `remainder_address`, `inputs`, `timestampe` in the
  // future and declare `security` field in `iota_config_t`
  flex_trit_t seed[NUM_FLEX_TRITS_ADDRESS];
  flex_trits_from_trytes(seed, NUM_TRITS_HASH, (tryte_t const*)iconf->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  if (iota_client_prepare_transfers(service, seed, 2, transfers, NULL, NULL, false, current_timestamp_ms(),
                                    out_bundle) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  BUNDLE_FOREACH(out_bundle, txn) {
    serialized_txn = transaction_serialize(txn);
    if (serialized_txn == NULL) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
    hash_array_push(raw_tx, serialized_txn);
    free(serialized_txn);
  }

  ret = ta_send_trytes(info, iconf, service, raw_tx);
  if (ret) {
    ta_log_error("Error in ta_send_trytes. Push transaction trytes to buffer.\n");
    res->uuid = (char*)malloc(sizeof(char) * UUID_STR_LEN);
    push_txn_to_buffer(cache, raw_tx, res->uuid);
    goto done;
  }

  txn = (iota_transaction_t*)utarray_front(out_bundle);
  ret = hash243_queue_push(&find_tx_req->bundles, transaction_bundle(txn));
  if (ret) {
    ret = SC_CCLIENT_HASH;
    ta_log_error("%s\n", "SC_CCLIENT_HASH");
    goto done;
  }

  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  res->hash = find_tx_res->hashes;
  find_tx_res->hashes = NULL;

done:
  transfer_message_free(&transfer);
  hash_array_free(raw_tx);
  transfer_array_free(transfers);
  bundle_transactions_free(&out_bundle);
  find_transactions_req_free(&find_tx_req);
  find_transactions_res_free(&find_tx_res);
  return ret;
}

status_t ta_find_transactions_obj_by_tag(const iota_client_service_t* const service,
                                         const find_transactions_req_t* const req, transaction_array_t* res) {
  if (req == NULL || res == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  find_transactions_res_t* txn_res = find_transactions_res_new();
  ta_find_transaction_objects_req_t* obj_req = ta_find_transaction_objects_req_new();
  if (txn_res == NULL || obj_req == NULL) {
    ta_log_error("%s\n", "SC_TA_OOM");
    goto done;
  }

  // get transaction hash
  if (iota_client_find_transactions(service, req, txn_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  hash243_queue_copy(&obj_req->hashes, txn_res->hashes, hash243_queue_count(txn_res->hashes));

  ret = ta_find_transaction_objects(service, obj_req, res);
  if (ret) {
    ta_log_error("%d\n", ret);
    goto done;
  }

done:
  find_transactions_res_free(&txn_res);
  ta_find_transaction_objects_req_free(&obj_req);
  return ret;
}

status_t ta_find_transaction_objects(const iota_client_service_t* const service,
                                     const ta_find_transaction_objects_req_t* const req, transaction_array_t* res) {
  status_t ret = SC_OK;
  flex_trit_t tx_trits[NUM_TRITS_SERIALIZED_TRANSACTION];
  iota_transaction_t* temp = NULL;
  get_trytes_req_t* req_get_trytes = get_trytes_req_new();
  transaction_array_t* uncached_txn_array = transaction_array_new();
  flex_trit_t* temp_txn_trits = NULL;
  if (req == NULL || res == NULL || req_get_trytes == NULL || uncached_txn_array == NULL) {
    ret = SC_TA_NULL;
    ta_log_error("%s\n", "SC_TA_NULL");
    goto done;
  }
  char txn_hash[NUM_TRYTES_HASH + 1] = {0};
  char cache_value[NUM_TRYTES_SERIALIZED_TRANSACTION + 1] = {0};
  txn_hash[NUM_TRYTES_HASH] = '\0';
  cache_value[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';

  // append transaction object which is already cached to transaction_array_t
  // if not, append uncached to request object of `iota_client_find_transaction_objectss`
  hash243_queue_entry_t* q_iter = NULL;
  CDL_FOREACH(req->hashes, q_iter) {
    flex_trits_to_trytes((tryte_t*)txn_hash, NUM_TRYTES_HASH, q_iter->hash, NUM_TRITS_HASH, NUM_TRITS_HASH);

    ret = cache_get(txn_hash, cache_value);
    if (ret == SC_OK) {
      flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)cache_value,
                             NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);

      // deserialize raw data to transaction object
      temp = transaction_deserialize(tx_trits, true);
      if (temp == NULL) {
        ret = SC_CCLIENT_OOM;
        ta_log_error("%s\n", "SC_CCLIENT_OOM");
        goto done;
      }

      transaction_array_push_back(res, temp);
      transaction_free(temp);

      // reset the string `cache_value`
      cache_value[0] = '\0';
    } else {
      if (hash243_queue_push(&req_get_trytes->hashes, q_iter->hash) != RC_OK) {
        ret = SC_CCLIENT_HASH;
        ta_log_error("%s\n", "SC_CCLIENT_HASH");
        goto done;
      }
      ret = SC_OK;
    }
  }

  if (req_get_trytes->hashes != NULL) {
    if (iota_client_get_transaction_objects(service, req_get_trytes, uncached_txn_array) != RC_OK) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
  }

  // append response of `iota_client_find_transaction_objects` into cache
  TX_OBJS_FOREACH(uncached_txn_array, temp) {
    temp_txn_trits = transaction_serialize(temp);
    if (!flex_trits_are_null(temp_txn_trits, FLEX_TRIT_SIZE_8019)) {
      // TODO Save fetched transaction object in a set.

      iota_transaction_t* append_txn = transaction_deserialize(temp_txn_trits, true);
      transaction_array_push_back(res, append_txn);
      transaction_free(append_txn);
    } else {
      ret = SC_CCLIENT_NOT_FOUND;
      ta_log_error("%s\n", "SC_CCLIENT_NOT_FOUND");
      goto done;
    }
    free(temp_txn_trits);
    temp_txn_trits = NULL;
  }

done:
  get_trytes_req_free(&req_get_trytes);
  transaction_array_free(uncached_txn_array);
  free(temp_txn_trits);
  return ret;
}

static int idx_sort(void const* lhs, void const* rhs) {
  iota_transaction_t* _lhs = (iota_transaction_t*)lhs;
  iota_transaction_t* _rhs = (iota_transaction_t*)rhs;

  return (transaction_current_index(_lhs) < transaction_current_index(_rhs))
             ? -1
             : (transaction_current_index(_lhs) > transaction_current_index(_rhs));
}

static void get_first_bundle_from_transactions(transaction_array_t const* transactions,
                                               bundle_transactions_t* const bundle) {
  iota_transaction_t* tail = NULL;
  iota_transaction_t* curr_tx = NULL;
  iota_transaction_t* prev = NULL;

  utarray_sort(transactions, idx_sort);
  tail = (iota_transaction_t*)utarray_eltptr(transactions, 0);
  bundle_transactions_add(bundle, tail);

  prev = tail;
  TX_OBJS_FOREACH(transactions, curr_tx) {
    if (transaction_current_index(curr_tx) == (transaction_current_index(prev) + 1) &&
        (memcmp(transaction_hash(curr_tx), transaction_trunk(prev), FLEX_TRIT_SIZE_243) == 0)) {
      bundle_transactions_add(bundle, curr_tx);
      prev = curr_tx;
    }
  }
}

status_t ta_get_bundle(const iota_client_service_t* const service, tryte_t const* const bundle_hash,
                       bundle_transactions_t* const bundle) {
  status_t ret = SC_OK;
  flex_trit_t bundle_hash_flex[FLEX_TRIT_SIZE_243];
  transaction_array_t* tx_objs = transaction_array_new();
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  if (find_tx_req == NULL) {
    ret = SC_CCLIENT_OOM;
    ta_log_error("%s\n", "SC_CCLIENT_OOM");
    goto done;
  }

  // find transactions by bundle hash
  flex_trits_from_trytes(bundle_hash_flex, NUM_TRITS_BUNDLE, bundle_hash, NUM_TRITS_HASH, NUM_TRYTES_BUNDLE);
  hash243_queue_push(&find_tx_req->bundles, bundle_hash_flex);
  ret = iota_client_find_transaction_objects(service, find_tx_req, tx_objs);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  // retreive only first bundle
  get_first_bundle_from_transactions(tx_objs, bundle);

done:
  transaction_array_free(tx_objs);
  find_transactions_req_free(&find_tx_req);
  return ret;
}

status_t ta_send_bundle(const ta_config_t* const info, const iota_config_t* const iconf,
                        const iota_client_service_t* const service, bundle_transactions_t* const bundle) {
  Kerl kerl;
  kerl_init(&kerl);
  bundle_finalize(bundle, &kerl);
  transaction_array_t* out_tx_objs = transaction_array_new();
  hash8019_array_p raw_trytes = hash8019_array_new();
  iota_transaction_t* curr_tx = NULL;
  flex_trit_t trits_8019[FLEX_TRIT_SIZE_8019];

  BUNDLE_FOREACH(bundle, curr_tx) {
    transaction_serialize_on_flex_trits(curr_tx, trits_8019);
    hash_array_push(raw_trytes, trits_8019);
  }

  ta_send_trytes(info, iconf, service, raw_trytes);

  hash_array_free(raw_trytes);
  transaction_array_free(out_tx_objs);

  return SC_OK;
}

status_t ta_get_bundles_by_addr(const iota_client_service_t* const service, tryte_t const* const addr,
                                bundle_array_t* bundle_array) {
  status_t ret = SC_OK;
  tryte_t bundle_hash[NUM_TRYTES_BUNDLE + 1] = {0};
  find_transactions_req_t* txn_req = find_transactions_req_new();
  find_transactions_res_t* txn_res = find_transactions_res_new();
  ta_find_transaction_objects_req_t* obj_req = ta_find_transaction_objects_req_new();
  transaction_array_t* obj_res = transaction_array_new();

  if (txn_req == NULL || txn_res == NULL || obj_req == NULL || obj_res == NULL) {
    ta_log_error("%s\n", "SC_TA_OOM");
    ret = SC_TA_OOM;
    goto done;
  }

  flex_trit_t addr_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(addr_trits, NUM_TRITS_HASH, (const tryte_t*)addr, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  find_transactions_req_address_add(txn_req, addr_trits);

  if (iota_client_find_transactions(service, txn_req, txn_res) != RC_OK) {
    ta_log_error("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }

  // In case the requested transction hashes is an empty one
  if (hash243_queue_count(txn_res->hashes) > 0) {
    hash243_queue_copy(&obj_req->hashes, txn_res->hashes, hash243_queue_count(txn_res->hashes));
  } else {
    ta_log_error("%s\n", "SC_MAM_NOT_FOUND");
    ret = SC_MAM_NOT_FOUND;
    goto done;
  }

  ret = ta_find_transaction_objects(service, obj_req, obj_res);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  iota_transaction_t* curr_tx = NULL;
  bundle_transactions_t* bundle = NULL;
  hash243_set_t bundle_hash_set = NULL;
  TX_OBJS_FOREACH(obj_res, curr_tx) {
    if (!hash243_set_contains(bundle_hash_set, transaction_bundle(curr_tx))) {
      bundle_transactions_new(&bundle);
      flex_trits_to_trytes(bundle_hash, NUM_TRYTES_BUNDLE, transaction_bundle(curr_tx), NUM_TRITS_BUNDLE,
                           NUM_TRITS_BUNDLE);
      ret = ta_get_bundle(service, bundle_hash, bundle);
      if (ret != SC_OK) {
        ta_log_error("%d\n", ret);
        goto done;
      }

      bundle_array_add(bundle_array, bundle);
      hash243_set_add(&bundle_hash_set, transaction_bundle(curr_tx));
      free(bundle);
    }
  }

done:
  find_transactions_req_free(&txn_req);
  find_transactions_res_free(&txn_res);
  ta_find_transaction_objects_req_free(&obj_req);
  transaction_array_free(obj_res);
  hash243_set_free(&bundle_hash_set);
  return ret;
}

status_t ta_get_iri_status(const iota_client_service_t* const service) {
  status_t ret = SC_OK;
  char const* const getnodeinfo = "{\"command\": \"getNodeInfo\"}";
  char* iri_result = NULL;
  char_buffer_t* res_buff = char_buffer_new();
  char_buffer_t* req_buff = char_buffer_new();
  char_buffer_set(req_buff, getnodeinfo);

  // Check IRI host connection
  retcode_t result = iota_service_query(service, req_buff, res_buff);
  if (result != RC_OK) {
    ta_log_error("%s\n", error_2_string(result));
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }
  str_from_char_buffer(res_buff, &iri_result);

  // Check whether IRI host is at the latest milestone
  int latestMilestoneIndex, latestSolidSubtangleMilestoneIndex;
  ret = get_iri_status_milestone_deserialize(iri_result, &latestMilestoneIndex, &latestSolidSubtangleMilestoneIndex);
  if (ret != SC_OK) {
    ta_log_error("check iri connection failed deserialized. status code: %d\n", ret);
    goto done;
  }

  // The tolerant difference between latestSolidSubtangleMilestoneIndex and latestMilestoneIndex is less equal than 1.
  if ((latestSolidSubtangleMilestoneIndex - latestMilestoneIndex) > 1) {
    ret = SC_CORE_IRI_UNSYNC;
    ta_log_error("%s\n", "SC_CORE_IRI_UNSYNC");
  }

done:
  free(iri_result);
  char_buffer_free(req_buff);
  char_buffer_free(res_buff);

  return ret;
}

status_t ta_update_iri_conneciton(ta_config_t* const ta_conf, iota_client_service_t* const service) {
  status_t ret = SC_OK;
  for (int i = 0; i < MAX_IRI_LIST_ELEMENTS && ta_conf->iota_host_list[i]; i++) {
    // update new IRI host

    service->http.host = ta_conf->iota_host_list[i];
    service->http.port = ta_conf->iota_port_list[i];
    ta_log_info("Try to connect to %s:%d\n", service->http.host, service->http.port);

    // Run from the first one until found a good one.
    if (ta_get_iri_status(service) == SC_OK) {
      ta_log_info("Connect to %s:%d\n", service->http.host, service->http.port);
      goto done;
    }
  }
  if (ret) {
    ta_log_error("All IRI host on priority list failed.\n");
  }

done:
  return ret;
}

status_t push_txn_to_buffer(const ta_cache_t* const cache, hash8019_array_p raw_txn_flex_trit_array, char* uuid) {
  status_t ret = SC_OK;
  if (!uuid) {
    ret = SC_CORE_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  uuid_t binuuid;
  uuid_generate_random(binuuid);
  uuid_unparse(binuuid, uuid);
  if (!uuid[0]) {
    ta_log_error("%s\n", "Failed to generate UUID");
    goto done;
  }

  // TODO We push only one transaction raw trits into list, we need to solve this in future.
  ret = cache_set(uuid, UUID_STR_LEN - 1, hash_array_at(raw_txn_flex_trit_array, 0),
                  NUM_FLEX_TRITS_SERIALIZED_TRANSACTION, cache->timeout);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret =
      cache_list_push(cache->buffer_list_name, strlen(cache->buffer_list_name), uuid, UUID_STR_LEN - 1, cache->timeout);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  return ret;
}

status_t broadcast_buffered_txn(const ta_core_t* const core) {
  status_t ret = SC_OK;
  int uuid_list_len = 0;
  hash8019_array_p txn_trytes_array = hash8019_array_new();

  do {
    char uuid[UUID_STR_LEN];
    flex_trit_t req_txn_flex_trits[NUM_FLEX_TRITS_SERIALIZED_TRANSACTION + 1];

    ret = cache_list_size(core->cache.buffer_list_name, &uuid_list_len);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    ret = cache_list_peek(core->cache.buffer_list_name, UUID_STR_LEN, uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // TODO Now we assume every time we call `cache_get()`, we would get a transaction object. However, in the future,
    // the returned result may be a bunlde.
    ret = cache_get(uuid, (char*)req_txn_flex_trits);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    hash_array_push(txn_trytes_array, req_txn_flex_trits);
    ret = ta_send_trytes(&core->ta_conf, &core->iota_conf, &core->iota_service, txn_trytes_array);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    utarray_done(txn_trytes_array);

    // Pop transaction from buffered list
    ret = cache_list_pop(core->cache.buffer_list_name, (char*)req_txn_flex_trits);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // Transfer the transaction to another list in where we store all the successfully broadcasted transactions.
    ret = cache_list_push(core->cache.done_list_name, strlen(core->cache.done_list_name), uuid, UUID_STR_LEN - 1,
                          core->cache.timeout);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  } while (!uuid_list_len);

done:
  hash_array_free(txn_trytes_array);
  return ret;
}
