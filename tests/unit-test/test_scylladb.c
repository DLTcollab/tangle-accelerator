/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifdef DB_ENABLE

#include "storage/ta_storage.h"
#include "tests/test_define.h"

#define logger_id scylladb_logger_id

static char* host = "localhost";
static char* keyspace_name;

static iota_transaction_t iota_tx1, iota_tx2;

static struct identity_s {
  char uuid_string[DB_UUID_STRING_LENGTH];
  char* hash;
  int8_t status;
} identities[] = {
    {.hash = TRANSACTION_1, .status = PENDING_TXN},
    {.hash = TRANSACTION_2, .status = INSERTING_TXN},
    {.hash = TRANSACTION_3, .status = CONFIRMED_TXN},
};

int identity_num = sizeof(identities) / sizeof(struct identity_s);

void test_db_get_trytes(db_client_service_t* service) {
  hash8019_queue_t res = NULL;

  db_get_trytes(service, &res, iota_tx1.consensus.hash);
  db_get_trytes(service, &res, iota_tx2.consensus.hash);

  hash8019_queue_t itr = res;
  TEST_ASSERT_EQUAL_MEMORY(TRYTES_2673_1, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_SERIALIZED_TRANSACTION);
  itr = itr->next;
  TEST_ASSERT_EQUAL_MEMORY(TRYTES_2673_2, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_SERIALIZED_TRANSACTION);

  hash8019_queue_free(&res);
  printf("test_db_get_trytes done\n");
}

void test_db_get_approvees(db_client_service_t* service) {
  hash243_queue_t res = NULL;
  db_get_approvee(service, &res, iota_tx1.attachment.trunk);
  db_get_approvee(service, &res, iota_tx1.attachment.branch);
  db_get_approvee(service, &res, iota_tx2.attachment.trunk);
  db_get_approvee(service, &res, iota_tx2.attachment.branch);

  hash243_queue_t itr = res;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx1.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);
  itr = itr->next;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx1.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);
  itr = itr->next;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx2.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);
  itr = itr->next;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx2.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);

  hash243_queue_free(&res);
  printf("test_db_get_approvee done\n");
}

void test_db_get_bundle(db_client_service_t* service) {
  hash243_queue_t res = NULL;

  db_get_transactions_by_bundle(service, &res, iota_tx1.essence.bundle);
  db_get_transactions_by_bundle(service, &res, iota_tx2.essence.bundle);
  hash243_queue_t itr = res;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx1.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);
  itr = itr->next;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx2.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);

  hash243_queue_free(&res);
  printf("test_db_get_bundle done\n");
}

void test_db_get_address(db_client_service_t* service) {
  hash243_queue_t res = NULL;

  db_get_transactions_by_address(service, &res, iota_tx1.essence.address);
  db_get_transactions_by_address(service, &res, iota_tx2.essence.address);
  hash243_queue_t itr = res;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx1.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);
  itr = itr->next;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx2.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);

  hash243_queue_free(&res);
  printf("test_db_get_address done\n");
}

void test_db_get_tag(db_client_service_t* service) {
  hash243_queue_t res = NULL;

  db_get_transactions_by_tag(service, &res, iota_tx1.essence.obsolete_tag);
  db_get_transactions_by_tag(service, &res, iota_tx2.essence.obsolete_tag);
  hash243_queue_t itr = res;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx1.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);
  itr = itr->next;
  TEST_ASSERT_EQUAL_MEMORY(iota_tx2.consensus.hash, itr->hash, sizeof(flex_trit_t) * NUM_FLEX_TRITS_HASH);

  hash243_queue_free(&res);
  printf("test_db_get_tag done\n");
}

void test_chronicle(void) {
  db_client_service_t service;
  size_t tx_deserialize_offset1, tx_deserialize_offset2;
  service.host = strdup(host);
  TEST_ASSERT_EQUAL_INT(db_client_service_init(&service, DB_USAGE_NULL), SC_OK);
  TEST_ASSERT_EQUAL_INT(db_chronicle_keyspace_init(&service, true, keyspace_name), SC_OK);

  db_chronicle_insert_transaction(&service, (const tryte_t*)HASH_OF_TRYTES_1, (const tryte_t*)TRYTES_2673_1);
  db_chronicle_insert_transaction(&service, (const tryte_t*)HASH_OF_TRYTES_2, (const tryte_t*)TRYTES_2673_2);

  tx_deserialize_offset1 = transaction_deserialize_from_trits(&iota_tx1, (const tryte_t*)TRYTES_2673_1, false);
  memcpy(iota_tx1.consensus.hash, HASH_OF_TRYTES_1, NUM_FLEX_TRITS_HASH);
  tx_deserialize_offset2 = transaction_deserialize_from_trits(&iota_tx2, (const tryte_t*)TRYTES_2673_2, false);
  memcpy(iota_tx2.consensus.hash, HASH_OF_TRYTES_2, NUM_FLEX_TRITS_HASH);

  test_db_get_trytes(&service);
  test_db_get_approvees(&service);
  test_db_get_address(&service);
  test_db_get_bundle(&service);
  test_db_get_tag(&service);
  db_client_service_free(&service);
}

void test_db_get_identity_objs_by_status(db_client_service_t* db_client_service) {
  db_identity_array_t* db_identity_array = db_identity_array_new();
  for (int i = 0; i < identity_num; i++) {
    db_get_identity_objs_by_status(db_client_service, identities[i].status, db_identity_array);
  }
  db_identity_t* itr;
  int idx = 0;
  IDENTITY_TABLE_ARRAY_FOREACH(db_identity_array, itr) {
    char uuid_string[DB_UUID_STRING_LENGTH];
    db_get_identity_uuid_string(itr, uuid_string);

    TEST_ASSERT_EQUAL_STRING(uuid_string, identities[idx].uuid_string);
    TEST_ASSERT_EQUAL_MEMORY(db_ret_identity_hash(itr), (cass_byte_t*)identities[idx].hash,
                             sizeof(cass_byte_t) * DB_NUM_TRYTES_HASH);
    idx++;
  }
  db_identity_array_free(&db_identity_array);
}

void test_db_get_identity_objs_by_uuid_string(db_client_service_t* db_client_service) {
  db_identity_array_t* db_identity_array = db_identity_array_new();

  for (int i = 0; i < identity_num; i++) {
    db_get_identity_objs_by_uuid_string(db_client_service, identities[i].uuid_string, db_identity_array);
  }
  db_identity_t* itr;
  int idx = 0;
  IDENTITY_TABLE_ARRAY_FOREACH(db_identity_array, itr) {
    TEST_ASSERT_EQUAL_MEMORY(db_ret_identity_hash(itr), (cass_byte_t*)identities[idx].hash,
                             sizeof(cass_byte_t) * DB_NUM_TRYTES_HASH);
    TEST_ASSERT_EQUAL_INT(db_ret_identity_status(itr), identities[idx].status);
    idx++;
  }
  db_identity_array_free(&db_identity_array);
}

void test_db_get_identity_objs_by_hash(db_client_service_t* db_client_service) {
  db_identity_array_t* db_identity_array = db_identity_array_new();

  for (int i = 0; i < identity_num; i++) {
    db_get_identity_objs_by_hash(db_client_service, (cass_byte_t*)identities[i].hash, db_identity_array);
  }
  db_identity_t* itr;
  int idx = 0;
  IDENTITY_TABLE_ARRAY_FOREACH(db_identity_array, itr) {
    char uuid_string[DB_UUID_STRING_LENGTH];
    db_get_identity_uuid_string(itr, uuid_string);
    TEST_ASSERT_EQUAL_STRING(uuid_string, identities[idx].uuid_string);
    TEST_ASSERT_EQUAL_INT(db_ret_identity_status(itr), identities[idx].status);
    idx++;
  }
  db_identity_array_free(&db_identity_array);
}

void test_db_identity_table(void) {
  db_client_service_t db_client_service;
  db_client_service.host = strdup(host);
  TEST_ASSERT_EQUAL_INT(db_client_service_init(&db_client_service, DB_USAGE_NULL), SC_OK);
  TEST_ASSERT_EQUAL_INT(db_init_identity_keyspace(&db_client_service, true, keyspace_name), SC_OK);
  for (int i = 0; i < identity_num; i++) {
    db_insert_tx_into_identity(&db_client_service, identities[i].hash, identities[i].status, identities[i].uuid_string);
  }
  test_db_get_identity_objs_by_status(&db_client_service);
  test_db_get_identity_objs_by_uuid_string(&db_client_service);
  test_db_get_identity_objs_by_hash(&db_client_service);

  db_client_service_free(&db_client_service);
}
#endif  // DB_ENABLE

int main(int argc, char** argv) {
#ifdef DB_ENABLE
  int cmdOpt;
  int optIdx;
  const struct option longOpt[] = {
      {"db_host", required_argument, NULL, 'h'}, {"keyspace", required_argument, NULL, 'k'}, {NULL, 0, NULL, 0}};

  keyspace_name = "test_scylla";
  /* Parse the command line options */
  /* TODO: Support macOS since getopt_long() is GNU extension */
  while (1) {
    cmdOpt = getopt_long(argc, argv, "b:", longOpt, &optIdx);
    if (cmdOpt == -1) break;

    /* Invalid option */
    if (cmdOpt == '?') continue;

    if (cmdOpt == 'h') {
      host = optarg;
    }
    if (cmdOpt == 'k') {
      keyspace_name = optarg;
    }
  }

  UNITY_BEGIN();
  if (ta_logger_init() != SC_OK) {
    ta_log_error("logger init fail\n");
    return EXIT_FAILURE;
  }
  scylladb_logger_init();
  RUN_TEST(test_db_identity_table);
  RUN_TEST(test_chronicle);
  scylladb_logger_release();
  return UNITY_END();
#else
  return 0;
#endif  // DB_ENABLE
}
