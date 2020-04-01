/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "accelerator/core/mam_core.h"
#include "common/model/transfer.h"
#include "utils/containers/hash/hash_array.h"

#define MAM_LOGGER "mam_core"

static logger_id_t logger_id;

void ta_mam_logger_init() { logger_id = logger_helper_enable(MAM_LOGGER, LOGGER_DEBUG, true); }

int ta_mam_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", MAM_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

/***********************************************************************************************************
 * Internal functions
 ***********************************************************************************************************/

static retcode_t ta_mam_write_header(mam_api_t *const api, tryte_t const *const channel_id, mam_psk_t_set_t psks,
                                     mam_ntru_pk_t_set_t ntru_pks, bundle_transactions_t *const bundle,
                                     trit_t *const msg_id) {
  retcode_t ret = RC_OK;

  // In TA we only write header on endpoint, since in our architecture we only post Message on signatures owned by
  // endpoint, but not the signatures owned by channel.
  ret = mam_api_bundle_write_header_on_channel(api, channel_id, psks, ntru_pks, bundle, msg_id);
  if (ret) {
    ta_log_error("ret: %d, %s\n", ret, error_2_string(ret));
  }
  return ret;
}

/**
 * @brief Writes a packet on a bundle
 *
 * @param api[in] The MAM API object
 * @param bundle[out] The bundle that the packet will be written into
 * @param payload[in] The payload to write
 * @param msg_id[in] The msg_id
 *
 * @return return code
 */
static retcode_t ta_mam_write_packet(mam_api_t *const api, bundle_transactions_t *const bundle,
                                     char const *const payload, trit_t const *const msg_id) {
  retcode_t ret = RC_OK;
  const bool last_packet = true;
  const mam_msg_checksum_t checksum = MAM_MSG_CHECKSUM_SIG;
  tryte_t *payload_trytes = (tryte_t *)malloc(2 * strlen(payload) * sizeof(tryte_t));

  ascii_to_trytes(payload, payload_trytes);

  ret = mam_api_bundle_write_packet(api, msg_id, payload_trytes, strlen(payload) * 2, checksum, last_packet, bundle);

  free(payload_trytes);
  return ret;
}

/**
 * @brief Find whether the tag of the first transaction of the bundle has already existed on Tangle, which means this
 * current Header has been published.
 *
 * @param tag_array[in,out] A tag array contains all the tags under the given Channel ID
 * @param bundle[out] The bundle contains the Header may be used
 *
 * @return Return true is the tags exist in tag array
 */
static bool msg_id_exist(hash81_array_p tag_array, flex_trit_t *tag) {
  bool existed = false;
  tryte_t tag_tryte[NUM_TRYTES_TAG + 1], tag_cursor_tryte[NUM_TRYTES_TAG + 1];
  tag_tryte[NUM_TRYTES_TAG] = 0;
  tag_cursor_tryte[NUM_TRYTES_TAG] = 0;

  flex_trits_to_trytes(tag_tryte, NUM_TRYTES_TAG, tag, NUM_FLEX_TRITS_TAG, NUM_FLEX_TRITS_TAG);

  flex_trit_t *tag_cursor = NULL;
  for (tag_cursor = (flex_trit_t *)utarray_front(tag_array); tag_cursor != NULL;
       tag_cursor = (flex_trit_t *)utarray_next(tag_array, tag_cursor)) {
    flex_trits_to_trytes(tag_cursor_tryte, NUM_TRYTES_TAG, tag_cursor, NUM_FLEX_TRITS_TAG, NUM_FLEX_TRITS_TAG);
    if (!memcmp(tag_cursor_tryte, tag_tryte, NUM_TRYTES_MAM_MSG_ID)) {
      existed = true;
      break;
    }
  }

  return existed;
}

/**
 * @brief Get a channel from its id
 *
 * @param api[in] MAM API instance
 * @param channel_id[in] Channel id
 *
 * @return a pointer to the channel or NULL if not found
 */
static mam_channel_t *mam_api_channel_get(mam_api_t const *const api, tryte_t const *const channel_id) {
  if (api == NULL || channel_id == NULL) {
    return NULL;
  }

  trit_t channel_id_trits[MAM_CHANNEL_ID_TRIT_SIZE];
  mam_channel_t_set_entry_t *entry = NULL, *tmp = NULL;

  trytes_to_trits(channel_id, channel_id_trits, MAM_CHANNEL_ID_TRYTE_SIZE);

  SET_ITER(api->channels, entry, tmp) {
    if (memcmp(trits_begin(mam_channel_id(&entry->value)), channel_id_trits, MAM_CHANNEL_ID_TRIT_SIZE) == 0) {
      return &entry->value;
    }
  }

  return NULL;
}

/**
 * @brief Get an endpoint from its id
 *
 * @param api[in] MAM API instance
 * @param channel_id[in] Associated channel id
 * @param endpoint_id[in] Associated endpoint id
 *
 * @return a pointer to the endpoint or NULL if not found
 */
static mam_endpoint_t *mam_api_endpoint_get(mam_api_t const *const api, tryte_t const *const channel_id,
                                            tryte_t const *const endpoint_id) {
  trit_t endpoint_id_trits[MAM_ENDPOINT_ID_TRIT_SIZE];
  mam_endpoint_t_set_entry_t *entry = NULL, *tmp = NULL;
  mam_channel_t *parent_channel = mam_api_channel_get(api, channel_id);

  if (endpoint_id == NULL || parent_channel == NULL) {
    return NULL;
  }

  trytes_to_trits(endpoint_id, endpoint_id_trits, MAM_ENDPOINT_ID_TRYTE_SIZE);

  SET_ITER(parent_channel->endpoints, entry, tmp) {
    if (memcmp(trits_begin(mam_endpoint_id(&entry->value)), endpoint_id_trits, MAM_ENDPOINT_ID_TRIT_SIZE) == 0) {
      return &entry->value;
    }
  }

  return NULL;
}

/***********************************************************************************************************
 * External functions
 ***********************************************************************************************************/

void bundle_transactions_renew(bundle_transactions_t **bundle) {
  bundle_transactions_free(bundle);
  bundle_transactions_new(bundle);
}

status_t ta_mam_init(mam_api_t *const api, const iota_config_t *const iconf, tryte_t const *const seed) {
  if (!api || (!iconf && !seed)) {
    return SC_MAM_NULL;
  }

  if (seed) {
    if (mam_api_init(api, seed) != RC_OK) {
      ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
      return SC_MAM_FAILED_INIT;
    }
  } else {
    // Use local MAM file on the current machine
    retcode_t rc = mam_api_load(iconf->mam_file_path, api, NULL, 0);
    if (rc == RC_UTILS_FAILED_TO_OPEN_FILE) {
      if (mam_api_init(api, (tryte_t *)iconf->seed) != RC_OK) {
        ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
        return SC_MAM_FAILED_INIT;
      }
    } else if (rc != RC_OK) {
      ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
      return SC_MAM_FAILED_INIT;
    }
  }

  return SC_OK;
}

status_t ta_set_mam_key(mam_psk_t_set_t *const psks, mam_ntru_pk_t_set_t *const ntru_pks, UT_array const *const psk,
                        UT_array const *const ntru_pk) {
  char **p = NULL;
  if (psk) {
    mam_psk_t psk_obj;
    while ((p = (char **)utarray_next(psk, p))) {
      trytes_to_trits((tryte_t *)*p, psk_obj.key, NUM_TRYTES_MAM_PSK_KEY_SIZE);
      if (mam_psk_t_set_add(psks, &psk_obj) != RC_OK) {
        ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
        return SC_MAM_FAILED_INIT;
      }
    }
  }

  if (ntru_pk) {
    mam_ntru_pk_t ntru_pk_obj;
    while ((p = (char **)utarray_next(ntru_pk, p))) {
      trytes_to_trits((tryte_t *)*p, ntru_pk_obj.key, NUM_TRYTES_MAM_NTRU_PK_SIZE);
      if (mam_ntru_pk_t_set_add(ntru_pks, &ntru_pk_obj) != RC_OK) {
        ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
        return SC_MAM_FAILED_INIT;
      }
    }
  }
  return SC_OK;
}

status_t create_channel_fetch_all_transactions(const iota_client_service_t *const service, mam_api_t *const api,
                                               const size_t channel_depth, tryte_t *const chid,
                                               hash81_array_p tag_array) {
  status_t ret = SC_OK;
  find_transactions_req_t *txn_req = find_transactions_req_new();
  transaction_array_t *obj_res = transaction_array_new();

  if (mam_api_channel_create(api, channel_depth, chid) != RC_OK) {
    ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
    ta_log_error("%s\n", "SC_MAM_FAILED_CREATE_OR_GET_ID");
    goto done;
  }

  flex_trit_t chid_flex_trit[NUM_TRITS_ADDRESS];
  flex_trits_from_trytes(chid_flex_trit, NUM_TRITS_ADDRESS, chid, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
  hash243_queue_push(&txn_req->addresses, chid_flex_trit);
  // TODO use `ta_find_transaction_objects(service, obj_req, obj_res)` instead of the original entangled function
  retcode_t ret_rc = iota_client_find_transaction_objects(service, txn_req, obj_res);
  if (ret_rc && ret_rc != RC_NULL_PARAM) {
    ret = SC_MAM_FAILED_DESTROYED;
    ta_log_error("%s\n", "SC_MAM_FAILED_DESTROYED");
    goto done;
  }

  iota_transaction_t *tx = NULL;
  TX_OBJS_FOREACH(obj_res, tx) {
    if (!msg_id_exist(tag_array, transaction_tag(tx))) {
      hash_array_push(tag_array, transaction_tag(tx));
    }
  }

done:
  find_transactions_req_free(&txn_req);
  transaction_array_free(obj_res);
  return ret;
}

status_t ta_mam_written_msg_to_bundle(const iota_client_service_t *const service, mam_api_t *const api,
                                      const size_t channel_depth, tryte_t *const chid, mam_psk_t_set_t psks,
                                      mam_ntru_pk_t_set_t ntru_pks, char const *const payload,
                                      bundle_transactions_t **bundle, tryte_t *msg_id,
                                      mam_mss_key_status_t *key_status) {
  status_t ret = SC_OK;
  if (!service || !api || !chid || channel_depth < 1) {
    ret = SC_MAM_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    return ret;
  }
  trit_t msg_id_trits[MAM_MSG_ID_SIZE];
  hash81_array_p tag_array = hash81_array_new();
  for (;;) {
    hash_array_free(tag_array);
    tag_array = hash81_array_new();

    ret = create_channel_fetch_all_transactions(service, api, channel_depth, chid, tag_array);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    /*
     * Three cases should be considered:
     * 1. Unused key exists in the current channel
     * 2. Unused key exists in the next channel
     * 3. Unused key exists in the channel after the next channel
     */

    // Calculate the smallest available msg_ord
    const int ch_leaf_num = 1 << channel_depth;
    int ch_remain_key_num = ch_leaf_num;
    int used_key_num = hash_array_len(tag_array);
    do {
      bundle_transactions_renew(bundle);
      used_key_num--;
      ch_remain_key_num--;

      if (ta_mam_write_header(api, chid, psks, ntru_pks, *bundle, msg_id_trits)) {
        ret = SC_MAM_FAILED_WRITE;
        ta_log_error("%s\n", "SC_MAM_FAILED_WRITE");
        goto done;
      }

      if (!msg_id_exist(tag_array, transaction_tag((iota_transaction_t *)utarray_front(*bundle)))) {
        ta_log_debug("%s\n", "Found avialable msg_id");
        break;
      }

      mam_mss_next(&mam_api_channel_get(api, chid)->mss);
    } while (ch_remain_key_num > 0);

    if (ch_remain_key_num == 1) {
      // Publish announcement, when there is only one mss key available.
      ta_log_debug("%s\n", "Publish announcement for the next channel.");
      *key_status = ANNOUNCE_CHID;
      break;
    } else if (ch_remain_key_num == 0 && used_key_num == 0) {
      // Check the next channel, since all the mss key are used in current channel
      continue;
    } else {
      *key_status = KEY_AVAILABLE;
      break;
    }
  }

  // Writing packet to bundle
  if (ta_mam_write_packet(api, *bundle, payload, msg_id_trits)) {
    ret = SC_MAM_FAILED_WRITE;
    ta_log_error("%s\n", "SC_MAM_FAILED_WRITE");
    goto done;
  }

  trits_to_trytes(msg_id_trits, msg_id, MAM_MSG_ID_SIZE);

done:
  hash_array_free(tag_array);
  return ret;
}

status_t ta_mam_write_announce_to_bundle(mam_api_t *const api, const size_t channel_depth,
                                         mam_mss_key_status_t key_status, tryte_t *const chid, mam_psk_t_set_t psks,
                                         mam_ntru_pk_t_set_t ntru_pks, tryte_t *const chid1,
                                         bundle_transactions_t **bundle) {
  status_t ret = SC_OK;
  trit_t msg_id[MAM_MSG_ID_SIZE];

  if (key_status == ANNOUNCE_CHID) {
    if (mam_api_channel_create(api, channel_depth, chid1) != RC_OK) {
      ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
      ta_log_error("%s\n", "SC_MAM_FAILED_CREATE_OR_GET_ID");
      goto done;
    }

    if (mam_api_bundle_announce_channel(api, chid, chid1, psks, ntru_pks, *bundle, msg_id) != RC_OK) {
      ret = SC_MAM_FAILED_WRITE_HEADER;
      ta_log_error("%s\n", "SC_MAM_FAILED_WRITE_HEADER");
      goto done;
    }
    tryte_t msg_id_tryte[NUM_TRYTES_MAM_MSG_ID + 1] = {};
    trits_to_trytes(msg_id, msg_id_tryte, NUM_TRYTES_MAM_MSG_ID);
  }

done:
  return ret;
}

status_t ta_mam_api_bundle_read(mam_api_t *const api, bundle_transactions_t *bundle, char **payload_out) {
  status_t ret = SC_OK;
  tryte_t *payload_trytes = NULL;
  size_t payload_size = 0;
  bool is_last_packet = false;
  retcode_t rc = mam_api_bundle_read(api, bundle, &payload_trytes, &payload_size, &is_last_packet);
  if (rc == RC_OK) {
    if (payload_trytes == NULL || payload_size == 0) {
      ret = SC_MAM_NO_PAYLOAD;
      ta_log_error("%s\n", ta_error_to_string(ret));
    } else {
      *payload_out = calloc(payload_size * 2 + 1, sizeof(char));
      if (*payload_out == NULL) {
        ret = SC_TA_OOM;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
      trytes_to_ascii(payload_trytes, payload_size, *payload_out);
    }
  } else {
    ret = SC_MAM_MESSAGE_NOT_FOUND;
    ta_log_error("entangled retcode_t:%d, %s\n", rc, error_2_string(rc));
  }

done:
  free(payload_trytes);
  return ret;
}
