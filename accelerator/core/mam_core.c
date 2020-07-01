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
#include "utils/fill_nines.h"
#include "utils/tryte_byte_conv.h"

#define MAM_LOGGER "mam_core"

typedef struct channel_info_s {
  int32_t ch_mss_depth;
  tryte_t *chid;
} channel_info_t;

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

static status_t ta_mam_write_header(mam_api_t *const api, tryte_t const *const channel_id, mam_psk_t_set_t psks,
                                    mam_ntru_pk_t_set_t ntru_pks, bundle_transactions_t *const bundle,
                                    trit_t *const msg_id) {
  retcode_t ret = mam_api_bundle_write_header_on_channel(api, channel_id, psks, ntru_pks, bundle, msg_id);
  if (ret) {
    ta_log_error("retcode: %d, %s\n", ret, error_2_string(ret));
    return SC_MAM_FAILED_WRITE_HEADER;
  }
  return SC_OK;
}

/**
 * @brief Write a packet into a bundle
 *
 * @param api[in] The MAM API object
 * @param payload[in] The payload to write
 * @param[in] msg_id The MAM msg_id of this packet
 * @param bundle[out] The bundle that the packet will be written into
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static status_t ta_mam_write_packet(mam_api_t *const api, char const *const payload, trit_t const *const msg_id,
                                    bundle_transactions_t *const bundle) {
  const bool last_packet = true;
  const mam_msg_checksum_t checksum = MAM_MSG_CHECKSUM_SIG;
  tryte_t *payload_trytes = (tryte_t *)malloc(2 * strlen(payload) * sizeof(tryte_t));

  ascii_to_trytes(payload, payload_trytes);

  retcode_t ret =
      mam_api_bundle_write_packet(api, msg_id, payload_trytes, strlen(payload) * 2, checksum, last_packet, bundle);
  if (ret) {
    ta_log_error("retcode: %d, %s\n", ret, error_2_string(ret));
    free(payload_trytes);
    return SC_MAM_FAILED_WRITE;
  }

  free(payload_trytes);
  return SC_OK;
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

/**
 * @brief Add all the keys in the list of Pre-Shared Key and NTRU public key into corresponding key set.
 *
 * The PSK keys are converting the index into trytes as PSK ID.
 *
 * @param psks[out] Pre-Shared Key set
 * @param ntru_pks[out] NTRU public key set
 * @param[in] psk List of Pre-Shared Key
 * @param ntru_pk[in] List of NTRU public key
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static status_t ta_set_mam_key(mam_encrypt_key_t *const mam_keys, UT_array const *const psk,
                               UT_array const *const ntru_pk, UT_array const *const ntru_sk) {
  status_t ret = SC_OK;
  char **p = NULL;
  if (psk) {
    mam_psk_t psk_obj;
    uint16_t psk_id_cnt = 0;
    while ((p = (char **)utarray_next(psk, p))) {
      tryte_t raw_psk_id[NUM_TRYTES_MAM_PSK_ID_SIZE + 1] = {}, psk_id[NUM_TRYTES_MAM_PSK_ID_SIZE + 1] = {};
      bytes_to_trytes((unsigned char *)&psk_id_cnt, sizeof(psk_id_cnt) / sizeof(char), (char *)raw_psk_id);
      ret = fill_nines((char *)psk_id, (char *)raw_psk_id, NUM_TRYTES_MAM_PSK_ID_SIZE);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        return ret;
      }

      trytes_to_trits((tryte_t *)psk_id, psk_obj.id, NUM_TRYTES_MAM_PSK_ID_SIZE);
      trytes_to_trits((tryte_t *)*p, psk_obj.key, NUM_TRYTES_MAM_PSK_KEY_SIZE);
      if (mam_psk_t_set_add(&mam_keys->psks, &psk_obj) != RC_OK) {
        ret = SC_MAM_FAILED_INIT;
        ta_log_error("%s\n", ta_error_to_string(ret));
        return ret;
      }
    }
  }

  if (ntru_pk) {
    mam_ntru_pk_t ntru_pk_obj;
    while ((p = (char **)utarray_next(ntru_pk, p))) {
      trytes_to_trits((tryte_t *)*p, ntru_pk_obj.key, NUM_TRYTES_MAM_NTRU_PK_SIZE);
      if (mam_ntru_pk_t_set_add(&mam_keys->ntru_pks, &ntru_pk_obj) != RC_OK) {
        ret = SC_MAM_FAILED_INIT;
        ta_log_error("%s\n", ta_error_to_string(ret));
        return ret;
      }
    }
  }

  if (ntru_sk) {
    mam_ntru_sk_t ntru_sk_obj;
    while ((p = (char **)utarray_next(ntru_sk, p))) {
      trytes_to_trits((tryte_t *)*p, ntru_sk_obj.secret_key, NUM_TRYTES_MAM_NTRU_SK_SIZE);
      if (mam_ntru_sk_t_set_add(&mam_keys->ntru_sks, &ntru_sk_obj) != RC_OK) {
        ret = SC_MAM_FAILED_INIT;
        ta_log_error("%s\n", ta_error_to_string(ret));
        return ret;
      }
    }
  }
  return SC_OK;
}

/**
 * @brief Free 'mam_encrypt_key_t' object
 *
 * @param mam_key[in] 'mam_encrypt_key_t' object to be freed
 *
 */
static inline void mam_encrypt_key_free(mam_encrypt_key_t *mam_key) {
  mam_psk_t_set_free(&mam_key->psks);
  mam_ntru_pk_t_set_free(&mam_key->ntru_pks);
  mam_ntru_sk_t_set_free(&mam_key->ntru_sks);
}

/**
 * @brief Initialize a 'mam_api_t' object
 *
 * @param api[in,out] The MAM API object
 * @param[in] iconf IOTA API parameter configurations
 * @param seed[in] Seed of MAM channels. It is an optional choice
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static status_t ta_mam_init(mam_api_t *const api, const iota_config_t *const iconf, tryte_t const *const seed) {
  if (!api || (!iconf && !seed)) {
    return SC_NULL;
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

static status_t create_channel_fetch_all_transactions(const iota_client_service_t *const service, mam_api_t *const api,
                                                      const size_t channel_depth, bool first_iter, tryte_t *const chid,
                                                      hash81_array_p tag_array) {
  status_t ret = SC_OK;
  find_transactions_req_t *txn_req = find_transactions_req_new();
  transaction_array_t *obj_res = transaction_array_new();
  if (!txn_req || !obj_res) {
    ret = SC_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // We have created a channel when we want to get to the given channel at the first loop in
  // `ta_mam_written_msg_to_bundle()`, so we don't need to create a channel once again.
  if (!first_iter) {
    if (mam_api_channel_create(api, channel_depth, chid) != RC_OK) {
      ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
      ta_log_error("%s\n", "SC_MAM_FAILED_CREATE_OR_GET_ID");
      goto done;
    }
  }

  flex_trit_t chid_flex_trit[NUM_TRITS_ADDRESS];
  flex_trits_from_trytes(chid_flex_trit, NUM_TRITS_ADDRESS, chid, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
  hash243_queue_push(&txn_req->addresses, chid_flex_trit);
  // TODO use `ta_find_transaction_objects(service, obj_req, obj_res)` instead of the original 'iota.c' function
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

static bool is_setting_changed(const iota_client_service_t *const service, mam_api_t *const api,
                               const size_t channel_depth, tryte_t *const chid) {
  bool changed = true;
  find_transactions_req_t *txn_req = find_transactions_req_new();
  transaction_array_t *txn_res = transaction_array_new();
  if (!txn_req || !txn_res) {
    ta_log_error("%s\n", ta_error_to_string(SC_OOM));
    goto done;
  }

  if (mam_api_channel_create(api, channel_depth, chid) != RC_OK) {
    ta_log_error("%s\n", ta_error_to_string(SC_MAM_FAILED_CREATE_OR_GET_ID));
    goto done;
  }

  flex_trit_t chid_flex_trit[NUM_TRITS_ADDRESS];
  flex_trits_from_trytes(chid_flex_trit, NUM_TRITS_ADDRESS, chid, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
  hash243_queue_push(&txn_req->addresses, chid_flex_trit);
  // TODO use `ta_find_transaction_objects(service, txn_req, txn_res)` instead of the original 'iota.c' function
  retcode_t ret_rc = iota_client_find_transaction_objects(service, txn_req, txn_res);
  if (ret_rc && ret_rc != RC_NULL_PARAM) {
    ta_log_error("%s\n", ta_error_to_string(SC_MAM_FAILED_DESTROYED));
    goto done;
  }

  changed = (transaction_array_len(txn_res) == 0);

done:
  find_transactions_req_free(&txn_req);
  transaction_array_free(txn_res);
  return changed;
}

/**
 * @brief Write payload to bundle on the smallest secret key.
 *
 * With given channel_depth and endpoint_depth, generate the corresponding Channel ID and Endpoint ID, and write the
 * payload to a bundle. The payload is signed with secret key which has the smallest ordinal number.
 *
 * @param[in] service IOTA node service
 * @param api[in,out] The MAM API object
 * @param channel_depth[in] Depth of channel merkle tree
 * @param mam_key[in] Key object to encrypt MAM mesage
 * @param payload[in] The message that is going to send with MAM
 * @param bundle[out] The bundle contains the Header and Packets of the current Message
 * @param chid[out] Channel ID
 * @param msg_id[out] Unique Message identifier under each channel
 * @param mam_operation[out]
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static status_t ta_mam_written_msg_to_bundle(const iota_client_service_t *const service, mam_api_t *const api,
                                             const channel_info_t *channel_info, mam_encrypt_key_t mam_key,
                                             char const *const payload, bundle_transactions_t **bundle,
                                             tryte_t *const chid, tryte_t *const msg_id,
                                             mam_send_operation_t *mam_operation) {
  status_t ret = SC_OK;
  if (!service || !api || !chid || !msg_id || !channel_info || channel_info->ch_mss_depth < 1) {
    ret = SC_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    return ret;
  }
  trit_t msg_id_trits[MAM_MSG_ID_SIZE];
  hash81_array_p tag_array = hash81_array_new();

  // Get to the assigned beginning channel ID. If the initial setting is different, then tangle-accelerator won't be
  // able to generate same chid.
  if (channel_info->chid) {
    // The current setting hasn't been used on Tangle, so we should take the normal procedure.
    if (is_setting_changed(service, api, channel_info->ch_mss_depth, chid)) {
      goto end_find_starting_chid;
    }

    int cnt = 0;
    while (memcmp(channel_info->chid, chid, NUM_TRYTES_ADDRESS)) {
      if (mam_api_channel_create(api, channel_info->ch_mss_depth, chid) != RC_OK) {
        ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }

      if (cnt++ > 100) {
        ret = SC_MAM_EXCEEDED_CHID_ITER;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }

  end_find_starting_chid:
    ta_log_debug("%s\n", "Finish finding starting chid");
  }

  // If a starting chid is provided, then we don't need to create a new chid in the first iteration of the following
  // loop.
  // FIXME: We should figure out a way to avoid passing 'first_iter_with_given_chid' to
  // 'create_channel_fetch_all_transactions()'
  bool first_iter_with_given_chid = (bool)channel_info->chid;
  for (;;) {
    hash_array_free(tag_array);
    tag_array = hash81_array_new();

    ret = create_channel_fetch_all_transactions(service, api, channel_info->ch_mss_depth, first_iter_with_given_chid,
                                                chid, tag_array);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    first_iter_with_given_chid = false;

    /*
     * Three cases should be considered:
     * 1. Unused key exists in the current channel
     * 2. Unused key exists in the next channel
     * 3. Unused key exists in the channel after the next channel
     */

    // Calculate the smallest available msg_ord
    const int ch_leaf_num = 1 << channel_info->ch_mss_depth;
    int ch_remain_key_num = ch_leaf_num;
    int used_key_num = hash_array_len(tag_array);
    do {
      bundle_transactions_renew(bundle);
      used_key_num--;
      ch_remain_key_num--;

      ret = ta_mam_write_header(api, chid, mam_key.psks, mam_key.ntru_pks, *bundle, msg_id_trits);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }

      if (!msg_id_exist(tag_array, transaction_tag((iota_transaction_t *)utarray_front(*bundle)))) {
        ta_log_debug("%s\n", "Found available msg_id");
        break;
      }

      mam_mss_next(&mam_api_channel_get(api, chid)->mss);
    } while (ch_remain_key_num > 0);

    if (ch_remain_key_num == 1) {
      // Publish announcement, when there is only one mss key available.
      ta_log_debug("%s\n", "Publish announcement for the next channel.");
      *mam_operation = ANNOUNCE_CHID;
      break;
    } else if (ch_remain_key_num == 0 && used_key_num == 0) {
      // Check the next channel, since all the mss key are used in current channel
      continue;
    } else {
      *mam_operation = SEND_MESSAGE;
      break;
    }
  }

  // Writing packet to bundle
  if (ta_mam_write_packet(api, payload, msg_id_trits, *bundle)) {
    ret = SC_MAM_FAILED_WRITE;
    ta_log_error("%s\n", "SC_MAM_FAILED_WRITE");
    goto done;
  }

  trits_to_trytes(msg_id_trits, msg_id, MAM_MSG_ID_SIZE);

done:
  hash_array_free(tag_array);
  return ret;
}

/**
 * @brief Write an announcement to bundle.
 *
 * Write the announcement of the next Channel ID.
 *
 * @param api[in,out] The MAM API object
 * @param channel_depth[in] Depth of channel merkle tree
 * @param[in] chid Channel ID
 * @param mam_key[in] Key object to encrypt MAM mesage
 * @param chid1[in] The next Channel ID
 * @param bundle[out] The bundle which contains the Header and Packets of the current Message
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static status_t ta_mam_write_announce_to_bundle(mam_api_t *const api, const size_t channel_depth, tryte_t *const chid,
                                                mam_encrypt_key_t mam_key, tryte_t *const chid1,
                                                bundle_transactions_t **bundle) {
  status_t ret = SC_OK;
  trit_t msg_id[MAM_MSG_ID_SIZE];
  if (mam_api_channel_create(api, channel_depth, chid1) != RC_OK) {
    ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
    ta_log_error("%s\n", "SC_MAM_FAILED_CREATE_OR_GET_ID");
    goto done;
  }

  if (mam_api_bundle_announce_channel(api, chid, chid1, mam_key.psks, mam_key.ntru_pks, *bundle, msg_id) != RC_OK) {
    ret = SC_MAM_FAILED_WRITE_HEADER;
    ta_log_error("%s\n", "SC_MAM_FAILED_WRITE_HEADER");
    goto done;
  }

done:
  return ret;
}

/**
 * @brief Read the MAM message from a bundle
 *
 * @param api[in] The MAM API object
 * @param bundle[in] The bundle that contains the message
 * @param payload_out[out] The output playload in ascii
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static status_t ta_mam_api_bundle_read(mam_api_t *const api, bundle_transactions_t *bundle, char **payload_out) {
  status_t ret = SC_OK;
  tryte_t *payload_trytes = NULL;
  size_t payload_size = 0;
  bool is_last_packet = false;
  retcode_t rc = mam_api_bundle_read(api, bundle, &payload_trytes, &payload_size, &is_last_packet);
  if (rc == RC_OK) {
    if (payload_trytes == NULL || payload_size == 0) {
      ret = SC_MAM_NO_PAYLOAD;
      ta_log_debug("%s\n", ta_error_to_string(ret));
    } else {
      *payload_out = calloc(payload_size * 2 + 1, sizeof(char));
      if (*payload_out == NULL) {
        ret = SC_OOM;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
      trytes_to_ascii(payload_trytes, payload_size, *payload_out);
    }
  } else {
    ret = SC_MAM_READ_MESSAGE_ERROR;
    ta_log_error("retcode_t: %d, %s\n", rc, error_2_string(rc));
  }

done:
  free(payload_trytes);
  return ret;
}

/***********************************************************************************************************
 * External functions
 ***********************************************************************************************************/

status_t ta_send_mam_message(const ta_config_t *const info, const iota_config_t *const iconf,
                             const iota_client_service_t *const service, ta_send_mam_req_t const *const req,
                             ta_send_mam_res_t *const res) {
  status_t ret = SC_OK;
  mam_api_t mam;
  tryte_t chid[MAM_CHANNEL_ID_TRYTE_SIZE] = {}, msg_id[NUM_TRYTES_MAM_MSG_ID] = {};
  bundle_transactions_t *bundle = NULL;
  send_mam_data_mam_v1_t *data = (send_mam_data_mam_v1_t *)req->data;
  send_mam_key_mam_v1_t *key = (send_mam_key_mam_v1_t *)req->key;
  mam_encrypt_key_t mam_key = {.psks = NULL, .ntru_pks = NULL, .ntru_sks = NULL};
  bool msg_sent = false;

  // Creating MAM API
  ret = ta_mam_init(&mam, iconf, data->seed);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_set_mam_key(&mam_key, key->psk_array, key->ntru_array, NULL);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  mam_send_operation_t mam_operation;
  while (!msg_sent) {
    bundle_transactions_renew(&bundle);
    struct channel_info_s channel_info = {.ch_mss_depth = data->ch_mss_depth, .chid = data->chid};

    // Write both Header and Packet into one single bundle.
    ret = ta_mam_written_msg_to_bundle(service, &mam, &channel_info, mam_key, data->message, &bundle, chid, msg_id,
                                       &mam_operation);
    if (ret == SC_OK) {
      msg_sent = true;
    } else if (ret == SC_MAM_ALL_MSS_KEYS_USED) {
      ta_log_debug("%s\n", ta_error_to_string(ret));
      goto mam_announce;
    } else {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    // Sending bundle
    ret = ta_send_bundle(info, iconf, service, bundle);
    if (ret != SC_OK) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    ret = send_mam_res_set_msg_result(res, chid, msg_id, bundle);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

  mam_announce:
    if (mam_operation == ANNOUNCE_CHID) {
      bundle_transactions_renew(&bundle);
      // Send announcement for the next endpoint (epid1) or next channel (chid1)
      tryte_t chid1[MAM_CHANNEL_ID_TRYTE_SIZE] = {};
      ret = ta_mam_write_announce_to_bundle(&mam, data->ch_mss_depth, chid, mam_key, chid1, &bundle);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }

      ret = ta_send_bundle(info, iconf, service, bundle);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }

      ret = send_mam_res_set_announce(res, chid1, bundle);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
    }
  }

done:
  // Destroy MAM API
  if (ret != SC_MAM_FAILED_INIT) {
    // If `seed` is not assigned, then the local MAM file will be used. Therefore, we need to close the MAM file.
    if (!data->seed && mam_api_save(&mam, iconf->mam_file_path, NULL, 0) != RC_OK) {
      ret = SC_MAM_FILE_SAVE;
      ta_log_error("%s\n", ta_error_to_string(ret));
    }
    if (mam_api_destroy(&mam)) {
      ret = SC_MAM_FAILED_DESTROYED;
      ta_log_error("%s\n", ta_error_to_string(ret));
    }
  }
  bundle_transactions_free(&bundle);
  mam_encrypt_key_free(&mam_key);
  return ret;
}

status_t ta_recv_mam_message(const iota_config_t *const iconf, const iota_client_service_t *const service,
                             ta_recv_mam_req_t *const req, ta_recv_mam_res_t *const res) {
  status_t ret = SC_OK;
  mam_api_t mam;
  bundle_array_t *bundle_array = NULL;
  bundle_array_new(&bundle_array);
  mam_pk_t_set_t init_trusted_ch = NULL;
  recv_mam_data_id_mam_v1_t *data_id = (recv_mam_data_id_mam_v1_t *)req->data_id;
  recv_mam_key_mam_v1_t *key = (recv_mam_key_mam_v1_t *)req->key;
  if (mam_api_init(&mam, (tryte_t *)iconf->seed) != RC_OK) {
    ret = SC_MAM_FAILED_INIT;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  if (data_id->chid) {
    if (mam_api_add_trusted_channel_pk(&mam, (tryte_t *)data_id->chid) != RC_OK) {
      ret = SC_MAM_INVAID_CHID_OR_EPID;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  }

  if (data_id->bundle_hash) {
    bundle_transactions_t *bundle = NULL;
    bundle_transactions_new(&bundle);
    ret = ta_get_bundle(service, (tryte_t *)data_id->bundle_hash, bundle);
    if (ret != SC_OK) {
      bundle_transactions_free(&bundle);
      ta_log_error("%s\n", "Failed to get bundle by bundle hash");
      goto done;
    }
    bundle_array_add(bundle_array, bundle);
    bundle_transactions_free(&bundle);
  } else if (data_id->chid) {
    ret = ta_get_bundles_by_addr(service, (tryte_t *)data_id->chid, bundle_array);
    if (ret != SC_OK) {
      ta_log_error("%s\n", "Failed to get bundle by chid");
      goto done;
    }
  }

  // Add decryption keys
  mam_encrypt_key_t mam_key = {.psks = NULL, .ntru_pks = NULL, .ntru_sks = NULL};
  ret = ta_set_mam_key(&mam_key, key->psk_array, NULL, key->ntru_array);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  mam_psk_t_set_entry_t *curr_psk_p = NULL;
  mam_psk_t_set_entry_t *tmp_psk_p = NULL;
  HASH_ITER(hh, mam_key.psks, curr_psk_p, tmp_psk_p) {
    if (mam_api_add_psk(&mam, &curr_psk_p->value)) {
      ta_log_error("%s\n", "Failed to add PSK keys");
      goto done;
    }
  }

  // Copy the trusted_channel_pks, before fetching the information from MAM.
  mam_pk_t_set_entry_t *curr_entry = NULL;
  mam_pk_t_set_entry_t *tmp_entry = NULL;
  HASH_ITER(hh, mam.trusted_channel_pks, curr_entry, tmp_entry) {
    mam_pk_t_set_add(&init_trusted_ch, &(curr_entry->value));
  }

  bundle_transactions_t *bundle = NULL;
  BUNDLE_ARRAY_FOREACH(bundle_array, bundle) {
    char *payload = NULL;
    status_t read_ret = ta_mam_api_bundle_read(&mam, bundle, &payload);
    if (read_ret != SC_OK && read_ret != SC_MAM_NO_PAYLOAD) {
      // If we read a bundle which contains MAM announcement, then it will return "SC_MAM_NO_PAYLOAD"
      ta_log_error("%s\n", ta_error_to_string(read_ret));
      goto done;
    }

    utarray_push_back(res->payload_array, &payload);
    free(payload);
  }

  // Find the channel ID which was just added
  HASH_ITER(hh, mam.trusted_channel_pks, curr_entry, tmp_entry) {
    if (!mam_pk_t_set_contains(init_trusted_ch, &(curr_entry->value))) {
      trits_to_trytes(curr_entry->value.key, (tryte_t *)res->chid1, NUM_TRITS_ADDRESS);
    }
  }

done:
  // Destroy MAM API
  if (ret != SC_MAM_FAILED_INIT) {
    if (mam_api_save(&mam, iconf->mam_file_path, NULL, 0) != RC_OK) {
      ta_log_error("%s\n", ta_error_to_string(ret));
    }
    if (mam_api_destroy(&mam) != RC_OK) {
      ret = SC_MAM_FAILED_DESTROYED;
      ta_log_error("%s\n", ta_error_to_string(ret));
    }
  }
  bundle_array_free(&bundle_array);
  mam_pk_t_set_free(&init_trusted_ch);
  mam_encrypt_key_free(&mam_key);
  return ret;
}
