#include <microhttpd.h>

#include "accelerator/http.h"
#include "cJSON.h"

typedef struct ta_http_request_s {
  bool valid_content_type;
  char *request;
} ta_http_request_t;

static status_t ta_http_process_request(ta_http_t *const http,
                                        char const *const url,
                                        char const *const payload,
                                        char **const out) {
  // TODO: url parser
  // TODO: TA api callback functions,
  //      includes error response, handling option request
  cJSON *res = cJSON_CreateObject();
  cJSON_AddStringToObject(res, "message", "OK");
  *out = cJSON_PrintUnformatted(res);

  return SC_OK;
}

static int ta_http_header_iter(void *cls, enum MHD_ValueKind kind,
                               const char *key, const char *value) {
  ta_http_request_t *header = cls;

  if (0 == strcmp(MHD_HTTP_HEADER_CONTENT_TYPE, key)) {
    header->valid_content_type = !strcmp("application/json", value);
  }
  return MHD_YES;
}

static int ta_http_handler(void *cls, struct MHD_Connection *connection,
                           const char *url, const char *method,
                           const char *version, const char *upload_data,
                           size_t *upload_data_size, void **ptr) {
  int ret = MHD_NO;
  int is_post = 0, is_options = 0;
  ta_http_t *api = (ta_http_t *)cls;
  ta_http_request_t *http_req = *ptr;
  struct MHD_Response *response = NULL;
  char *response_buf = NULL;

  // Only accept POST, GET, OPTIONS
  if (strncmp(method, MHD_HTTP_METHOD_POST, 4) == 0) {
    is_post = 1;
  } else if (strncmp(method, MHD_HTTP_METHOD_OPTIONS, 7) == 0) {
    is_options = 1;
  } else if (strncmp(method, MHD_HTTP_METHOD_GET, 3) != 0) {
    return MHD_NO;
  }

  // if http_req is NULL, that means it's the first call of the connection
  if (http_req == NULL) {
    http_req = calloc(1, sizeof(ta_http_request_t));
    *ptr = http_req;

    // Only POST request needs to get header information
    if (is_post) {
      MHD_get_connection_values(connection, MHD_HEADER_KIND,
                                ta_http_header_iter, http_req);
    }
    return MHD_YES;
  }

  // Check request header for post request only
  if (is_post && !http_req->valid_content_type) {
    ret = MHD_NO;
    goto cleanup;
  }

  // While upload_data_size > 0 process upload_data
  if (*upload_data_size > 0) {
    if (http_req->request == NULL) {
      http_req->request = (char *)malloc(*upload_data_size);
      strncpy(http_req->request, upload_data, *upload_data_size);
    } else {
      ret = MHD_NO;
      goto cleanup;
    }

    *upload_data_size = 0;
    return MHD_YES;
  }

  if (is_post && (http_req->request == NULL)) {
    // POST but no body, so we skip this request
    ret = MHD_NO;
    goto cleanup;
  }

  /* decide which API function should be called */
  ta_http_process_request(api, url, http_req->request, &response_buf);

  response = MHD_create_response_from_buffer(strlen(response_buf), response_buf,
                                             MHD_RESPMEM_MUST_COPY);
  // Set response header
  MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN,
                          "*");
  if (is_options) {
    // header information for OPTIONS request
    MHD_add_response_header(response, "Access-Control-Allow-Methods",
                            "GET, POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers",
                            "Origin, Content-Type, Accept");
    MHD_add_response_header(response, "Access-Control-Max-Age", "86400");
  } else {
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE,
                            "application/json");
  }

  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);

cleanup:
  free(response_buf);
  if (http_req) {
    if (http_req->request) {
      free(http_req->request);
    }
    free(http_req);
  }
  *ptr = NULL;
  return ret;
}

status_t ta_http_init(ta_http_t *const http, ta_core_t *const core) {
  if (http == NULL) {
    return SC_HTTP_NULL;
  }

  http->core = core;
  http->running = false;
  return SC_OK;
}

status_t ta_http_start(ta_http_t *const http) {
  if (http == NULL) {
    return SC_HTTP_NULL;
  }

  http->daemon = MHD_start_daemon(
      MHD_USE_AUTO_INTERNAL_THREAD | MHD_USE_ERROR_LOG | MHD_USE_DEBUG,
      atoi(http->core->info.port), NULL, NULL, ta_http_handler, http,
      MHD_OPTION_END);
  if (http->daemon == NULL) {
    return SC_HTTP_OOM;
  }
  http->running = true;
  return SC_OK;
}

status_t ta_http_stop(ta_http_t *const http) {
  if (http == NULL) {
    return SC_HTTP_NULL;
  }

  MHD_stop_daemon(http->daemon);
  http->running = false;
  return SC_OK;
}
