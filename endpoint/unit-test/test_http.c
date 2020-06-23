#include <stdlib.h>
#include <string.h>
#include "common/ta_errors.h"
#include "http_parser.h"
#include "tests/test_define.h"
#include "utils/connectivity/conn_http.h"

#define TEST_PORT "8000"
#define TEST_HOST "localhost"
#define TEST_GET_REQUEST "GET /address HTTP/1.1\r\nHOST: %s\r\n\r\n"
#define TEST_API "transaction/"

#define TEST_POST_MESSAGE \
  "{\
  \"value\": 0,\
  \"message\": \"ZBCDKDTCFDTCSCEAQCMDEAHDPCBDVC9DTCRAPCRCRCTC9DTCFDPCHDCDFD\",\
  \"tag\": \"POWEREDBYTANGLEACCELERATOR9\",\
  \"address\": \"POWEREDBYTANGLEACCELERATOR9999999999999999999999999999999999999999999999999999999\"\
}"

#define POST_REQUEST            \
  "POST /transaction/ HTTP/1.1\r\n\
Host: " TEST_HOST ":" TEST_PORT \
  "\r\n\
Connection: Keep-Alive\r\n\
Content-Type: application/json\r\n\
Content-Length: 224\r\n\
\r\n\
" TEST_POST_MESSAGE "\r\n\r\n"

#define BUF_SIZE 4096

static char* req = NULL;

void setUp(void) { conn_http_logger_init(); }

void tearDown(void) {
  conn_http_logger_release();
  free(req);
}

void test_http(void) {
  connect_info_t info = {.https = false};
  char post_message[BUF_SIZE] = {0}, response[BUF_SIZE] = {0};
  http_parser parser;
  http_parser_settings settings = {};
  settings.on_body = parser_body_callback;

  snprintf(post_message, BUF_SIZE, "%s", TEST_POST_MESSAGE);

  set_post_request(TEST_API, TEST_HOST, atoi(TEST_PORT), post_message, &req);
  TEST_ASSERT_EQUAL_INT8_ARRAY(POST_REQUEST, req, sizeof(POST_REQUEST));

  TEST_ASSERT_EQUAL_INT(SC_OK, http_open(&info, "nonce", TEST_HOST, TEST_PORT));
  TEST_ASSERT_EQUAL_INT(SC_OK, http_send_request(&info, req));
  TEST_ASSERT_EQUAL_INT(SC_OK, http_read_response(&info, response, sizeof(response) / sizeof(char)));
  TEST_ASSERT_EQUAL_INT(SC_OK, http_close(&info));

  http_parser_init(&parser, HTTP_RESPONSE);
  http_parser_execute(&parser, &settings, response, strlen(response));

  TEST_ASSERT_EQUAL_INT(SC_HTTP_OK, parser.status_code);
}

int main(void) {
  UNITY_BEGIN();
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  RUN_TEST(test_http);

  return UNITY_END();
}
