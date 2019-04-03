#include <time.h>
#include "accelerator/apis.h"
#include "test_define.h"

static ta_core_t ta_core;
struct timespec start_time, end_time;

#if defined(ENABLE_STAT)
#define TEST_COUNT 100
#else
#define TEST_COUNT 1
#endif

double diff_time(struct timespec start, struct timespec end) {
  struct timespec diff;
  if (end.tv_nsec - start.tv_nsec < 0) {
    diff.tv_sec = end.tv_sec - start.tv_sec - 1;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec + 1000000000;
  } else {
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

void test_time_start(struct timespec* start) {
  clock_gettime(CLOCK_REALTIME, start);
}

void test_time_end(struct timespec* start, struct timespec* end, double* sum) {
  clock_gettime(CLOCK_REALTIME, end);
  double difference = diff_time(*start, *end);
#if defined(ENABLE_STAT)
  printf("%lf\n", difference);
#endif
  *sum += difference;
}

void test_generate_address(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(
        SC_OK,
        api_generate_address(&ta_core.tangle, &ta_core.service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of generate_address: %lf\n", sum / TEST_COUNT);
}

void test_get_tips_pair(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(
        SC_OK,
        api_get_tips_pair(&ta_core.tangle, &ta_core.service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_tips_pair: %lf\n", sum / TEST_COUNT);
}

void test_get_tips(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            api_get_tips(&ta_core.service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_tips: %lf\n", sum / TEST_COUNT);
}

void test_send_transfer(void) {
  const char* json =
      "{\"value\":100,"
      "\"message\":\"" TAG_MSG "\",\"tag\":\"" TAG_MSG
      "\","
      "\"address\":\"" TRYTES_81_1 "\"}";
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_send_transfer(&ta_core.tangle, &ta_core.service, json,
                                 &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of send_transfer: %lf\n", sum / TEST_COUNT);
}

void test_get_transaction_object(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_get_transaction_object(&ta_core.service, TRYTES_81_3,
                                          &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_transaction_object: %lf\n", sum / TEST_COUNT);
}

void test_find_transactions_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_find_transactions_by_tag(&ta_core.service, FIND_TAG_MSG,
                                            &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_transactions_by_tag: %lf\n", sum / TEST_COUNT);
}

void test_find_transactions_obj_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_find_transactions_obj_by_tag(&ta_core.service, FIND_TAG_MSG,
                                                &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_tx_obj_by_tag: %lf\n", sum / TEST_COUNT);
}

void test_send_mam_message(void) {
  double sum = 0;

  char* bundle_hash_result;
  char* channel_id_result;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(
        SC_OK,
        api_mam_send_message(&ta_core.tangle, &ta_core.service, TEST_PAYLOAD,
                             &bundle_hash_result, &channel_id_result));
    test_time_end(&start_time, &end_time, &sum);

    for (size_t i = 0; i < FLEX_TRIT_SIZE_243; i++) {
      printf("%c", channel_id_result[i]);
    }
    printf("\n");
    printf("Bundle: ");
    for (size_t i = 0; i < FLEX_TRIT_SIZE_243; i++) {
      printf("%c", bundle_hash_result[i]);
    }
    printf("\n");

    free(bundle_hash_result);
    free(channel_id_result);
  }
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
}

void test_receive_mam_message(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_receive_mam_message(&ta_core.service, TEST_BUNDLE_HASH,
                                       &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
}

int main(void) {
  UNITY_BEGIN();

  ta_config_init(&ta_core.info, &ta_core.tangle, &ta_core.service);

  printf("Total samples for each API test: %d\n", TEST_COUNT);
  RUN_TEST(test_generate_address);
  RUN_TEST(test_get_tips_pair);
  RUN_TEST(test_get_tips);
  RUN_TEST(test_send_transfer);
  RUN_TEST(test_get_transaction_object);
  RUN_TEST(test_find_transactions_by_tag);
  RUN_TEST(test_find_transactions_obj_by_tag);
  RUN_TEST(test_receive_mam_message);
  RUN_TEST(test_send_mam_message);
  ta_config_destroy(&ta_core.service);
  return UNITY_END();
}
