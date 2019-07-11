/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "map/mode.h"
#include "common/trinary/tryte_ascii.h"
#include "mam/mam/mam_channel_t_set.h"

retcode_t map_channel_create(mam_api_t *const api, tryte_t *const channel_id, const size_t depth) {
  if (mam_channel_t_set_size(api->channels) == 0) {
    mam_api_channel_create(api, depth, channel_id);
  } else {
    mam_channel_t *channel = &api->channels->value;
    trits_to_trytes(trits_begin(mam_channel_id(channel)), channel_id, NUM_TRITS_ADDRESS);
  }

  return RC_OK;
}

retcode_t map_announce_channel(mam_api_t *const api, tryte_t const *const channel_id,
                               bundle_transactions_t *const bundle, trit_t *const msg_id,
                               tryte_t *const new_channel_id) {
  retcode_t ret = RC_OK;

  ERR_BIND_RETURN(mam_api_channel_create(api, MSS_DEPTH, new_channel_id), ret);

  ret = mam_api_bundle_announce_channel(api, channel_id, new_channel_id, NULL, NULL, bundle, msg_id);

  return ret;
}

retcode_t map_announce_endpoint(mam_api_t *const api, tryte_t const *const channel_id,
                                bundle_transactions_t *const bundle, trit_t *const msg_id,
                                tryte_t *const new_endpoint_id) {
  retcode_t ret = RC_OK;

  ERR_BIND_RETURN(mam_api_endpoint_create(api, MSS_DEPTH, channel_id, new_endpoint_id), ret);

  ret = mam_api_bundle_announce_endpoint(api, channel_id, new_endpoint_id, NULL, NULL, bundle, msg_id);

  return ret;
}

retcode_t map_write_header_on_channel(mam_api_t *const api, tryte_t const *const channel_id,
                                      bundle_transactions_t *const bundle, trit_t *const msg_id) {
  retcode_t ret = RC_OK;

  ret = mam_api_bundle_write_header_on_channel(api, channel_id, NULL, NULL, bundle, msg_id);

  return ret;
}

retcode_t map_write_header_on_endpoint(mam_api_t *const api, tryte_t const *const channel_id,
                                       tryte_t const *const endpoint_id, bundle_transactions_t *const bundle,
                                       trit_t *const msg_id) {
  retcode_t ret = RC_OK;

  mam_api_bundle_write_header_on_endpoint(api, channel_id, endpoint_id, NULL, NULL, bundle, msg_id);

  return ret;
}

retcode_t map_write_packet(mam_api_t *const api, bundle_transactions_t *const bundle, char const *const payload,
                           trit_t const *const msg_id, bool is_last_packet) {
  retcode_t ret = RC_OK;
  tryte_t *payload_trytes = (tryte_t *)malloc(2 * strlen(payload) * sizeof(tryte_t));

  ascii_to_trytes(payload, payload_trytes);
  ret = mam_api_bundle_write_packet(api, msg_id, payload_trytes, strlen(payload) * 2, 0, is_last_packet, bundle);

  free(payload_trytes);
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
