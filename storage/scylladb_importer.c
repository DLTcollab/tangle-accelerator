/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <getopt.h>
#include "ta_storage.h"

#define logger_id scylladb_logger_id

typedef struct {
  pthread_mutex_t thread_mutex;
  db_permanode_pool_t* pool;
  char* file_path;
} db_importer_thread_t;

static status_t init_importer_data(db_importer_thread_t* thread_data, db_permanode_pool_t* pool, char* file_list) {
  status_t ret = SC_OK;
  pthread_mutex_init(&thread_data->thread_mutex, NULL);
  thread_data->pool = pool;
  thread_data->file_path = strdup(file_list);
  return ret;
}

static void* importer_handler(void* data) {
#define BUFFER_SIZE (NUM_FLEX_TRITS_HASH + 1 + NUM_TRYTES_SERIALIZED_TRANSACTION + 12)  // 12 is for snapshot_index

  status_t ret = SC_OK;
  db_importer_thread_t* thread_data = (db_importer_thread_t*)data;
  pthread_mutex_lock(&thread_data->thread_mutex);
  FILE* list_file = NULL;
  char file_name_buffer[256];

  if ((list_file = fopen(thread_data->file_path, "r")) == NULL) {
    /* The specified configuration file does not exist */
    ret = SC_CONF_FOPEN_ERROR;
    ta_log_error("Fail to open file %s\n", thread_data->file_path);
    goto exit;
  }

  while (fgets(file_name_buffer, 256, list_file) != NULL) {
    char input_buffer[BUFFER_SIZE];
    FILE* file = NULL;

    int name_len = strlen(file_name_buffer);
    if (name_len > 0) {
      file_name_buffer[name_len - 1] = 0;
    } else {
      ta_log_warning("Empty file name\n");
      continue;
    }

    if ((file = fopen(file_name_buffer, "r")) == NULL) {
      /* The specified configuration file does not exist */
      ret = SC_CONF_FOPEN_ERROR;
      ta_log_error("open file %s fail\n", file_name_buffer);
      goto exit;
    }
    ta_log_info("%s %s\n", "starting to import file : ", file_name_buffer);
    int cnt = 1;
    int cnt_base1000 = 0;
    while (fgets(input_buffer, BUFFER_SIZE, file) != NULL) {
      if (cnt % 1000 == 0) {
        ta_log_info("Import %d K transactions\n", ++cnt_base1000);
        cnt = 0;
      }
      if (input_buffer[strlen(input_buffer) - 1] != '\n') {
        ret = SC_STORAGE_INVALID_INPUT;
        ta_log_error("%s\n", "historical dump file format error");
        continue;
      }

      do {
        ret = db_permanode_thpool_add((tryte_t*)input_buffer, (tryte_t*)input_buffer + NUM_FLEX_TRITS_HASH + 1,
                                      thread_data->pool);
        if (ret != SC_OK) {
          pthread_cond_wait(&thread_data->pool->finish_request, &thread_data->thread_mutex);
        }
      } while (ret != SC_OK);

      cnt++;
    }

    ta_log_info("Successfully import file : %s\n", file_name_buffer);
  }

exit:
  if (ret == SC_OK) {
    ta_log_info("%s %s\n", "Successfully import file : ", thread_data->file_path);
  } else {
    ta_log_error("Fail to import file : %s\n", thread_data->file_path);
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  int thread_num = 1;
  pthread_t* worker_threads; /* thread's structures   */
  pthread_t importer_thread;
  db_worker_thread_t* worker_data;
  db_importer_thread_t importer_data;
  db_permanode_pool_t pool;

  char* db_host = "localhost";
  char* file_path = NULL;
  const struct option longOpt[] = {{"db_host", required_argument, NULL, 's'},
                                   {"file", required_argument, NULL, 'f'},
                                   {"thread_num", required_argument, NULL, 't'},
                                   {NULL, 0, NULL, 0}};
  /* Parse the command line options */
  while (1) {
    int cmdOpt;
    int optIdx;
    cmdOpt = getopt_long(argc, argv, "sf:", longOpt, &optIdx);
    if (cmdOpt == -1) break;

    /* Invalid option */
    if (cmdOpt == '?') break;

    if (cmdOpt == 's') {
      db_host = optarg;
    }
    if (cmdOpt == 'f') {
      file_path = optarg;
    }
    if (cmdOpt == 't') {
      thread_num = atoi(optarg);
    }
  }
  if (file_path == NULL) {
    ta_log_error("%s\n", "TA_NULL");
    return 0;
  }
  if (ta_logger_init() != SC_OK) {
    ta_log_error("logger init fail\n");
    return EXIT_FAILURE;
  }
  scylladb_logger_init();
  worker_threads = malloc(thread_num * sizeof(pthread_t));
  worker_data = malloc(thread_num * sizeof(db_worker_thread_t));

  db_init_permanode_threadpool(&pool);
  /* create the request-handling threads */
  for (int i = 0; i < thread_num; i++) {
    db_init_worker_thread_data(worker_data + i, &pool, db_host);
    pthread_create(&worker_threads[i], NULL, db_permanode_worker_handler, (void*)&worker_data[i]);
  }
  init_importer_data(&importer_data, &pool, file_path);
  pthread_create(&importer_thread, NULL, (void*)importer_handler, (void*)&importer_data);

  pthread_join(importer_thread, NULL);
  db_permanode_tpool_wait(&pool);
  free(worker_data);
  free(worker_threads);

  scylladb_logger_release();

  return 0;
}
