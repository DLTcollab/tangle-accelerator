/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include "scylladb_client.h"
#include "scylladb_utils.h"
#define logger_id scylladb_logger_id

static struct db_keyspace_names_s {
  db_client_usage_t usage;
  const char* name;
} db_keyspace_names[] = {{DB_USAGE_REATTACH, "reattachment"}, {DB_USAGE_CHRONICLE, "db_chronicle"}};
static const int db_keyspace_name_nums = sizeof(db_keyspace_names) / sizeof(struct db_keyspace_names_s);

static const char* get_keyspace_name(db_client_usage_t usage) {
  for (int i = 0; i < db_keyspace_name_nums; i++) {
    if (db_keyspace_names[i].usage == usage) {
      return db_keyspace_names[i].name;
    }
  }
  return NULL;
}

static void print_error(CassFuture* future) {
  const char* message;
  size_t message_length;
  cass_future_error_message(future, &message, &message_length);
  ta_log_error("Error: %.*s\n", (int)message_length, message);
}

static CassCluster* create_cluster(const char* hosts) {
  CassCluster* cluster = cass_cluster_new();
  cass_cluster_set_contact_points(cluster, hosts);
  return cluster;
}

static CassError connect_session(CassSession* session, const CassCluster* cluster, const char* keyspace_name) {
  CassError rc = CASS_OK;
  CassFuture* future;
  if (keyspace_name == NULL) {
    future = cass_session_connect(session, cluster);
  } else {
    future = cass_session_connect_keyspace(session, cluster, keyspace_name);
  }

  cass_future_wait(future);
  rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    print_error(future);
  }
  cass_future_free(future);

  return rc;
}

status_t db_client_service_init(db_client_service_t* service, db_client_usage_t usage) {
  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)\n");
    return SC_TA_NULL;
  }
  if (service->host == NULL) {
    ta_log_error("NULL pointer to ScyllaDB hostname\n");
    return SC_TA_NULL;
  }

  /**< This object is thread-safe. It is best practice to create and reuse a single object per application. */
  service->uuid_gen = cass_uuid_gen_new();
  service->session = cass_session_new();
  service->cluster = create_cluster(service->host);
  const char* keyspace_name = get_keyspace_name(usage);
  if (connect_session(service->session, service->cluster, keyspace_name) != CASS_OK) {
    ta_log_error("connect ScyllaDB cluster with host : %s failed\n", service->host);
    service->enabled = false;
    return SC_STORAGE_CONNECT_FAIL;
  }
  service->enabled = true;
  return SC_OK;
}

status_t db_client_service_free(db_client_service_t* service) {
  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)\n");
    return SC_TA_NULL;
  }
  if (service->uuid_gen != NULL) {
    cass_uuid_gen_free(service->uuid_gen);
  }
  if (service->session != NULL) {
    cass_session_free(service->session);
  }
  if (service->cluster != NULL) {
    cass_cluster_free(service->cluster);
  }
  free(service->host);
  return SC_OK;
}
