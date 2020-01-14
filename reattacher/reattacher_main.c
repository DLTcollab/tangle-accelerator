/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include <getopt.h>
#include <sys/time.h>"
#include "accelerator/config.h"
#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#include "common/model/bundle.h"
#include "storage/ta_storage.h"

#define PRESISTENT_PENDING_SECOND 1800 /**< 30 mins */
#define DELAY_INTERVAL 300             /**< 5 mins */

#define logger_id scylladb_logger_id

static status_t init_iota_client_service(iota_client_service_t* const serv) {
  if (serv == NULL) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  serv->http.path = "/";
  serv->http.content_type = "application/json";
  serv->http.accept = "application/json";
  serv->http.api_version = 1;
  serv->http.ca_pem = NULL;
  serv->serializer_type = SR_JSON;
  if (iota_client_core_init(serv) != RC_OK) {
    ta_log_error("Failed to connect to IRI.\n");
    return SC_TA_OOM;
  }
  return SC_OK;
}

static status_t handle_pending_txn(iota_client_service_t* iota_service, db_client_service_t* db_service,
                                   db_identity_t* obj) {
  status_t ret = SC_OK;
  hash243_queue_t req_txn = NULL;
  get_inclusion_states_res_t* res = get_inclusion_states_res_new();
  flex_trit_t trit_hash[NUM_TRITS_HASH];
  char tryte_hash[NUM_TRYTES_HASH];
  flex_trits_from_trytes(trit_hash, NUM_TRITS_HASH, (const tryte_t*)db_ret_identity_hash(obj), NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);

  hash243_queue_push(&req_txn, trit_hash);

  if (iota_client_get_latest_inclusion(iota_service, req_txn, res) != RC_OK ||
      get_inclusion_states_res_states_count(res) != 1) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("Failed to get inclustion status\n");
    db_show_identity_info(obj);
    goto exit;
  }
  if (get_inclusion_states_res_states_at(res, 0)) {
    // confirmed transaction
    ta_log_info("Find confirmed transaction\n");
    db_set_identity_status(obj, CONFIRMED_TXN);
    ret = db_insert_identity_table(db_service, obj);
    if (ret != SC_OK) {
      ta_log_error("Failed to insert identity table\n");
      db_show_identity_info(obj);
      goto exit;
    }
  } else if (db_ret_identity_time_elapsed(obj) > PRESISTENT_PENDING_SECOND) {
    // reattach
    ta_log_info("Reattach pending transaction\n");
    db_show_identity_info(obj);
    bundle_transactions_t* res_bundle_txn;
    bundle_transactions_new(&res_bundle_txn);

    if (iota_client_replay_bundle(iota_service, trit_hash, MILESTONE_DEPTH, MWM, NULL, res_bundle_txn) != RC_OK) {
      ta_log_error("Failed to reattach to Tangle\n");
      db_show_identity_info(obj);
      ret = SC_CCLIENT_FAILED_RESPONSE;
      goto reattach_done;
    }

    /**
     * < get the second transaction in the bundle,
     *   the first transaction is the original transaction before reattachment
     */
    iota_transaction_t* txn = bundle_at(res_bundle_txn, 1);
    flex_trits_to_trytes((tryte_t*)tryte_hash, NUM_TRYTES_HASH, transaction_hash(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);

    db_set_identity_hash(obj, (cass_byte_t*)tryte_hash, NUM_TRYTES_HASH);
    db_set_identity_timestamp(obj, time(NULL));

    ret = db_insert_identity_table(db_service, obj);
    if (ret != SC_OK) {
      ta_log_error("Failed to insert identity table\n");
      goto exit;
    }

  reattach_done:
    bundle_transactions_free(&res_bundle_txn);
  }

exit:
  hash243_queue_free(&req_txn);
  get_inclusion_states_res_free(&res);

  return ret;
}

int main(int argc, char** argv) {
  int optIdx;
  db_client_service_t db_service;
  iota_client_service_t iota_service;
  db_service.host = strdup("localhost");
  iota_service.http.host = "localhost";
  iota_service.http.port = 14265;

  const struct option longOpt[] = {{"iri_host", required_argument, NULL, 'h'},
                                   {"iri_port", required_argument, NULL, 'p'},
                                   {"db_host", required_argument, NULL, 'd'},
                                   {NULL, 0, NULL, 0}};

  /* Parse the command line options */
  /* TODO: Support macOS since getopt_long() is GNU extension */
  while (1) {
    int cmdOpt = getopt_long(argc, argv, "b:", longOpt, &optIdx);
    if (cmdOpt == -1) break;

    /* Invalid option */
    if (cmdOpt == '?') break;

    if (cmdOpt == 'h') {
      iota_service.http.host = optarg;
    }
    if (cmdOpt == 'p') {
      iota_service.http.port = atoi(optarg);
    }
    if (cmdOpt == 'd') {
      free(db_service.host);
      db_service.host = strdup(optarg);
    }
  }
  if (ta_logger_init() != SC_OK) {
    ta_log_error("logger init fail\n");
    return EXIT_FAILURE;
  }
  scylladb_logger_init();
  if (db_client_service_init(&db_service, DB_USAGE_REATTACH) != SC_OK) {
    ta_log_error("Failed to init db client service\n");
    return EXIT_FAILURE;
  }
  if (init_iota_client_service(&iota_service) != SC_OK) {
    ta_log_error("Failed to init iota client service\n");
    return EXIT_FAILURE;
  }
  while (1) {
    db_identity_array_t* id_array = db_identity_array_new();
    db_get_identity_objs_by_status(&db_service, PENDING_TXN, id_array);
    db_identity_t* itr;
    IDENTITY_TABLE_ARRAY_FOREACH(id_array, itr) {
      if (handle_pending_txn(&iota_service, &db_service, itr) != SC_OK) {
        ta_log_warning("Failed to handle pending transaction\n");
        db_show_identity_info(itr);
      }
    }
    db_identity_array_free(&id_array);
    sleep(DELAY_INTERVAL);
  }

  db_client_service_free(&db_service);
  iota_client_core_destroy(&iota_service);
  scylladb_logger_release();
  return 0;
}
