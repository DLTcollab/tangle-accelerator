/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include <zmq.h>
#include "accelerator/config.h"
#include "cclient/service.h"
#include "scylla_api.h"

#define SCYLLA_LISTENER_LOGGER "SCYLLA_LISTENER"

static logger_id_t logger_id;
void scylla_listener_logger_init() { logger_id = logger_helper_enable(SCYLLA_LISTENER_LOGGER, LOGGER_DEBUG, true); }

int scylla_listener_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(logger_id, "%s.\n", SCYLLA_LISTENER_LOGGER);
    return EXIT_FAILURE;
  }
  return 0;
}

//  Receive ZMQ string from socket and convert into C string
//  Caller must free returned string. Returns NULL if the context
//  is being terminated.
static char* receive_zmq_string_to_heap(void* socket) {
#define BUFFER_LEN 450
  char buffer[BUFFER_LEN];

  int size = zmq_recv(socket, buffer, BUFFER_LEN - 1, 0);
  if (size == -1) {
    return NULL;
  }
  if (size < BUFFER_LEN) {
    buffer[size] = '\0';
  }

  return strndup(buffer, sizeof(buffer) - 1);
}

static void init_iri_client_service(iota_client_service_t* const serv, char const* const host, uint16_t const port) {
  serv->http.path = "/";
  serv->http.content_type = "application/json";
  serv->http.accept = "application/json";
  serv->http.host = host;
  serv->http.port = port;
  serv->http.api_version = 1;
  serv->http.ca_pem = NULL;
  serv->serializer_type = SR_JSON;
  iota_client_core_init(serv);
}

static status_t insert_tx_objs_into_ScyllaDB(CassSession* session, transaction_array_t* const transactions,
                                             scylla_iota_transaction_t* trans) {
  status_t ret = SC_OK;
  iota_transaction_t* curr_tx = NULL;
  TX_OBJS_FOREACH(transactions, curr_tx) {
    set_transaction_bundle(trans, (cass_byte_t*)curr_tx->essence.bundle, NUM_FLEX_TRITS_BUNDLE);
    set_transaction_address(trans, (cass_byte_t*)curr_tx->essence.address, NUM_FLEX_TRITS_ADDRESS);
    set_transaction_hash(trans, (cass_byte_t*)curr_tx->consensus.hash, NUM_FLEX_TRITS_HASH);
    set_transaction_trunk(trans, (cass_byte_t*)curr_tx->attachment.trunk, NUM_FLEX_TRITS_TRUNK);
    set_transaction_branch(trans, (cass_byte_t*)curr_tx->attachment.branch, NUM_FLEX_TRITS_BRANCH);
    set_transaction_message(trans, (cass_byte_t*)curr_tx->data.signature_or_message, NUM_FLEX_TRITS_MESSAGE);
    set_transaction_value(trans, curr_tx->essence.value);
    set_transaction_timestamp(trans, curr_tx->essence.timestamp);
    if ((ret = insert_transaction_into_bundleTable(session, trans, 1)) != SC_OK) {
      goto exit;
    }
    if ((ret = insert_transaction_into_edgeTable(session, trans, 1)) != SC_OK) {
      goto exit;
    }
  }

exit:
  return ret;
}

int main(int argc, char* argv[]) {
  status_t ret = SC_OK;
  CassCluster* cluster = NULL;
  CassSession* session = cass_session_new();
  char* scylla_host = NULL;
  int scylla_port = 0;
  char* iri_host = IRI_HOST;
  uint16_t iri_port = IRI_PORT;
  char* zmq_server = NULL;
  iota_client_service_t iri_serv;
  retcode_t ret_code;

  scylla_iota_transaction_t* transaction;
  char tx_buffer[NUM_FLEX_TRITS_HASH + 1];
  int cmdOpt;
  int optIdx;
  const struct option longOpt[] = {
      {"scylla_host", required_argument, NULL, 's'}, {"scylla_port", required_argument, NULL, 'x'},
      {"zmq_server", required_argument, NULL, 'z'},  {"iri_host", required_argument, NULL, 'i'},
      {"iri_port", required_argument, NULL, 'p'},    {NULL, 0, NULL, 0}};

  /* Parse the command line options */
  while (1) {
    cmdOpt = getopt_long(argc, argv, "sizpx:", longOpt, &optIdx);
    if (cmdOpt == -1) break;

    /* Invalid option */
    if (cmdOpt == '?') break;

    if (cmdOpt == 's') {
      scylla_host = optarg;
    }
    if (cmdOpt == 'x') {
      scylla_port = atoi(optarg);
    }
    if (cmdOpt == 'i') {
      iri_host = optarg;
    }
    if (cmdOpt == 'p') {
      iri_port = atoi(optarg);
    }
    if (cmdOpt == 'z') {
      zmq_server = optarg;
    }
  }
  if (scylla_host == NULL || zmq_server == NULL) {
    ta_log_error("%s\n", "must specify options --scylla_host  --zmq_server");
    return -1;
  }
  if (logger_helper_init(LOGGER_ERR) != RC_OK) {
    return -1;
  }
  scylla_api_logger_init();
  scylla_listener_logger_init();
  if (new_scylla_iota_transaction(&transaction) != SC_OK) {
    ta_log_error("%s\n", "new_scylla_iota_transaction fail");
    return -1;
  }

  /* setting IRI */
  init_iri_client_service(&iri_serv, iri_host, iri_port);

  /* setting Scylla */
  init_scylla(&cluster, session, scylla_host, scylla_port, true, "zmq_table");

  /* setting ZeroMQ */
  void* context = zmq_ctx_new();
  void* subscriber = zmq_socket(context, ZMQ_SUB);
  if (zmq_connect(subscriber, zmq_server) != 0) {
    goto exit;
  }
  if (zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "sn", strlen("sn")) != 0) {
    goto exit;
  }
  /* receiving IRI publication */
  while (1) {
    get_trytes_req_t* tx_req = get_trytes_req_new();
    transaction_array_t* tx_objs = transaction_array_new();
    int milestone_idx;
    char* zmq_receive_string = receive_zmq_string_to_heap(subscriber);

    sscanf(zmq_receive_string, "sn %d %s", &milestone_idx, tx_buffer);
    if ((ret_code = hash243_queue_push(&tx_req->hashes, (flex_trit_t const* const)tx_buffer)) != RC_OK) {
      ret = SC_STORAGE_OOM;
      ta_log_error("%s\n", "SC_STORAGE_OOM");
      goto loop_end;
    }

    if ((ret_code = iota_client_get_transaction_objects(&iri_serv, tx_req, tx_objs)) != RC_OK) {
      ret = SC_STORAGE_SYNC_ERROR;
      ta_log_error("%s\n", "SC_STORAGE_SYNC_ERROR");
      goto loop_end;
    }

    if ((ret = insert_tx_objs_into_ScyllaDB(session, tx_objs, transaction)) != SC_OK) {
      ta_log_alert("%s\n", "insert transaction fail\n");
      goto loop_end;
    }

  loop_end:
    free(zmq_receive_string);
    get_trytes_req_free(&tx_req);
    transaction_array_free(tx_objs);
    if (ret != SC_OK) {
      goto exit;
    }
  }

exit:
  free_scylla_iota_transaction(&transaction);
  zmq_close(subscriber);
  zmq_ctx_destroy(context);
  cass_cluster_free(cluster);
  cass_session_free(session);
  scylla_api_logger_release();
  scylla_listener_logger_release();
  if (ret != SC_OK) {
    return -1;
  }
  return 0;
}
