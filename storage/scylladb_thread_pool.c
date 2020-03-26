#include <stdio.h> /* standard I/O routines                     */
#define __USE_GNU
#include <stdlib.h> /* rand() and srand() functions              */
#include "scylladb_thread_pool.h"
/* number of threads used to service requests */

#define logger_id scylladb_logger_id

status_t db_init_permanode_threadpool(db_permanode_pool_t* pool) {
  if (pthread_mutex_init(&pool->request_mutex, NULL)) {
    ta_log_error("fail to init pthread mutex\n");
    return SC_PTHREAD_ERROR;
  }
  pthread_cond_init(&pool->got_request, NULL);
  pthread_cond_init(&pool->finish_request, NULL);

  pool->num_requests = 0;
  pool->requests = NULL;
  pool->last_request = NULL;
  return SC_OK;
}

status_t db_init_worker_thread_data(db_worker_thread_t* thread_data, db_permanode_pool_t* pool, char* host) {
  status_t ret;
  pthread_mutex_init(&thread_data->thread_mutex, NULL);
  thread_data->pool = pool;
  thread_data->service.host = strdup(host);
  ret = db_client_service_init(&thread_data->service, DB_USAGE_PERMANODE);
  return ret;
}

status_t db_permanode_thpool_add(const tryte_t* hash, const tryte_t* trytes, db_permanode_pool_t* pool) {
  status_t ret = SC_OK;
  int rc;                    /* return code of pthreads functions.  */
  struct request* a_request; /* pointer to newly added request.     */
  rc = pthread_mutex_lock(&pool->request_mutex);
  if (pool->num_requests >= 100) {
    // ta_log_info("request num exceed\n");
    pthread_mutex_unlock(&pool->request_mutex);
    return SC_STORAGE_REQUEST_EXCEEDED;
  }
  rc = pthread_mutex_unlock(&pool->request_mutex);

  /* create structure with new request */
  a_request = (struct request*)malloc(sizeof(struct request));
  if (!a_request) { /* malloc failed?? */
    ta_log_error("db_permanode_thpool_add: out of memory\n");
    return SC_TA_OOM;
  }
  memcpy(a_request->hash, hash, sizeof(a_request->hash));
  memcpy(a_request->trytes, trytes, sizeof(a_request->trytes));

  rc = pthread_mutex_lock(&pool->request_mutex);

  if (pool->num_requests == 0) { /* special case - list is empty */
    pool->requests = a_request;
    pool->last_request = a_request;
  } else {
    pool->last_request->next = a_request;
    pool->last_request = a_request;
  }

  /* increase total number of pending requests by one. */
  pool->num_requests++;

  /* unlock mutex */
  rc = pthread_mutex_unlock(&pool->request_mutex);

  /* signal the condition variable - there's a new request to handle */
  rc = pthread_cond_broadcast(&pool->got_request);

  return ret;
}

static struct request* get_request(db_permanode_pool_t* pool) {
  int rc;                    /* return code of pthreads functions.  */
  struct request* a_request; /* pointer to request.                 */

  /* lock the mutex, to assure exclusive access to the list */
  rc = pthread_mutex_lock(&pool->request_mutex);

  if (pool->num_requests > 0) {
    a_request = pool->requests;
    pool->requests = a_request->next;
    if (pool->requests == NULL) { /* this was the last request on the list */
      pool->last_request = NULL;
    }
    /* decrease the total number of pending requests */
    pool->num_requests--;
  } else { /* requests list is empty */
    a_request = NULL;
  }

  /* unlock mutex */
  rc = pthread_mutex_unlock(&pool->request_mutex);

  /* return the request to the caller. */
  return a_request;
}

static void handle_request(struct request* a_request, db_client_service_t* service) {
  if (a_request) {
    status_t ret;
    int retry_cnt = 0;
    do {
      ret = db_permanode_insert_transaction(service, a_request->hash, a_request->trytes);
      if (ret != SC_OK) {
        ta_log_error("Fail to insert transaction %s\n", a_request->hash);
        retry_cnt++;

        if (retry_cnt >= 10) {
          do {
            char* host = strdup(service->host);
            db_client_service_free(service);
            service->host = host;

          while (db_client_service_init(service, DB_USAGE_PERMANODE) != SC_OK) {
            sleep(10);
          }
          ta_log_info("init db service done\n");
          retry_cnt = 0;
        }
      }
    } while (ret != SC_OK);
  }
}

void* db_permanode_worker_handler(void* data) {
  struct request* a_request; /* pointer to a request.               */
  db_worker_thread_t* thread_data = (db_worker_thread_t*)data;
  pthread_mutex_lock(&thread_data->thread_mutex);

  while (1) {
    a_request = get_request(thread_data->pool);
    if (a_request) { /* got a request - handle it and free it */

      handle_request(a_request, &thread_data->service);
      free(a_request);
      pthread_cond_signal(&thread_data->pool->finish_request);
    } else {
      pthread_cond_wait(&thread_data->pool->got_request, &thread_data->thread_mutex);
    }
  }
  return NULL;
}
