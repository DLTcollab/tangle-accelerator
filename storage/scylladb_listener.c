/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include <getopt.h>
#include <zmq.h>
#include "accelerator/config.h"
#include "cclient/api/extended/extended_api.h"
#include "ta_storage.h"

#define logger_id scylladb_logger_id

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

static void init_iota_client_service(iota_client_service_t* const serv, char const* const host, uint16_t const port) {
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

typedef struct {
  pthread_mutex_t thread_mutex;
  db_permanode_pool_t* pool;
  iota_client_service_t* service;
} db_listener_data_t;

static status_t db_init_listener_data(db_listener_data_t* data, db_permanode_pool_t* pool,
                                      iota_client_service_t* service) {
  status_t ret = SC_OK;
  pthread_mutex_init(&data->thread_mutex, NULL);
  data->pool = pool;
  data->service = service;
  return ret;
}

static void* listener_handler(void* in) {
  db_listener_data_t* data = (db_listener_data_t*)in;
  pthread_mutex_lock(&data->thread_mutex);
  char* zmq_server = malloc(strlen("tcp://") + strlen(data->service->http.host) + strlen(":5556") + 1);
  strncpy(zmq_server, "tcp://", strlen("tcp://"));
  strcat(zmq_server, data->service->http.host);
  strcat(zmq_server, ":5556");
  get_trytes_res_t* tx_res = get_trytes_res_new();
  get_trytes_req_t* tx_req = get_trytes_req_new();
  char tx_buffer[NUM_FLEX_TRITS_SERIALIZED_TRANSACTION + 1];
  ta_log_info("DB listener start with host : %s\n", zmq_server);
  /* setting ZeroMQ */
  void* context = zmq_ctx_new();
  void* subscriber = zmq_socket(context, ZMQ_SUB);
  if (subscriber == NULL) {
    ta_log_error("NULL subscriber\n");
  }
  if (zmq_connect(subscriber, zmq_server) != 0) {
    ta_log_error("Fail to connect zmq server : %s\n", zmq_server);
    goto exit;
  }
#ifdef IRI_SUPPORT_SN_TRYTES
  if (zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "sn_trytes", strlen("sn_trytes")) != 0) {
    goto exit;
  }
#else
  if (zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "sn", strlen("sn")) != 0) {
    ta_log_error("Fail to set setsockopt : sn\n");
    goto exit;
  }
#endif

  /* receiving IRI publication */
  while (1) {
    char* zmq_receive_string = receive_zmq_string_to_heap(subscriber);
#ifdef IRI_SUPPORT_SN_TRYTES
    sscanf(zmq_receive_string, "sn_trytes %s", tx_buffer);
    hash8019_queue_push(&tx_res, tx_buffer);
#else

    int milestone_idx;
    sscanf(zmq_receive_string, "sn %d %s", &milestone_idx, tx_buffer);
    ta_log_info("Get hash %s\n", tx_buffer);
    if (hash243_queue_push(&tx_req->hashes, (flex_trit_t const* const)tx_buffer) != RC_OK) {
      ta_log_error("%s\n", "SC_STORAGE_OOM");
      goto loop_end;
    }
    if (iota_client_get_trytes(data->service, tx_req, tx_res) != RC_OK) {
      ta_log_error("Fail to get trytes from IRI\n");
      ;
    }

#endif
    while (hash8019_queue_empty(tx_res->trytes) == false) {
      while (db_permanode_thpool_add((tryte_t*)tx_req->hashes, (tryte_t*)tx_res->trytes->hash, data->pool) != SC_OK) {
        pthread_cond_wait(&data->pool->finish_request, &data->thread_mutex);
      }
      hash8019_queue_pop(&tx_res->trytes);
      hash243_queue_pop(&tx_req->hashes);
    }

  loop_end:
    free(zmq_receive_string);
  }

exit:
  ta_log_info("Exit db listener\n");
  free(zmq_server);
  zmq_close(subscriber);
  zmq_ctx_destroy(context);
  get_trytes_req_free(&tx_req);
  get_trytes_res_free(&tx_res);
  return NULL;
}

int main(int argc, char* argv[]) {
  iota_client_service_t iota_service;
  char* db_host = "localhost";
  char* iri_host = "localhost";
  uint16_t iri_port = IRI_PORT;
  int thread_num = 1;
  pthread_t* worker_threads; /* thread's structures   */
  pthread_t listener_thread;
  db_worker_thread_t* worker_data;
  db_listener_data_t listener_data;
  db_permanode_pool_t pool;

  const struct option longOpt[] = {{"db_host", required_argument, NULL, 's'},
                                   {"iri_host", required_argument, NULL, 'i'},
                                   {"iri_port", required_argument, NULL, 'p'},
                                   {"thread_num", required_argument, NULL, 't'},
                                   {NULL, 0, NULL, 0}};
  /* Parse the command line options */
  while (1) {
    int cmdOpt;
    int optIdx;
    cmdOpt = getopt_long(argc, argv, "sfpt:", longOpt, &optIdx);
    if (cmdOpt == -1) break;

    /* Invalid option */
    if (cmdOpt == '?') break;

    if (cmdOpt == 's') {
      db_host = optarg;
    }
    if (cmdOpt == 'i') {
      iri_host = optarg;
    }
    if (cmdOpt == 'p') {
      iri_port = atoi(optarg);
    }
    if (cmdOpt == 't') {
      thread_num = atoi(optarg);
    }
  }
  if (ta_logger_init() != SC_OK) {
    ta_log_error("logger init fail\n");
    return EXIT_FAILURE;
  }
  scylladb_logger_init();

  ta_log_info("db_host = %s, iri_host = %s\n", db_host, iri_host);
  init_iota_client_service(&iota_service, iri_host, iri_port);
  worker_threads = malloc(thread_num * sizeof(pthread_t));
  worker_data = malloc(thread_num * sizeof(db_worker_thread_t));

  if (db_init_permanode_threadpool(&pool) != SC_OK) {
    ta_log_error("Fail to init permanode threadpool\n");
    return EXIT_FAILURE;
  }
  /* create the request-handling threads */
  for (int i = 0; i < thread_num; i++) {
    if (db_init_worker_thread_data(worker_data + i, &pool, db_host) != SC_OK) {
      ta_log_error("Fail to init permanode thpool worker\n");
      return EXIT_FAILURE;
    }
    pthread_create(&worker_threads[i], NULL, (void*)db_permanode_worker_handler, (void*)&worker_data[i]);
  }
  db_init_listener_data(&listener_data, &pool, &iota_service);
  pthread_create(&listener_thread, NULL, (void*)listener_handler, (void*)&listener_data);
  pthread_join(listener_thread, NULL);
  free(worker_data);
  free(worker_threads);

  scylladb_logger_release();

  return 0;
}
