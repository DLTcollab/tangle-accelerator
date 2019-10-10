/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include <getopt.h>
#include "scylla_api.h"

#define SCYLLA_IMPORTER_LOGGER "SCYLLA_IMPORTER"

static logger_id_t logger_id;
void scylla_importer_logger_init() { logger_id = logger_helper_enable(SCYLLA_IMPORTER_LOGGER, LOGGER_DEBUG, true); }

int scylla_importer_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(logger_id, "%s.\n", SCYLLA_IMPORTER_LOGGER);
    return EXIT_FAILURE;
  }
  return 0;
}

static status_t import_historical_data(CassSession* session, char* file_path) {
  /* The format of each line in dump files is HASH,TRYTES,snapshot_index */
#define BUFFER_SIZE (NUM_FLEX_TRITS_HASH + 1 + NUM_TRYTES_SERIALIZED_TRANSACTION + 12)  // 12 is for snapshot_index
  status_t ret = SC_OK;
  FILE* list_file = NULL;
  FILE* file = NULL;
  char input_buffer[BUFFER_SIZE];
  char file_name_buffer[256];
  scylla_iota_transaction_t* transaction;

  int buf_idx;
  if (input_buffer == NULL) {
    ta_log_error("%s\n", "SC_STORAGE_OOM");
    ret = SC_STORAGE_OOM;
    goto exit;
  }
  if (new_scylla_iota_transaction(&transaction) != SC_OK) {
    ta_log_error("%s\n", "SC_STORAGE_OOM");
    ret = SC_STORAGE_OOM;
    goto exit;
  }
  if ((list_file = fopen(file_path, "r")) == NULL) {
    /* The specified configuration file does not exist */
    ret = SC_STORAGE_OPEN_ERROR;
    ta_log_error("%s\n", "SC_STORAGE_OPEN_ERROR");
    goto exit;
  }

  while (fgets(file_name_buffer, 256, list_file) != NULL) {
    int name_len = strlen(file_name_buffer);
    if (name_len > 0) {
      file_name_buffer[name_len - 1] = 0;
    }

    if ((file = fopen(file_name_buffer, "r")) == NULL) {
      /* The specified configuration file does not exist */
      ret = SC_STORAGE_OPEN_ERROR;
      ta_log_error("open file %s fail\n", file_name_buffer);
      goto exit;
    }

    ta_log_notice("%s %s\n", "starting import file : ", file_name_buffer);

    while (fgets(input_buffer, BUFFER_SIZE, file) != NULL) {
      if (input_buffer[strlen(input_buffer) - 1] != '\n') {
        ret = SC_STORAGE_INVAILD_INPUT;
        ta_log_error("%s\n", "historical dump file format error");
        goto exit;
      }

      buf_idx = 0;
      set_transaction_hash(transaction, (cass_byte_t*)(input_buffer + buf_idx), NUM_FLEX_TRITS_HASH);
      buf_idx += NUM_FLEX_TRITS_HASH + 1;
      set_transaction_message(transaction, (cass_byte_t*)(input_buffer + buf_idx), NUM_FLEX_TRITS_MESSAGE);
      buf_idx += NUM_FLEX_TRITS_MESSAGE;
      set_transaction_address(transaction, (cass_byte_t*)(input_buffer + buf_idx), NUM_FLEX_TRITS_ADDRESS);
      buf_idx += NUM_FLEX_TRITS_ADDRESS;
      /* skip tag */
      buf_idx += NUM_FLEX_TRITS_HASH;
      set_transaction_bundle(transaction, (cass_byte_t*)(input_buffer + buf_idx), NUM_FLEX_TRITS_BUNDLE);
      buf_idx += NUM_FLEX_TRITS_BUNDLE;
      set_transaction_trunk(transaction, (cass_byte_t*)(input_buffer + buf_idx), NUM_FLEX_TRITS_TRUNK);
      buf_idx += NUM_FLEX_TRITS_TRUNK;
      set_transaction_branch(transaction, (cass_byte_t*)(input_buffer + buf_idx), NUM_FLEX_TRITS_BRANCH);
      set_transaction_timestamp(transaction, 0);
      set_transaction_value(transaction, 0);

      insert_transaction_into_bundleTable(session, transaction, 1);
      insert_transaction_into_edgeTable(session, transaction, 1);
    }
    ta_log_notice("%s %s\n", "successfully import file : ", file_name_buffer);
  }
exit:
  if (ret == SC_OK) {
    ta_log_info("%s %s\n", "successfully import file : ", "file_path");
  }
  return ret;
}

int main(int argc, char* argv[]) {
  CassCluster* cluster = NULL;
  CassSession* session = cass_session_new();
  char* scylla_host = NULL;
  int scylla_port = 0;
  int cmdOpt;
  int optIdx;
  char* file_path = NULL;
  const struct option longOpt[] = {{"scylla_host", required_argument, NULL, 's'},
                                   {"scylla_port", required_argument, NULL, 'p'},
                                   {"file", required_argument, NULL, 'f'},
                                   {NULL, 0, NULL, 0}};

  /* Parse the command line options */
  while (1) {
    cmdOpt = getopt_long(argc, argv, "sf:", longOpt, &optIdx);
    if (cmdOpt == -1) break;

    /* Invalid option */
    if (cmdOpt == '?') break;

    if (cmdOpt == 's') {
      scylla_host = optarg;
    }
    if (cmdOpt == 'p') {
      scylla_port = atoi(optarg);
    }
    if (cmdOpt == 'f') {
      file_path = optarg;
    }
  }
  if (scylla_host == NULL || file_path == NULL) {
    ta_log_error("%s\n", "TA_NULL");
    return 0;
  }
  if (logger_helper_init(LOGGER_INFO) != RC_OK) {
    return 0;
  }
  scylla_api_logger_init();
  scylla_importer_logger_init();

  /* setting Scylla */
  init_scylla(&cluster, session, scylla_host, scylla_port, false, "zmq_table");
  import_historical_data(session, file_path);

  cass_cluster_free(cluster);
  cass_session_free(session);
  scylla_api_logger_release();
  scylla_importer_logger_release();

  return 0;
}
