#include "common/ta_errors.h"

const char* ta_error_to_string(status_t err) {
  switch (err) {
    case SC_OK:
      return "No error.";
    case SC_HTTP_OK:
      return "HTTP response OK.";
    case SC_HTTP_BAD_REQUEST:
      return "HTTP response, error when parsing request.";
    case SC_HTTP_NOT_FOUND:
      return "HTTP request not found.";
    case SC_HTTP_INTERNAL_SERVICE_ERROR:
      return "HTTP response, other errors in TA.";
    case SC_TA_OOM:
      return "Failed to create TA object.";
    case SC_TA_NULL:
      return "NULL TA objects.";
    case SC_TA_WRONG_REQUEST_OBJ:
      return "Wrong TA request object.";
    case SC_TA_LOGGER_INIT_FAIL:
      return "Failed to init TA logger.";
    case SC_CCLIENT_OOM:
      return "Failed to create cclient object.";
    case SC_CCLIENT_NOT_FOUND:
      return "Empty result from cclient.";
    case SC_CCLIENT_FAILED_RESPONSE:
      return "Error in cclient response.";
    case SC_CCLIENT_INVALID_FLEX_TRITS:
      return "Invalid flex trits.";
    case SC_CCLIENT_HASH:
      return "Hash container operation error.";
    case SC_CCLIENT_JSON_KEY:
      return "JSON key not found.";
    case SC_CCLIENT_JSON_PARSE:
      return "JSON parsing error.";
    case SC_CCLIENT_FLEX_TRITS:
      return "flex_trits converting error.";
    case SC_CCLIENT_JSON_CREATE:
      return "JSON create object error.";
    case SC_SERIALIZER_JSON_CREATE:
      return "Failed to create JSON object in serializer.";
    case SC_SERIALIZER_NULL:
      return "NULL object in serializer.";
    case SC_SERIALIZER_JSON_PARSE:
      return "Failed to parse JSON object in serializer.";
    case SC_SERIALIZER_JSON_PARSE_NOT_TRYTE:
      return "Unicode value in JSON.";
    case SC_SERIALIZER_INVALID_REQ:
      return "Invalid request value in JSON.";
    case SC_SERIALIZER_MESSAGE_OVERRUN:
      return "Message length is out of valid size.";
    case SC_CACHE_NULL:
      return "NULL object in cache.";
    case SC_CACHE_FAILED_RESPONSE:
      return "Failed in cache operations.";
    case SC_CACHE_OFF:
      return "Cache server is not turned on.";
    case SC_MAM_NULL:
      return "NULL object in MAM.";
    case SC_MAM_NOT_FOUND:
      return "Empty result from MAM.";
    case SC_MAM_FAILED_INIT:
      return "Error in MAM initialization.";
    case SC_MAM_FAILED_RESPONSE:
      return "Error in MAM response.";
    case SC_MAM_FAILED_DESTROYED:
      return "Error in MAM destroy.";
    case SC_MAM_NO_PAYLOAD:
      return "No payload or no chid in MAM.";
    case SC_MAM_FAILED_WRITE:
      return "Failed to write in MAM.";
    case SC_MAM_FILE_SAVE:
      return "Failed to save MAM file.";
    case SC_MAM_ALL_MSS_KEYS_USED:
      return "All MSS private keys of current given parameters are used.";
    case SC_MAM_FAILED_CREATE_OR_GET_ID:
      return "Failed to created/get chid or epid or msg_id.";
    case SC_MAM_FAILED_WRITE_HEADER:
      return "Failed to write header in MAM.";
    case SC_RES_NULL:
      return "NULL object in response.";
    case SC_CONF_NULL:
      return "NULL object in configuration.";
    case SC_CONF_MISSING_ARGUMENT:
      return "No argument in CLI.";
    case SC_CONF_UNKNOWN_OPTION:
      return "Undefined option in CLI.";
    case SC_CONF_LOCK_INIT:
      return "Failed to init lock.";
    case SC_CONF_LOCK_DESTROY:
      return "Failed to destroy lock.";
    case SC_CONF_PARSER_ERROR:
      return "Failed to initialize yaml parser.";
    case SC_CONF_FOPEN_ERROR:
      return "Failed to open file.";
    case SC_UTILS_WRONG_REQUEST_OBJ:
      return "Wrong TA request object.";
    case SC_UTILS_TIMER_ERROR:
      return "Errors occurred in timer function.";
    case SC_UTILS_TIMER_EXPIRED:
      return "Timer expired.";
    case SC_HTTP_OOM:
      return "Failed to create http object.";
    case SC_HTTP_NULL:
      return "NULL object in http.";
    case SC_HTTP_INVALID_REGEX:
      return "Invalid URL regular expression rule in http.";
    case SC_HTTP_URL_NOT_MATCH:
      return "URL doesn't match regular expression rule.";
    case SC_HTTP_URL_PARSE_ERROR:
      return "URL parameter parsing error.";
    case SC_HTTP_COMMAND_NOT_MATCH:
      return "Proxy API command hash does not match.";
    case SC_MQTT_OOM:
      return "Failed to create MQTT object.";
    case SC_MQTT_NULL:
      return "NULL object in MQTT.";
    case SC_MQTT_INIT:
      return "Error during initialization in MQTT.";
    case SC_MOSQ_OBJ_INIT_ERROR:
      return "Error initializing mosquitto object.";
    case SC_MQTT_TOPIC_SET:
      return "Error setting topic in MQTT.";
    case SC_MQTT_OPT_SET:
      return "Error setting options of mosquitto object.";
    case SC_MQTT_INVALID_TAG:
      return "Received invalid tag length in MQTT.";
    case SC_CLIENT_CONNECT:
      return "Error in connecting to broker.";
    case SC_STORAGE_OOM:
      return "Failed to create storage object.";
    case SC_STORAGE_CONNECT_FAIL:
      return "Failed to connect ScyllaDB node.";
    case SC_STORAGE_INVALID_INPUT:
      return "Invalid input parameter in ScyllaDB.";
    case SC_STORAGE_CASSANDRA_QUERY_FAIL:
      return "Failed to execute Cassandra query.";
    case SC_CORE_OOM:
      return "Failed to create core object.";
    case SC_CORE_NULL:
      return "NULL object in core.";
    case SC_CORE_IRI_UNSYNC:
      return "IRI host is not synchronized.";
    default:
      return "Unknown error.";
  }
}
