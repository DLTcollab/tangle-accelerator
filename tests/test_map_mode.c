#include "map/mode.h"
#include "test_define.h"

#define CHID "UANFAVTSAXZMYUWRECNAOJDAQVTTORVGJCCISMZYAFFU9EYLBMZKEJ9VNXVFFGUTCHONEYVWVUTBTDJLO"
#define NEW_CHID "ONMTPDICUWBGEGODWKGBGMLNAZFXNHCJITSSTBTGMXCXBXJFBPOPXFPOJTXKOOSAJOZAYANZZBFKYHJ9N"
#define EPID "KI99YKKLFALYRUVRXKKRJCPVFISPMNCQQSMB9BGUWIHZTYFQOBZWYSVRNKVFJLSPPLPSFNBNREJWOR99U"

void test_channel_create(void) {
  mam_api_t mam;
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);

  map_channel_create(&mam, channel_id);
  channel_id[MAM_CHANNEL_ID_TRYTE_SIZE] = '\0';
  TEST_ASSERT_EQUAL_STRING(CHID, channel_id);

  mam_api_destroy(&mam);
}

void test_announce_channel(void) {
  mam_api_t mam;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  trit_t msg_id[MAM_MSG_ID_SIZE];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);

  map_channel_create(&mam, channel_id);
  map_announce_channel(&mam, channel_id, bundle, msg_id, channel_id);
  channel_id[MAM_CHANNEL_ID_TRYTE_SIZE] = '\0';
  TEST_ASSERT_EQUAL_STRING(NEW_CHID, channel_id);

  bundle_transactions_free(&bundle);
  mam_api_destroy(&mam);
}

void test_announce_endpoint(void) {
  mam_api_t mam;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  trit_t msg_id[MAM_MSG_ID_SIZE];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);

  map_channel_create(&mam, channel_id);
  // Channel_id is actually the new endpoint id
  map_announce_endpoint(&mam, channel_id, bundle, msg_id, channel_id);
  channel_id[MAM_CHANNEL_ID_TRYTE_SIZE] = '\0';
  TEST_ASSERT_EQUAL_STRING(EPID, channel_id);

  bundle_transactions_free(&bundle);
  mam_api_destroy(&mam);
}

void test_write_message(void) {
  mam_api_t mam;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  trit_t msg_id[MAM_MSG_ID_SIZE];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);
  retcode_t ret = RC_ERROR;

  map_channel_create(&mam, channel_id);
  ret = map_write_header_on_channel(&mam, channel_id, bundle, msg_id);
  TEST_ASSERT_EQUAL(RC_OK, ret);

  ret = map_write_packet(&mam, bundle, TEST_PAYLOAD, msg_id, true);
  TEST_ASSERT_EQUAL(RC_OK, ret);

  bundle_transactions_free(&bundle);
  mam_api_destroy(&mam);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_channel_create);
  RUN_TEST(test_announce_channel);
  RUN_TEST(test_announce_endpoint);
  RUN_TEST(test_write_message);
  return UNITY_END();
}
