#include "common/ta_errors.h"

const char* ta_error_to_string(status_t err) {
  switch (err) {
    case SC_OK:
      return "No error.";

    // HTTP
    case SC_HTTP_OK:
      return "HTTP response OK.";
    case SC_HTTP_BAD_REQUEST:
      return "HTTP response, error when parsing request.";
    case SC_HTTP_NOT_FOUND:
      return "HTTP request not found.";
    case SC_HTTP_INTERNAL_SERVICE_ERROR:
      return "HTTP response, other errors in TA.";

    // Tangle-Accelerator
    case SC_OOM:
      return "Failed to create TA object.";
    case SC_NULL:
      return "NULL TA objects.";
    case SC_TA_WRONG_REQUEST_OBJ:
      return "Wrong TA request object.";
    case SC_TA_LOGGER_INIT_FAIL:
      return "Failed to init TA logger.";

    // CClient module
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

    // Serializer
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

    // Cache
    case SC_CACHE_FAILED_RESPONSE:
      return "Failed in cache operations.";
    case SC_CACHE_OFF:
      return "Cache server is not turned on.";
    case SC_CACHE_INIT_FINI:
      return "Failed to initialize or destroy lock in cache";
    case SC_CACHE_LOCK_FAILURE:
      return "Failed to lock or unlock cache operation";

    // MAM
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
      return "Failed to write MAM packet.";
    case SC_MAM_FILE_SAVE:
      return "Failed to save MAM file.";
    case SC_MAM_ALL_MSS_KEYS_USED:
      return "All MSS private keys of current given parameters are used.";
    case SC_MAM_FAILED_CREATE_OR_GET_ID:
      return "Failed to created/get chid or epid or msg_id.";
    case SC_MAM_FAILED_WRITE_HEADER:
      return "Failed to write header or packet in MAM.";
    case SC_MAM_READ_MESSAGE_ERROR:
      return "Met error when reading MAM message from bundle";
    case SC_MAM_INVAID_CHID_OR_EPID:
      return "Failed to add trusted channel ID or endpoint ID";
    case SC_MAM_EXCEEDED_CHID_ITER:
      return "Too much iteration for finding a starting chid";

    // Configuration
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

    // Utilities
    case SC_UTILS_WRONG_INPUT_ARG:
      return "Wrong utilities input object";
    case SC_UTILS_TIMER_ERROR:
      return "Errors occurred in timer function";
    case SC_UTILS_TIMER_EXPIRED:
      return "Timer expired";
    case SC_UTILS_HTTPS_SEND_ERROR:
      return "Failed to send message";
    case SC_UTILS_HTTPS_INIT_ERROR:
      return "HTTPS module initialize error";
    case SC_UTILS_HTTPS_X509_ERROR:
      return "HTTPS X509 certificate parse error";
    case SC_UTILS_HTTPS_CONN_ERROR:
      return "HTTPS initial connection error";
    case SC_UTILS_HTTPS_SSL_ERROR:
      return "HTTPS setting SSL config error";
    case SC_UTILS_HTTPS_RESPONSE_ERROR:
      return "HTTPS response error";
    case SC_UTILS_TEXT_SERIALIZE:
      return "Error occurred when serializing message";
    case SC_UTILS_TEXT_DESERIALIZE:
      return "Error occurred when deserializing message";
    case SC_UTILS_OVERFLOW_ERROR:
      return "Overflow error";
    case SC_UTILS_CIPHER_ERROR:
      return "Error occurred when encrypting or descrypting message";

    // Connection HTTP
    case SC_HTTP_INVALID_REGEX:
      return "Invalid URL regular expression rule in http.";
    case SC_HTTP_URL_NOT_MATCH:
      return "URL doesn't match regular expression rule.";
    case SC_HTTP_URL_PARSE_ERROR:
      return "URL parameter parsing error.";
    case SC_HTTP_COMMAND_NOT_MATCH:
      return "Proxy API command hash does not match.";

    // Connection MQTT
    case SC_MQTT_INIT:
      return "Error during initialization in MQTT.";
    case SC_MQTT_MOSQ_OBJ_INIT_ERROR:
      return "Error initializing mosquitto object.";
    case SC_MQTT_TOPIC_SET:
      return "Error setting topic in MQTT.";
    case SC_MQTT_OPT_SET:
      return "Error setting options of mosquitto object.";
    case SC_MQTT_CONNECT:
      return "Error in connecting to broker.";
    case SC_MQTT_INVALID_TAG:
      return "Received invalid tag length in MQTT";

    // Storage
    case SC_STORAGE_CONNECT_FAIL:
      return "Failed to connect ScyllaDB node.";
    case SC_STORAGE_INVALID_INPUT:
      return "Invalid input parameter in ScyllaDB.";
    case SC_STORAGE_CASSANDRA_QUERY_FAIL:
      return "Failed to execute Cassandra query.";

    // Core
    case SC_CORE_NODE_UNSYNC:
      return "IOTA full node host is not synchronized.";

      // Endpoint
    case SC_ENDPOINT_DEVICE_INIT:
      return "Failed to initialize the device.";
    case SC_ENDPOINT_DEVICE_FINI:
      return "Failed to finalize the device";
    case SC_ENDPOINT_UART:
      return "UART error occurred in device component";
    case SC_ENDPOINT_UART_SET_ATTR:
      return "UART error occurred when setting UART attribute";
    case SC_ENDPOINT_SEC_FAULT:
      return "Error occurred inside secure storage";
    case SC_ENDPOINT_SEC_ITEM_NOT_FOUND:
      return "Item not found inside secure storage";
    case SC_ENDPOINT_SEC_UNAVAILABLE:
      return "Secure storage service is unavailable";
    case SC_ENDPOINT_SEND_TRANSFER:
      return "Error occurred when sending the transfer message";
    case SC_ENDPOINT_GET_KEY_ERROR:
      return "Error occurred when get private key from endpoint device";
    case SC_ENDPOINT_GET_DEVICE_ID_ERROR:
      return "Error occurred when get device id from endpoint device";
    case SC_ENDPOINT_DNS_RESOLVE_ERROR:
      return "Error occurred when resolving the domain name";
    case SC_ENDPOINT_CAN_OPEN_ERROR:
      return "Error occurred when opening CAN BUS socket";
    case SC_ENDPOINT_CAN_SEND_ERROR:
      return "Error occurred when writing message into CAN BUS socket";
    case SC_ENDPOINT_CAN_RECV_ERROR:
      return "Error occurred when reading message from CAN BUS socket";
    case SC_ENDPOINT_CAN_CLOSE_ERROR:
      return "Error occurred when closing CAN BUS socket";
    case SC_ENDPOINT_UART_WRITE_ERROR:
      return "Error occurred when writing message to UART";
    case SC_ENDPOINT_UART_READ_ERROR:
      return "Error occurred when reading message from UART";
    case SC_ENDPOINT_SET_KEY_ERROR:
      return "Fail to set a new AES key";

    // Crypto
    case SC_CRYPTO_RAND_ERR:
      return "Failed to generate random number generator";
    case SC_CRYPTO_GENKEY_ERR:
      return "Failed to generate ECDH public key";
    case SC_CRYPTO_SECRET_ERR:
      return "Failed to compute ECDH shared secret";

    default:
      return "Unknown error.";
  }
}
