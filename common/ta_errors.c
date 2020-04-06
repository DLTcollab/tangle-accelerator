#include "common/ta_errors.h"

const char* ta_error_to_string(status_t err) {
  switch (err) {
    case SC_OK:
      return "No error\n";
    case SC_HTTP_OK:
      return "HTTP response OK\n";
    case SC_HTTP_BAD_REQUEST:
      return "HTTP response, error when parsing request\n";
    case SC_HTTP_NOT_FOUND:
      return "HTTP request not found\n";
    case SC_HTTP_INTERNAL_SERVICE_ERROR:
      return "HTTP response, other errors in TA\n";
    case SC_TA_OOM:
      return "Failed to create TA object\n";
    case SC_TA_NULL:
      return "NULL TA objects\n";
    case SC_TA_WRONG_REQUEST_OBJ:
      return "Wrong TA request object\n";
    case SC_TA_LOGGER_INIT_FAIL:
      return "Failed to init TA logger\n";
    case SC_CCLIENT_OOM:
      return "Failed to create cclient object\n";
    case SC_CCLIENT_NOT_FOUND:
      return "Empty result from cclient\n";
    case SC_CCLIENT_FAILED_RESPONSE:
      return "Error in cclient response\n";
    case SC_CCLIENT_INVALID_FLEX_TRITS:
      return "Invalid flex trits\n";
    case SC_CCLIENT_HASH:
      return "Hash container operation error\n";
    case SC_CCLIENT_JSON_KEY:
      return "JSON key not found\n";
    case SC_CCLIENT_JSON_PARSE:
      return "JSON parsing error\n";
    case SC_CCLIENT_FLEX_TRITS:
      return "flex_trits converting error\n";
    case SC_CCLIENT_JSON_CREATE:
      return "JSON create object error\n";
    case SC_SERIALIZER_JSON_CREATE:
      return "Failed to create JSON object in serializer\n";
    case SC_SERIALIZER_NULL:
      return "NULL object in serializer\n";
    case SC_SERIALIZER_JSON_PARSE:
      return "Failed to parse JSON object in serializer\n";
    case SC_SERIALIZER_JSON_PARSE_ASCII:
      return "Unicode value in JSON\n";
    case SC_SERIALIZER_INVALID_REQ:
      return "Invalid request value in JSON\n";
    case SC_CACHE_NULL:
      return "NULL object in cache\n";
    case SC_CACHE_FAILED_RESPONSE:
      return "Failed in cache operations\n";
    case SC_CACHE_OFF:
      return "Cache server is not turned on\n";
    case SC_MAM_NULL:
      return "NULL object in MAM\n";
    case SC_MAM_NOT_FOUND:
      return "Empty result from MAM\n";
    case SC_MAM_FAILED_INIT:
      return "Error in MAM initialization\n";
    case SC_MAM_FAILED_RESPONSE:
      return "Error in MAM response\n";
    case SC_MAM_FAILED_DESTROYED:
      return "Error in MAM destroy\n";
    case SC_MAM_NO_PAYLOAD:
      return "No payload or no chid in MAM\n";
    case SC_MAM_FAILED_WRITE:
      return "Failed to write in MAM\n";
    case SC_MAM_FILE_SAVE:
      return "Failed to save MAM file\n";
    case SC_MAM_ALL_MSS_KEYS_USED:
      return "All MSS private keys of current given parameters are used\n";
    case SC_MAM_FAILED_CREATE_OR_GET_ID:
      return "Failed to created/get chid or epid or msg_id\n";
    case SC_MAM_FAILED_WRITE_HEADER:
      return "Failed to write header in MAM\n";
    case SC_RES_NULL:
      return "NULL object in response\n";
    case SC_CONF_NULL:
      return "NULL object in configuration\n";
    case SC_CONF_MISSING_ARGUMENT:
      return "No argument in CLI\n";
    case SC_CONF_UNKNOWN_OPTION:
      return "Undefined option in CLI\n";
    case SC_CONF_LOCK_INIT:
      return "Failed to init lock\n";
    case SC_CONF_LOCK_DESTROY:
      return "Failed to destroy lock\n";
    case SC_CONF_PARSER_ERROR:
      return "Failed to initialize yaml parser\n";
    case SC_CONF_FOPEN_ERROR:
      return "Failed to open file\n";
    case SC_UTILS_WRONG_REQUEST_OBJ:
      return "Wrong TA request object\n";
    case SC_UTILS_TIMER_ERROR:
      return "Errors occurred in timer function\n";
    case SC_UTILS_TIMER_EXPIRED:
      return "Timer expired\n";
    case SC_HTTP_OOM:
      return "Failed to create http object\n";
    case SC_HTTP_NULL:
      return "NULL object in http\n";
    case SC_HTTP_INVALID_REGEX:
      return "Invalid URL regular expression rule in http\n";
    case SC_HTTP_URL_NOT_MATCH:
      return "URL doesn't match regular expression rule\n";
    case SC_HTTP_URL_PARSE_ERROR:
      return "URL parameter parsing error\n";
    case SC_MQTT_OOM:
      return "Failed to create MQTT object\n";
    case SC_MQTT_NULL:
      return "NULL object in MQTT\n";
    case SC_MQTT_INIT:
      return "Error during initialization in MQTT\n";
    case SC_MOSQ_OBJ_INIT_ERROR:
      return "Error initializing mosquitto object\n";
    case SC_MQTT_TOPIC_SET:
      return "Error setting topic in MQTT\n";
    case SC_MQTT_OPT_SET:
      return "Error setting options of mosquitto object\n";
    case SC_CLIENT_CONNECT:
      return "Error in connecting to broker\n";
    case SC_STORAGE_OOM:
      return "Failed to create storage object\n";
    case SC_STORAGE_CONNECT_FAIL:
      return "Failed to connect ScyllaDB node\n";
    case SC_STORAGE_INVALID_INPUT:
      return "Invaild input parameter in ScyllaDB\n";
    case SC_STORAGE_CASSANDRA_QUREY_FAIL:
      return "Failed to execute Cassandra query\n";
    case SC_CORE_OOM:
      return "Failed to create core object\n";
    case SC_CORE_NULL:
      return "NULL object in core\n";
    case SC_CORE_IRI_UNSYNC:
      return "IRI host is unsynchronized\n";
    default:
      return "Unknown error\n";
  }
}
