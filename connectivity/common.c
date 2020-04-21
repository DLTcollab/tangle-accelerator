#include <regex.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "common.h"
#include "common/logger.h"
#include "common/ta_errors.h"

#define CONN_LOGGER "connectivity"
static logger_id_t logger_id;

void conn_logger_init() { logger_id = logger_helper_enable(CONN_LOGGER, LOGGER_DEBUG, true); }

int conn_logger_release() {
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", CONN_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t api_path_matcher(char const *const path, char *const regex_rule) {
  if (regex_rule == NULL) {
    ta_log_error("%s\n", "SC_HTTP_NULL");
    return SC_HTTP_NULL;
  }
  regex_t reg;
  regmatch_t pmatch;
  status_t ret = SC_OK;
  int reg_flag = REG_EXTENDED;

  if (regcomp(&reg, regex_rule, reg_flag) != 0) {
    ta_log_error("%s\n", "SC_HTTP_INVALID_REGEX");
    return SC_HTTP_INVALID_REGEX;
  }
  if (regexec(&reg, path, 1, &pmatch, 0) != 0) {
    // Did not match pattern
    ret = SC_HTTP_URL_NOT_MATCH;
  } else {
    if ((size_t)(pmatch.rm_eo - pmatch.rm_so) != strlen(path)) {
      ret = SC_HTTP_URL_NOT_MATCH;
    }
  }

  regfree(&reg);
  return ret;
}

status_t set_response_content(status_t ret, char **json_result) {
  int http_ret;
  if (ret == SC_OK) {
    return SC_HTTP_OK;
  }

  switch (ret) {
    case SC_CCLIENT_NOT_FOUND:
    case SC_MAM_NOT_FOUND:
      http_ret = SC_HTTP_NOT_FOUND;
      ta_log_error("%s\n", "SC_HTTP_NOT_FOUND");
      *json_result = strdup(STR_HTTP_NOT_FOUND);
      break;
    case SC_CCLIENT_JSON_KEY:
    case SC_MAM_NO_PAYLOAD:
    case SC_HTTP_URL_NOT_MATCH:
      http_ret = SC_HTTP_BAD_REQUEST;
      ta_log_error("%s\n", "SC_HTTP_BAD_REQUEST");
      *json_result = strdup(STR_HTTP_BAD_REQUEST);
      break;
    case SC_SERIALIZER_MESSAGE_OVERRUN:
      http_ret = SC_HTTP_BAD_REQUEST;
      ta_log_error("%s\n", ta_error_to_string(ret));
      *json_result = strdup(STR_HTTP_BAD_REQUEST_MESSAGE_OVERRUN);
      break;
    default:
      http_ret = SC_HTTP_INTERNAL_SERVICE_ERROR;
      ta_log_error("%s\n", "SC_HTTP_INTERNAL_SERVICE_ERROR");
      *json_result = strdup(STR_HTTP_INTERNAL_SERVICE_ERROR);
      break;
  }
  return http_ret;
}
