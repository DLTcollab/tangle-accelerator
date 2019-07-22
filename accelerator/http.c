#include <arpa/inet.h>
#include <microhttpd.h>
#include <regex.h>
#include <string.h>
#include <time.h>

#include "accelerator/http.h"
#include "cJSON.h"

#define HTTP_LOGGER "http"

static logger_id_t http_logger_id;

typedef struct ta_http_request_s {
  bool valid_content_type;
  char *request;
} ta_http_request_t;

void http_logger_init() { http_logger_id = logger_helper_enable(HTTP_LOGGER, LOGGER_DEBUG, true); }

int http_logger_release() {
  logger_helper_release(http_logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(http_logger_id, "[%s:%s] Destroying logger failed %s.\n", __func__, __LINE__, HTTP_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

static status_t ta_http_url_matcher(char const *const url, char *const regex_rule) {
  if (regex_rule == NULL) {
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_NULL");
    return SC_HTTP_NULL;
  }
  regex_t reg;
  status_t ret = SC_OK;
  int reg_flag = REG_EXTENDED | REG_NOSUB;

  if (regcomp(&reg, regex_rule, reg_flag) != 0) {
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_INVALID_REGEX");
    return SC_HTTP_INVALID_REGEX;
  }
  if (regexec(&reg, url, 0, NULL, 0) != 0) {
    // Did not match pattern
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_URL_NOT_MATCH");
    ret = SC_HTTP_URL_NOT_MATCH;
  }

  regfree(&reg);
  return ret;
}

static status_t ta_get_url_parameter(char const *const url, int index, char **param) {
  if (param == NULL) {
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_NULL");
    return SC_HTTP_NULL;
  }
  if (index < 0) {
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
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_URL_PARSE_ERROR");
    return SC_HTTP_URL_PARSE_ERROR;
  }

  // Copy found parameter
  int token_len = strlen(tmp);
  *param = (char *)malloc(token_len * sizeof(char));
  if (param == NULL) {
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_OOM");
    return SC_HTTP_OOM;
  }
  strncpy(*param, tmp, token_len);
  return SC_OK;
}

static int set_response_content(status_t ret, char **json_result) {
  int http_ret;
  if (ret == SC_OK) {
    return MHD_HTTP_OK;
  }

  cJSON *json_obj = cJSON_CreateObject();
  switch (ret) {
    case SC_CCLIENT_NOT_FOUND:
    case SC_MAM_NOT_FOUND:
      http_ret = MHD_HTTP_NOT_FOUND;
      log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "MHD_HTTP_NOT_FOUND");
      cJSON_AddStringToObject(json_obj, "message", "Request not found");
      break;
    case SC_CCLIENT_JSON_KEY:
    case SC_MAM_NO_PAYLOAD:
      http_ret = MHD_HTTP_BAD_REQUEST;
      log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "MHD_HTTP_BAD_REQUEST");
      cJSON_AddStringToObject(json_obj, "message", "Invalid request header");
      break;
    default:
      http_ret = MHD_HTTP_INTERNAL_SERVER_ERROR;
      log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "MHD_HTTP_INTERNAL_SERVER_ERROR");
      cJSON_AddStringToObject(json_obj, "message", "Internal service error");
      break;
  }
  *json_result = cJSON_PrintUnformatted(json_obj);
  cJSON_Delete(json_obj);
  return http_ret;
}

static inline int process_generate_address_request(ta_http_t *const http, char **const out) {
  status_t ret = SC_OK;
  ret = api_generate_address(&http->core->tangle, &http->core->service, out);
  return set_response_content(ret, out);
}

static inline int process_find_txn_hash_request(ta_http_t *const http, char const *const payload, char **const out) {
  status_t ret = SC_OK;
  ret = api_find_transactions(&http->core->service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_find_txn_obj_request(ta_http_t *const http, char const *const payload, char **const out) {
  status_t ret = SC_OK;
  ret = api_find_transaction_objects(&http->core->service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_get_tips_pair_request(ta_http_t *const http, char **const out) {
  status_t ret = SC_OK;
  ret = api_get_tips_pair(&http->core->tangle, &http->core->service, out);
  return set_response_content(ret, out);
}

static inline int process_get_tips_request(ta_http_t *const http, char **const out) {
  status_t ret = SC_OK;
  ret = api_get_tips(&http->core->service, out);
  return set_response_content(ret, out);
}

static inline int process_send_transfer_request(ta_http_t *const http, char const *const payload, char **const out) {
  status_t ret = SC_OK;
  ret = api_send_transfer(&http->core->tangle, &http->core->service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_recv_mam_msg_request(ta_http_t *const http, char const *const url, char **const out) {
  status_t ret = SC_OK;
  char *bundle = NULL;
  ret = ta_get_url_parameter(url, 1, &bundle);
  if (ret == SC_OK) {
    ret = api_receive_mam_message(&http->core->service, bundle, out);
  }
  return set_response_content(ret, out);
}

static inline int process_mam_send_msg_request(ta_http_t *const http, char const *const payload, char **const out) {
  status_t ret = SC_OK;
  ret = api_mam_send_message(&http->core->tangle, &http->core->service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_send_trytes_request(ta_http_t *const http, char const *const payload, char **const out) {
  status_t ret = SC_OK;
  ret = api_send_trytes(&http->core->tangle, &http->core->service, payload, out);
  return set_response_content(ret, out);
}

static inline int process_invalid_path_request(char **const out) {
  cJSON *json_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_obj, "message", "Invalid path");
  *out = cJSON_PrintUnformatted(json_obj);
  return MHD_HTTP_BAD_REQUEST;
}

static inline int process_method_not_allowed_request(char **const out) {
  cJSON *json_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_obj, "message", "Method not allowed");
  *out = cJSON_PrintUnformatted(json_obj);
  return MHD_HTTP_METHOD_NOT_ALLOWED;
}

static inline int process_options_request(char **const out) {
  cJSON *json_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_obj, "message", "OPTIONS request");
  *out = cJSON_PrintUnformatted(json_obj);
  return MHD_HTTP_OK;
}

static int ta_http_process_request(ta_http_t *const http, char const *const url, char const *const payload,
                                   char **const out, int options) {
  if (options) {
    return process_options_request(out);
  }

  if (ta_http_url_matcher(url, "/address") == SC_OK) {
    return process_generate_address_request(http, out);
  } else if (ta_http_url_matcher(url, "/transaction/hash") == SC_OK) {
    if (payload != NULL) {
      return process_find_txn_hash_request(http, payload, out);
    } else {
      return process_method_not_allowed_request(out);
    }
  } else if (ta_http_url_matcher(url, "/transaction/object") == SC_OK) {
    if (payload != NULL) {
      return process_find_txn_obj_request(http, payload, out);
    } else {
      return process_method_not_allowed_request(out);
    }
  } else if (ta_http_url_matcher(url, "/tips/pair") == SC_OK) {
    return process_get_tips_pair_request(http, out);
  } else if (ta_http_url_matcher(url, "/tips") == SC_OK) {
    return process_get_tips_request(http, out);
  } else if (ta_http_url_matcher(url, "/transaction") == SC_OK) {
    if (payload != NULL) {
      return process_send_transfer_request(http, payload, out);
    } else {
      return process_method_not_allowed_request(out);
    }
  } else if (ta_http_url_matcher(url, "/mam/[A-Z9]{81}") == SC_OK) {
    return process_recv_mam_msg_request(http, url, out);
  } else if (ta_http_url_matcher(url, "/mam") == SC_OK) {
    if (payload != NULL) {
      return process_mam_send_msg_request(http, payload, out);
    } else {
      return process_method_not_allowed_request(out);
    }
  } else if (ta_http_url_matcher(url, "/tryte") == SC_OK) {
    if (payload != NULL) {
      return process_send_trytes_request(http, payload, out);
    } else {
      return process_method_not_allowed_request(out);
    }
  } else {
    return process_invalid_path_request(out);
  }
  return MHD_HTTP_OK;
}

static int ta_http_header_iter(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
  ta_http_request_t *header = cls;

  if (0 == strcmp(MHD_HTTP_HEADER_CONTENT_TYPE, key)) {
    header->valid_content_type = !strcmp("application/json", value);
  }
  return MHD_YES;
}

static int request_log(void *cls, const struct sockaddr *addr, socklen_t addrlen) {
  char buf[30];
  struct sockaddr_in *addr_ip = (struct sockaddr_in *)addr;
  char *ip = inet_ntoa(addr_ip->sin_addr);
  time_t rawtime = time(NULL);
  strftime(buf, 30, "%c", localtime(&rawtime));
  printf("%s  - -  [%s]", ip, buf);
  return MHD_YES;
}

static int ta_http_handler(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                           const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  int ret = MHD_NO, req_ret = MHD_HTTP_OK;
  int post = 0, options = 0;
  ta_http_t *api = (ta_http_t *)cls;
  ta_http_request_t *http_req = *ptr;
  struct MHD_Response *response = NULL;
  char *response_buf = NULL;

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
    http_req = calloc(1, sizeof(ta_http_request_t));
    *ptr = http_req;

    // Only POST request needs to get header information
    if (post) {
      MHD_get_connection_values(connection, MHD_HEADER_KIND, ta_http_header_iter, http_req);
    }
    return MHD_YES;
  }

  // Check request header for post request only
  if (post && !http_req->valid_content_type) {
    ret = MHD_NO;
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "MHD_NO");
    goto cleanup;
  }

  // While upload_data_size > 0 process upload_data
  if (*upload_data_size > 0) {
    if (http_req->request == NULL) {
      http_req->request = (char *)malloc(*upload_data_size);
      strncpy(http_req->request, upload_data, *upload_data_size);
    } else {
      ret = MHD_NO;
      log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "MHD_NO");
      goto cleanup;
    }

    *upload_data_size = 0;
    return MHD_YES;
  }

  if (post && (http_req->request == NULL)) {
    // POST but no body, so we skip this request
    ret = MHD_NO;
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "MHD_NO");
    goto cleanup;
  }

  /* decide which API function should be called */
  req_ret = ta_http_process_request(api, url, http_req->request, &response_buf, options);

  response = MHD_create_response_from_buffer(strlen(response_buf), response_buf, MHD_RESPMEM_MUST_COPY);
  // Set response header
  MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*");
  if (options) {
    // header information for OPTIONS request
    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers", "Origin, Content-Type, Accept");
    MHD_add_response_header(response, "Access-Control-Max-Age", "86400");
  } else {
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
  }

  ret = MHD_queue_response(connection, req_ret, response);
  MHD_destroy_response(response);

cleanup:
  // Log of incoming request
  printf(" \"%s %s\" %d\n", method, url, req_ret);

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
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_NULL");
    return SC_HTTP_NULL;
  }

  http->core = core;
  http->running = false;
  return SC_OK;
}

status_t ta_http_start(ta_http_t *const http) {
  if (http == NULL) {
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_NULL");
    return SC_HTTP_NULL;
  }

  http->daemon =
      MHD_start_daemon(MHD_USE_AUTO_INTERNAL_THREAD | MHD_USE_THREAD_PER_CONNECTION | MHD_USE_ERROR_LOG | MHD_USE_DEBUG,
                       atoi(http->core->info.port), request_log, NULL, ta_http_handler, http, MHD_OPTION_END);
  if (http->daemon == NULL) {
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_OOM");
    return SC_HTTP_OOM;
  }
  http->running = true;
  return SC_OK;
}

status_t ta_http_stop(ta_http_t *const http) {
  if (http == NULL) {
    log_error(http_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_HTTP_NULL");
    return SC_HTTP_NULL;
  }

  MHD_stop_daemon(http->daemon);
  http->running = false;
  return SC_OK;
}
