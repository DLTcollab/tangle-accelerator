#include <stdlib.h>
#include <string.h>
#include "common/ta_errors.h"
#include "endpoint/connectivity/conn_http.h"
#include "endpoint/https.h"
#include "http_parser.h"
#include "tests/test_define.h"

static char* TEST_TA_HOST;
static char* TEST_TA_PORT;

#define TEST_GET_REQUEST "GET /address HTTP/1.1\r\nHOST: %s\r\n\r\n"
#define TEST_API "transaction/"

#define TEST_POST_MESSAGE \
  "{\
  \"value\": 0,\
  \"message\": \"ZBCDKDTCFDTCSCEAQCMDEAHDPCBDVC9DTCRAPCRCRCTC9DTCFDPCHDCDFD\",\
  \"tag\": \"POWEREDBYTANGLEACCELERATOR9\",\
  \"address\": \"POWEREDBYTANGLEACCELERATOR9999999999999999999999999999999999999999999999999999999\"\
}"

#define POST_REQUEST \
  "POST /transaction/ HTTP/1.1\r\n\
Host: %s:%d\r\n\
Connection: Keep-Alive\r\n\
Content-Type: application/json\r\n\
Content-Length: 224\r\n\
\r\n\
" TEST_POST_MESSAGE "\r\n\r\n"

#define BUF_SIZE 4096

static char* req = NULL;
static https_response_t my_data;

void setUp(void) { conn_http_logger_init(); }

void tearDown(void) {
  conn_http_logger_release();
  free(my_data.buffer);
  free(req);
}

void test_http(void) {
  connect_info_t info = {.https = false};
  char post_message[BUF_SIZE] = {0}, response[BUF_SIZE] = {0};
  char post_request[BUF_SIZE] = {0};
  http_parser parser;
  http_parser_settings settings = {};
  parser.data = &my_data;

  snprintf(post_message, BUF_SIZE, "%s", TEST_POST_MESSAGE);

  set_post_request(TEST_API, TEST_TA_HOST, atoi(TEST_TA_PORT), post_message, &req);
  snprintf(post_request, BUF_SIZE, POST_REQUEST, TEST_TA_HOST, atoi(TEST_TA_PORT));
  TEST_ASSERT_EQUAL_INT8_ARRAY(post_request, req, sizeof(POST_REQUEST));

  TEST_ASSERT_EQUAL_INT(SC_OK, http_open(&info, "nonce", TEST_TA_HOST, TEST_TA_PORT));
  TEST_ASSERT_EQUAL_INT(SC_OK, http_send_request(&info, req));
  TEST_ASSERT_EQUAL_INT(SC_OK, http_read_response(&info, response, sizeof(response) / sizeof(char)));
  TEST_ASSERT_EQUAL_INT(SC_OK, http_close(&info));

  http_parser_init(&parser, HTTP_RESPONSE);
  http_parser_execute(&parser, &settings, response, strlen(response));

  TEST_ASSERT_EQUAL_INT(SC_HTTP_OK, parser.status_code);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  if (argc != 3) {
    printf("usage: ./test_endpoint_core [TA_HOST] [TA_PORT]");
    return UNITY_END();
  }

  TEST_TA_HOST = argv[1];
  TEST_TA_PORT = argv[2];

  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  RUN_TEST(test_http);

  return UNITY_END();
}
