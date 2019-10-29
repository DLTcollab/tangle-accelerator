/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "map/mode.h"
#include "common/model/transfer.h"
#include "utils/containers/hash/hash_array.h"

#define MODE_LOGGER "mode"

static logger_id_t logger_id;

void map_logger_init() { logger_id = logger_helper_enable(MODE_LOGGER, LOGGER_DEBUG, true); }

int map_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", MODE_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

/***********************************************************************************************************
 * Internal functions
 ***********************************************************************************************************/

static void bundle_transactions_renew(bundle_transactions_t **bundle) {
  bundle_transactions_free(bundle);
  bundle_transactions_new(bundle);
}

static retcode_t map_write_header(mam_api_t *const api, tryte_t const *const channel_id,
                                  tryte_t const *const endpoint_id, mam_psk_t_set_t psks, mam_ntru_pk_t_set_t ntru_pks,
                                  bundle_transactions_t *const bundle, trit_t *const msg_id) {
  retcode_t ret = RC_OK;

  // In TA we only write header on endpoint, since in our architecture we only post Message on signatures owned by
  // endpoint, but not the signatures owned by channel.
  ret = mam_api_bundle_write_header_on_endpoint(api, channel_id, endpoint_id, psks, ntru_pks, bundle, msg_id);
  if (ret) {
    ta_log_error("%d\n", ret);
  }
  return ret;
}

/**
 * Writes a packet on a bundle
 *
 * @param api - The API [in,out]
 * @param bundle - The bundle that the packet will be written into [out]
 * @param payload - The payload to write [in]
 * @param msg_id - The msg_id [in]
 *
 * @return return code
 */
static retcode_t map_write_packet(mam_api_t *const api, bundle_transactions_t *const bundle, char const *const payload,
                                  trit_t const *const msg_id) {
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
 * Find whether the tag of the first transaction of the bundle already has existed on Tangle, which means this current
 * Header has been published.
 *
 * @param tag_array - An tag array contains all the tag under the given Channel ID[in,out]
 * @param bundle - The bundle contains the Header may be used[out]
 *
 * @return Return true is the tags exist in tag array
 */
static bool msg_id_exist(hash81_array_p tag_array, bundle_transactions_t **bundle) {
  bool existed = false;
  flex_trit_t *tag = transaction_tag((iota_transaction_t *)utarray_front(*bundle));
  tryte_t tag_tryte[NUM_TRYTES_TAG + 1], tag_cursor_tryte[NUM_TRYTES_TAG + 1];
  tag_tryte[NUM_TRYTES_TAG] = 0;
  tag_cursor_tryte[NUM_TRYTES_TAG] = 0;

  flex_trits_to_trytes(tag_tryte, NUM_TRYTES_TAG, tag, NUM_FLEX_TRITS_TAG, NUM_FLEX_TRITS_TAG);

  flex_trit_t *tag_cursor = NULL;
  for (tag_cursor = (flex_trit_t *)utarray_front(tag_array); tag_cursor != NULL;
       tag_cursor = (flex_trit_t *)utarray_next(tag_array, tag_cursor)) {
    flex_trits_to_trytes(tag_cursor_tryte, NUM_TRYTES_TAG, tag_cursor, NUM_FLEX_TRITS_TAG, NUM_FLEX_TRITS_TAG);
    if (!memcmp(tag_cursor_tryte, tag_tryte, NUM_TRYTES_TAG)) {
      existed = true;
      break;
    }
  }

  return existed;
}

/**
 * Append Pre-Shared Key or NTRU public key into Pre-Shared Key set or NTRU public key set, respectively.
 *
 * @param psks - Pre-Shared Key set[in]
 * @param ntru_pks - NTRU public key set[in]
 * @param psk - Pre-Shared Key[in]
 * @param ntru_pk - NTRU public key[in]
 *
 * @return return code
 */
static status_t key_set_setup(mam_psk_t_set_t *const psks, mam_ntru_pk_t_set_t *const ntru_pks,
                              tryte_t const *const psk, tryte_t const *const ntru_pk) {
  if (psk) {
    mam_psk_t psk_obj;
    trytes_to_trits(psk, psk_obj.key, MAM_PSK_KEY_SIZE / 3);
    if (mam_psk_t_set_add(psks, &psk_obj) != RC_OK) {
      ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
      return SC_MAM_FAILED_INIT;
    }
  } else if (ntru_pk) {
    mam_ntru_pk_t ntru_pk_obj;
    trytes_to_trits(ntru_pk, ntru_pk_obj.key, MAM_NTRU_PK_SIZE / 3);
    if (mam_ntru_pk_t_set_add(ntru_pks, &ntru_pk_obj) != RC_OK) {
      ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
      return SC_MAM_FAILED_INIT;
    }
  }

  return SC_OK;
}

/**
 * Gets a channel from its id
 *
 * @param api - The API [in]
 * @param channel_id - The channel id [in]
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
 * Gets an endpoint from its id
 *
 * @param api - The API [in]
 * @param channel_id - The associated channel id [in]
 * @param endpoint_id - The endpoint id [in]
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

status_t map_mam_init(mam_api_t *const api, const iota_config_t *const iconf, tryte_t const *const seed,
                      int32_t channel_ord, mam_psk_t_set_t *const psks, mam_ntru_pk_t_set_t *const ntru_pks,
                      tryte_t const *const psk, tryte_t const *const ntru_pk) {
  status_t ret = SC_OK;
  if (!api || (!iconf && !seed)) {
    return SC_MAM_NULL;
  }

  if (seed) {
    if (mam_api_init(api, seed) != RC_OK) {
      ta_log_error("%s\n", "SC_MAM_FAILED_INIT");
      return SC_MAM_FAILED_INIT;
    }
    api->channel_ord = channel_ord;
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

  ret = key_set_setup(psks, ntru_pks, psk, ntru_pk);
  if (ret) {
    ta_log_error("%d\n", ret);
    return ret;
  }

  return SC_OK;
}

status_t map_written_msg_to_bundle(const iota_client_service_t *const service, mam_api_t *const api,
                                   const size_t channel_depth, const size_t endpoint_depth, tryte_t *const chid,
                                   tryte_t *const epid, mam_psk_t_set_t psks, mam_ntru_pk_t_set_t ntru_pks,
                                   char const *const payload, bundle_transactions_t **bundle, tryte_t *msg_id,
                                   uint32_t *ch_remain_sk, uint32_t *ep_remain_sk) {
  status_t ret = SC_OK;
  if (!service || !api || !chid || !epid || channel_depth < 1 || endpoint_depth < 1) {
    ta_log_error("%s\n", "SC_MAM_NULL");
    return SC_MAM_NULL;
  }
  find_transactions_req_t *txn_req = find_transactions_req_new();
  transaction_array_t *obj_res = transaction_array_new();
  hash81_array_p tag_array = hash81_array_new();
  if (mam_api_channel_create(api, channel_depth, chid) != RC_OK) {
    ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
    ta_log_error("%s\n", "SC_MAM_FAILED_CREATE_OR_GET_ID");
    goto done;
  }

  // Step 1. find all Messages ordinal under the current channel.
  // TODO fix NUM_FLEX_TRITS_ADDRESS is 81 not 243
  // TODO use `ta_find_transaction_objects(service, obj_req, obj_res)` instead of the original entangled function
  flex_trit_t chid_flex_trit[NUM_TRITS_ADDRESS];
  flex_trits_from_trytes(chid_flex_trit, NUM_TRITS_ADDRESS, chid, NUM_TRYTES_ADDRESS, NUM_TRYTES_ADDRESS);
  hash243_queue_push(&txn_req->addresses, chid_flex_trit);
  retcode_t ret_rc = RC_OK;
  ret_rc = iota_client_find_transaction_objects(service, txn_req, obj_res);
  if (ret_rc && ret_rc != RC_NULL_PARAM) {
    ret = SC_MAM_FAILED_DESTROYED;
    ta_log_error("%s\n", "SC_MAM_FAILED_DESTROYED");
    goto done;
  }

  iota_transaction_t *tx = NULL;
  TX_OBJS_FOREACH(obj_res, tx) { hash_array_push(tag_array, transaction_tag(tx)); }

  // Step 2. Calculate the smallest available ep_ord and msg_ord
  uint32_t ch_remain_sk_local = 1 << channel_depth, ep_remain_sk_local = 1 << endpoint_depth;
  trit_t msg_id_trits[MAM_MSG_ID_SIZE];
  while (true) {
    if (ch_remain_sk_local < 1) {
      ret = SC_MAM_ALL_MSS_KEYS_USED;
      ta_log_error("%s\n", "SC_MAM_ALL_MSS_KEYS_USED");
      goto done;
    }

    if (mam_api_endpoint_create(api, endpoint_depth, chid, epid) != RC_OK) {
      ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
      ta_log_error("%s\n", "SC_MAM_FAILED_CREATE_OR_GET_ID");
      goto done;
    }

    ep_remain_sk_local = 1 << endpoint_depth;
    while (ep_remain_sk_local) {
      if (map_write_header(api, chid, epid, psks, ntru_pks, *bundle, msg_id_trits)) {
        ret = SC_MAM_FAILED_WRITE;
        ta_log_error("%s\n", "SC_MAM_FAILED_WRITE");
        goto done;
      }

      if (!msg_id_exist(tag_array, bundle)) {
        bundle_transactions_renew(bundle);
        goto end_loop;
      }

      bundle_transactions_renew(bundle);

      ep_remain_sk_local--;
      mam_mss_next(&mam_api_endpoint_get(api, chid, epid)->mss);
    }

    ch_remain_sk_local--;
    mam_mss_next(&mam_api_channel_get(api, chid)->mss);
  }

end_loop:
  // Writing packet to bundle
  if (map_write_packet(api, *bundle, payload, msg_id_trits)) {
    ret = SC_MAM_FAILED_WRITE;
    ta_log_error("%s\n", "SC_MAM_FAILED_WRITE");
    goto done;
  }

  trits_to_trytes(msg_id_trits, msg_id, MAM_MSG_ID_SIZE);

done:
  *ch_remain_sk = ch_remain_sk_local;
  *ep_remain_sk = ep_remain_sk_local;
  find_transactions_req_free(&txn_req);
  transaction_array_free(obj_res);
  hash_array_free(tag_array);
  return ret;
}

status_t map_announce_next_mss_private_key_to_bundle(mam_api_t *const api, const size_t channel_depth,
                                                     const size_t endpoint_depth, tryte_t *const chid,
                                                     uint32_t *ch_remain_sk, uint32_t *ep_remain_sk,
                                                     mam_psk_t_set_t psks, mam_ntru_pk_t_set_t ntru_pks,
                                                     tryte_t *const chid1, bundle_transactions_t **bundle) {
  status_t ret = SC_OK;
  tryte_t epid1[NUM_TRYTES_ADDRESS];
  trit_t msg_id[MAM_MSG_ID_SIZE];

  if (*ch_remain_sk == 1) {
    // TODO uncomment the following code if we want to send chid1 once we run out all available private keys.
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

  } else if (*ep_remain_sk == 1) {
    // Send announcement when there are more they 2 available endpoint MSS private key existing
    if (mam_api_endpoint_create(api, endpoint_depth, chid, epid1) != RC_OK) {
      ret = SC_MAM_FAILED_CREATE_OR_GET_ID;
      ta_log_error("%s\n", "SC_MAM_FAILED_CREATE_OR_GET_ID");
      goto done;
    }

    if (mam_api_bundle_announce_endpoint(api, chid, epid1, psks, ntru_pks, *bundle, msg_id) != RC_OK) {
      ret = SC_MAM_FAILED_WRITE_HEADER;
      ta_log_error("%s\n", "SC_MAM_FAILED_WRITE_HEADER");
      goto done;
    }
  } else if (*ch_remain_sk == 0) {
    ret = SC_MAM_ALL_MSS_KEYS_USED;
    ta_log_error("%s\n", "SC_MAM_ALL_MSS_KEYS_USED");
    goto done;
  }

done:
  return ret;
}

retcode_t map_api_bundle_read(mam_api_t *const api, bundle_transactions_t *bundle, char **payload_out) {
  retcode_t ret = RC_OK;
  tryte_t *payload_trytes = NULL;
  size_t payload_size = 0;
  bool is_last_packet = false;
  ret = mam_api_bundle_read(api, bundle, &payload_trytes, &payload_size, &is_last_packet);
  if (ret == RC_OK) {
    if (payload_trytes == NULL || payload_size == 0) {
      ret = RC_MAM_MESSAGE_NOT_FOUND;
    } else {
      *payload_out = calloc(payload_size * 2 + 1, sizeof(char));
      if (*payload_out == NULL) {
        ret = RC_OOM;
        goto done;
      }
      trytes_to_ascii(payload_trytes, payload_size, *payload_out);
    }
  }

done:
  free(payload_trytes);
  return ret;
}
