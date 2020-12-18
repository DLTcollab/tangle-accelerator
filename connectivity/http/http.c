#include <arpa/inet.h>
#include <errno.h>
#include <microhttpd.h>
#include <string.h>
#include <time.h>

#include "connectivity/common.h"
#include "http.h"
#include "utils/macros.h"

#define HTTP_LOGGER "http"
#define MAX_REQUEST_LEN 65536

static logger_id_t logger_id;

typedef struct ta_http_request_s {
  bool valid_content_type;
  char *answer_string;
  uint32_t answer_code;
  char *request;
  size_t request_len;
} ta_http_request_t;

void http_logger_init() { logger_id = logger_helper_enable(HTTP_LOGGER, LOGGER_DEBUG, true); }

int http_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

static status_t ta_get_url_parameter(char const *const url, int index, char **param) {
  if (param == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }
  if (index < 0) {
    ta_log_warning("Index lower than 0, automatically set index to 0 instead.\n");
    index = 0;
  }

  // Copy url for parsing
  int url_len = strlen(url) + 1;
  char *tmp = NULL;
  char url_parse[url_len];
  snprintf(url_parse, url_len, "%s", url);

  // Parse URL with '/' and get parameter
  tmp = strtok(url_parse, "/");
  while (index) {
    --index;
    tmp = strtok(NULL, "/");
  }
  if (tmp == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_HTTP_URL_PARSE_ERROR));
    return SC_HTTP_URL_PARSE_ERROR;
  }

  // Copy found parameter
  int token_len = strlen(tmp);
  *param = (char *)malloc(token_len * sizeof(char));
  if (param == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_OOM));
    return SC_OOM;
  }
  strncpy(*param, tmp, token_len);
  return SC_OK;
}

static inline int process_find_txns_obj_by_tag_request(iota_client_service_t *const iota_service, char const *const url,
                                                       char **const out) {
  status_t ret;
  char *tag = NULL;
  ret = ta_get_url_parameter(url, 1, &tag);
  if (ret == SC_OK) {
    ret = api_find_transactions_obj_by_tag(iota_service, tag, out);
  }
  free(tag);
  return set_response_content(ret, out);
}

static inline int process_find_txns_by_tag_request(iota_client_service_t *const iota_service, char const *const url,
                                                   char **const out) {
  status_t ret;
  char *tag = NULL;
  ret = ta_get_url_parameter(url, 1, &tag);
  if (ret == SC_OK) {
    ret = api_find_transactions_by_tag(iota_service, tag, out);
  }
  free(tag);
  return set_response_content(ret, out);
}

static inline int process_fetch_buffered_request_status(ta_http_t *const http, char const *const url,
                                                        char **const out) {
  status_t ret;
  char *uuid = NULL;
  ret = ta_get_url_parameter(url, 1, &uuid);
  if (ret == SC_OK) {
    ret = api_fetch_buffered_request_status(&http->core->cache, uuid, out);
  }
  free(uuid);
  return set_response_content(ret, out);
}

static inline int process_find_txn_obj_single_request(iota_client_service_t *const iota_service, char const *const url,
                                                      char **const out) {
  status_t ret;
  char *hash = NULL;
  ret = ta_get_url_parameter(url, 1, &hash);
  if (ret == SC_OK) {
    ret = api_find_transaction_object_single(iota_service, hash, out);
  }
  free(hash);
  return set_response_content(ret, out);
}

static inline int process_find_txn_obj_request(iota_client_service_t *const iota_service, char const *const payload,
                                               char **const out) {
  status_t ret;
  ret = api_find_transaction_objects(iota_service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_send_transfer_request(ta_http_t *const http, iota_client_service_t *const iota_service,
                                                char const *const payload, char **const out) {
  status_t ret;
  ret = api_send_transfer(http->core, iota_service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_recv_mam_msg_request(ta_http_t *const http, iota_client_service_t *const iota_service,
                                               char const *const payload, char **const out) {
  status_t ret;
  ret = api_recv_mam_message(&http->core->iota_conf, iota_service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_get_node_status(iota_client_service_t *const iota_service, char **const out) {
  status_t ret;
  ret = api_get_node_status(iota_service, out);
  return set_response_content(ret, out);
}

#ifdef DB_ENABLE
static inline int process_get_identity_info_by_hash_request(ta_http_t *const http, char const *const url,
                                                            char **const out) {
  status_t ret = SC_OK;
  char *hash = NULL;
  ret = ta_get_url_parameter(url, 2, &hash);
  if (ret == SC_OK) {
    ret = api_get_identity_info_by_hash(&http->core->db_service, hash, out);
  }
  free(hash);

  return set_response_content(ret, out);
}

static inline int process_get_identity_info_by_id_request(ta_http_t *const http, char const *const url,
                                                          char **const out) {
  status_t ret;
  char *buf = NULL;
  ret = ta_get_url_parameter(url, 2, &buf);
  if (ret == SC_OK) {
    ret = api_get_identity_info_by_id(&http->core->db_service, buf, out);
  }
  free(buf);
  return set_response_content(ret, out);
}

static inline int process_find_transaction_by_id_request(ta_http_t *const http,
                                                         iota_client_service_t *const iota_service,
                                                         char const *const url, char **const out) {
  status_t ret;
  char *buf = NULL;
  ret = ta_get_url_parameter(url, 2, &buf);
  if (ret == SC_OK) {
    ret = api_find_transactions_by_id(iota_service, &http->core->db_service, buf, out);
  }
  free(buf);
  return set_response_content(ret, out);
}
#endif

static inline int process_send_mam_msg_request(ta_http_t *const http, char const *const payload, char **const out) {
  status_t ret;
  ret = api_send_mam_message(&http->core->cache, payload, out);
  return set_response_content(ret, out);
}

static inline int process_send_trytes_request(ta_http_t *const http, iota_client_service_t *const iota_service,
                                              char const *const payload, char **const out) {
  status_t ret;
  ret = api_send_trytes(&http->core->ta_conf, &http->core->iota_conf, iota_service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_get_ta_info_request(ta_http_t *const http, char **const out) {
  status_t ret;
  ret = api_get_ta_info(&http->core->ta_conf, &http->core->iota_conf, &http->core->cache, out);
  return set_response_content(ret, out);
}

static inline int process_proxy_api_request(ta_http_t *const http, iota_client_service_t *const iota_service,
                                            char const *const payload, char **const out) {
  status_t ret;
  ret = proxy_api_wrapper(&http->core->ta_conf, iota_service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_invalid_path_request(char **const out) {
  cJSON *json_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_obj, "message", "Invalid path");
  *out = cJSON_PrintUnformatted(json_obj);
  cJSON_Delete(json_obj);
  return MHD_HTTP_BAD_REQUEST;
}

static inline int process_method_not_allowed_request(char **const out) {
  cJSON *json_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_obj, "message", "Method not allowed");
  *out = cJSON_PrintUnformatted(json_obj);
  cJSON_Delete(json_obj);
  return MHD_HTTP_METHOD_NOT_ALLOWED;
}

static inline int process_options_request(char **const out) {
  cJSON *json_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_obj, "message", "OPTIONS request");
  *out = cJSON_PrintUnformatted(json_obj);
  cJSON_Delete(json_obj);
  return MHD_HTTP_OK;
}

static int ta_http_process_request(ta_http_t *const http, iota_client_service_t *const iota_service,
                                   char const *const url, char const *const payload, char **const out, int options) {
  if (options) {
    return process_options_request(out);
  }

  if (api_path_matcher(url, "/mam/(recv|send)[/]?") == SC_OK) {
    if (payload == NULL) {
      return process_method_not_allowed_request(out);
    }

    if (api_path_matcher(url, ".*/recv.*") == SC_OK) {
      return process_recv_mam_msg_request(http, iota_service, payload, out);
    } else {
      return process_send_mam_msg_request(http, payload, out);
    }
  } else if (api_path_matcher(
                 url, "/fetch/[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}[/]?") ==
             SC_OK) {
    return process_fetch_buffered_request_status(http, url, out);
  } else if (api_path_matcher(url, "/transaction/[A-Z9]{81}[/]?") == SC_OK) {
    return process_find_txn_obj_single_request(iota_service, url, out);
  } else if (api_path_matcher(url, "/transaction/object[/]?") == SC_OK) {
    if (payload != NULL) {
      return process_find_txn_obj_request(iota_service, payload, out);
    }
    return process_method_not_allowed_request(out);

  } else if (api_path_matcher(url, "/tag/[A-Z9]{1,27}/hashes[/]?") == SC_OK) {
    return process_find_txns_by_tag_request(iota_service, url, out);
  } else if (api_path_matcher(url, "/tag/[A-Z9]{1,27}[/]?") == SC_OK) {
    return process_find_txns_obj_by_tag_request(iota_service, url, out);
  } else if (api_path_matcher(url, "/status[/]?") == SC_OK) {
    return process_get_node_status(iota_service, out);
  }
#ifdef DB_ENABLE
  else if (api_path_matcher(url, "/identity/hash/[A-Z9]{81}[/]?") == SC_OK) {
    return process_get_identity_info_by_hash_request(http, url, out);
  } else if (api_path_matcher(url, "/identity/id/[a-z0-9-]{36}[/]?") == SC_OK) {
    return process_get_identity_info_by_id_request(http, url, out);
  } else if (api_path_matcher(url, "/transaction/id/[a-z0-9-]{36}[/]?") == SC_OK) {
    return process_find_transaction_by_id_request(http, iota_service, url, out);
  }
#endif
  else if (api_path_matcher(url, "/transaction[/]?") == SC_OK) {
    if (payload != NULL) {
      return process_send_transfer_request(http, iota_service, payload, out);
    }
    return process_method_not_allowed_request(out);
  } else if (api_path_matcher(url, "/tryte[/]?") == SC_OK) {
    if (payload != NULL) {
      return process_send_trytes_request(http, iota_service, payload, out);
    }
    return process_method_not_allowed_request(out);
  } else if (api_path_matcher(url, "/info[/]?") == SC_OK) {
    return process_get_ta_info_request(http, out);
  } else if (api_path_matcher(url, "/") == SC_OK) {
    if (payload != NULL) {
      return process_proxy_api_request(http, iota_service, payload, out);
    }
    return process_method_not_allowed_request(out);
  } else {
    ta_log_error("SC_HTTP_URL_NOT_MATCH : %s\n", url);
    return process_invalid_path_request(out);
  }
  return MHD_HTTP_OK;
}

static int ta_http_header_iter(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
  UNUSED(kind);
  ta_http_request_t *header = cls;

  if (0 == strncasecmp(MHD_HTTP_HEADER_CONTENT_TYPE, key, strlen(MHD_HTTP_HEADER_CONTENT_TYPE))) {
    if (api_path_matcher(value, "application/json(;?\\s*charset=(UTF|utf)-8)?") == SC_OK) {
      header->valid_content_type = true;
    } else {
      header->valid_content_type = false;
    }
  }
  return MHD_YES;
}

static int request_log(void *cls, const struct sockaddr *addr, socklen_t addrlen) {
  UNUSED(cls);
  UNUSED(addrlen);
  char buf[30];
  struct sockaddr_in *addr_ip = (struct sockaddr_in *)addr;
  char *ip = inet_ntoa(addr_ip->sin_addr);
  time_t rawtime = time(NULL);
  strftime(buf, 30, "%c", localtime(&rawtime));
  printf("%s  - -  [%s]", ip, buf);
  return MHD_YES;
}

/*
 * Append data from the end of ta_http_request_t.request
 */
static status_t build_request(ta_http_request_t *req, const char *data, size_t size) {
  if (req == NULL || data == NULL) {
    ta_log_error("Illegal NULL pointer\n");
    return SC_NULL;
  }
  if (size == 0) {
    ta_log_error("Illegal data size : 0\n");
    return SC_NULL;
  }
  if (size + req->request_len >= MAX_REQUEST_LEN) {
    req->answer_string = strdup(STR_HTTP_REQUEST_SIZE_EXCEED);
    req->answer_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
    return SC_HTTP_INTERNAL_SERVICE_ERROR;
  }
  char *request = malloc(req->request_len + size + 1);
  if (request == NULL) {
    req->answer_string = strdup(STR_HTTP_INTERNAL_SERVICE_ERROR);
    req->answer_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
    return SC_OOM;
  }
  if (req->request != NULL) {
    memcpy(request, req->request, req->request_len);
  }
  memcpy(request + req->request_len, data, size);
  request[req->request_len + size] = 0;
  free(req->request);
  req->request = request;
  req->request_len += size;

  return SC_OK;
}

static int ta_http_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                           const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  UNUSED(version);
  int ret = MHD_NO;
  int post = 0, options = 0;
  ta_http_t *api = (ta_http_t *)cls;
  ta_http_request_t *http_req = *ptr;
  struct MHD_Response *response = NULL;
  ta_log_debug("url = %s, method = %s version = %s upload_data = %s upload_data_size = %ld\n", url, method, version,
               upload_data, *upload_data_size);
  // Only accept POST, GET, OPTIONS
  if (strncmp(method, MHD_HTTP_METHOD_POST, 4) == 0) {
    post = 1;
  } else if (strncmp(method, MHD_HTTP_METHOD_OPTIONS, 7) == 0) {
    options = 1;
  } else if (strncmp(method, MHD_HTTP_METHOD_GET, 3) != 0) {
    return MHD_NO;
  }
  // if http_req is NULL, that means it's the first call of the connection
  if (http_req == NULL) {
    http_req = malloc(sizeof(ta_http_request_t));
    http_req->answer_code = MHD_NO;
    http_req->answer_string = NULL;
    http_req->request = NULL;
    http_req->request_len = 0;
    // Only POST request needs to get header information
    if (post) {
      MHD_get_connection_values(connection, MHD_HEADER_KIND, ta_http_header_iter, http_req);
    }
    *ptr = http_req;

    return MHD_YES;
  }

  // Check request header for post request only
  if (post && !http_req->valid_content_type) {
    ret = MHD_NO;
    ta_log_error("%s\n", "MHD_NO");
    goto cleanup;
  }
  // While upload_data_size > 0 process upload_data
  if (*upload_data_size > 0) {
    if (MHD_NO != http_req->answer_code) {
      *upload_data_size = 0;
      return MHD_YES;
    }
    if (build_request(http_req, upload_data, *upload_data_size) != SC_OK) {
      ta_log_error("Failed to build http request\n");
    }
    ta_log_debug("request = %s\n", http_req->request);

    *upload_data_size = 0;
    return MHD_YES;
  }

  if (post && (http_req->request == NULL)) {
    // POST but no body, so we skip this request
    ret = MHD_NO;
    ta_log_error("%s\n", "Received POST without body, skip request");
    goto cleanup;
  }

  if (http_req->answer_code == MHD_NO) {
    /* decide which API function should be called */
    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, api->core->iota_service.http.host, api->core->iota_service.http.port,
                               api->core->iota_service.http.ca_pem);
    http_req->answer_code =
        ta_http_process_request(api, &iota_service, url, http_req->request, &http_req->answer_string, options);
  }
  response = MHD_create_response_from_buffer(http_req->answer_string ? strlen(http_req->answer_string) : 0,
                                             http_req->answer_string, MHD_RESPMEM_MUST_COPY);
  // Set response header
  MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  if (options) {
    // header information for OPTIONS request
    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers",
                            "Origin, Content-Type, Accept, X-IOTA-API-Version");
    MHD_add_response_header(response, "Access-Control-Max-Age", "86400");
  } else {
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
  }
  ret = MHD_queue_response(connection, http_req->answer_code, response);
  MHD_destroy_response(response);

cleanup:
  // Log of incoming request
  printf(" \"%s %s\" %d\n", method, url, http_req->answer_code);

  if (http_req) {
    free(http_req->answer_string);
    free(http_req->request);
    free(http_req);
  }
  *ptr = NULL;
  return ret;
}

status_t ta_http_init(ta_http_t *const http, ta_core_t *const core) {
  if (http == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  http->core = core;
  return SC_OK;
}

status_t ta_http_start(ta_http_t *const http) {
  if (http == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  http->daemon = MHD_start_daemon(MHD_USE_EPOLL_INTERNAL_THREAD | MHD_USE_ERROR_LOG | MHD_USE_DEBUG,
                                  http->core->ta_conf.port, request_log, NULL, ta_http_handler, http,
                                  MHD_OPTION_THREAD_POOL_SIZE, http->core->ta_conf.http_tpool_size, MHD_OPTION_END);
  if (http->daemon == NULL) {
    ta_log_error("%s\n", strerror(errno));
    return SC_OOM;
  }
  return SC_OK;
}

status_t ta_http_stop(ta_http_t *const http) {
  if (http == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  MHD_stop_daemon(http->daemon);
  return SC_OK;
}
