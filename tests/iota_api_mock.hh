#include "cclient/iota_client_core_api.h"
#include "cclient/iota_client_extended_api.h"
#include "gmock/gmock.h"

#define TAG_MSG "TANGLEACCELERATOR9999999999"

#define TRYTES_81_1                                                            \
  "LCIKYSBE9IHXLIKCEJTTIQOTTAWSQCCQQ9A9VOKIWRBWVPXMCGUENWVVMQAMPEIVHEQ9JXLCNZ" \
  "OORVZTZ"
#define TRYTES_81_2                                                            \
  "RVORZ9SIIP9RCYMREUIXXVPQIPHVCNPQ9HZWYKFWYWZRE9JQKG9REPKIASHUUECPSQO9JT9XNM" \
  "VKWYGVA"

const flex_trit_t TRITS_81_1[] = {
    76,  67,  73,  75, 89, 83,  66, 69, 57,   73,   72,  88,  76,  73,  75, 67,
    69,  74,  84,  84, 73, 81,  79, 84, 84,   65,   87,  83,  81,  67,  67, 81,
    81,  57,  65,  57, 86, 79,  75, 73, 87,   82,   66,  87,  86,  80,  88, 77,
    67,  71,  85,  69, 78, 87,  86, 86, 77,   81,   65,  77,  80,  69,  73, 86,
    72,  69,  81,  57, 74, 88,  76, 67, 78,   90,   79,  79,  82,  86,  90, 84,
    90,  -13, -11, 78, -3, 127, 0,  0,  32,   -5,   -17, 114, -27, 127, 0,  0,
    0,   16,  0,   0,  0,  0,   0,  0,  -128, 17,   64,  0,   0,   0,   0,  0,
    64,  -11, -11, 78, -3, 127, 0,  0,  0,    0,    0,   0,   0,   0,   0,  0,
    0,   0,   0,   0,  0,  0,   0,  0,  -124, -15,  -69, 114, -27, 127, 0,  0,
    112, -13, -11, 78, -3, 127, 0,  0,  0,    16,   0,   0,   0,   0,   0,  0,
    0,   16,  0,   0,  0,  0,   0,  0,  -13,  -127, -70, 114, -27, 127, 0,  0,
    6,   8,   0,   0,  0,  0,   0,  0,  -39,  21,   26,  0,   0,   0,   0,  0,
    1,   0,   0,   0,  0,  0,   0,  0,  -92,  -127, 0,   0,   -24, 3,   0,  0,
    -24, 3,   0,   0,  0,  0,   0,  0,  0,    0,    0,   0,   0,   0,   0,  0,
    0,   0,   0,   0,  0,  0,   0,  0,  0,    16,   0,   0,   0,   0,   0,  0,
    0,   0,   0};

const flex_trit_t TRITS_81_2[] = {
    82,   86,  79, 82,  90,  57,  83, 73, 73,   80,  57,  82, 67,  89,  77, 82,
    69,   85,  73, 88,  88,  86,  80, 81, 73,   80,  72,  86, 67,  78,  80, 81,
    57,   72,  90, 87,  89,  75,  70, 87, 89,   87,  90,  82, 69,  57,  74, 81,
    75,   71,  57, 82,  69,  80,  75, 73, 65,   83,  72,  85, 85,  69,  67, 80,
    83,   81,  79, 57,  74,  84,  57, 88, 78,   77,  86,  75, 87,  89,  71, 86,
    65,   54,  80, 17,  -75, 127, 0,  0,  96,   0,   0,   0,  0,   0,   0,  0,
    -48,  17,  64, 0,   0,   0,   0,  0,  0,    0,   0,   0,  0,   0,   0,  0,
    0,    0,   0,  0,   0,   0,   0,  0,  124,  -40, -15, 17, -75, 127, 0,  0,
    -48,  -31, 49, -47, -2,  127, 0,  0,  -126, 27,  64,  0,  0,   0,   0,  0,
    0,    0,   0,  0,   0,   0,   0,  0,  124,  -40, -15, 17, -75, 127, 0,  0,
    0,    0,   0,  0,   0,   0,   0,  0,  -29,  7,   -14, 17, -75, 127, 0,  0,
    96,   0,   0,  0,   0,   0,   0,  0,  -25,  7,   -14, 17, -75, 127, 0,  0,
    0,    0,   0,  0,   0,   0,   0,  0,  38,   -31, -15, 17, -75, 127, 0,  0,
    32,   -30, 49, -47, -2,  127, 0,  0,  60,   -5,  -15, 17, -75, 127, 0,  0,
    0,    0,   0,  0,   0,   0,   0,  0,  0,    0,   0,   0,  96,  0,   0,  0,
    -126, 27,  64};

class IotaAPI {
 public:
  virtual ~IotaAPI() {}
  virtual retcode_t iota_client_get_transactions_to_approve(
      const iota_client_service_t* const service,
      const get_transactions_to_approve_req_t* const req,
      get_transactions_to_approve_res_t* res) {
    return RC_OK;
  }
  virtual retcode_t iota_client_get_tips(
      const iota_client_service_t* const service, get_tips_res_t* res) {
    return RC_OK;
  }
  virtual retcode_t iota_client_find_transactions(
      const iota_client_service_t* const service,
      const find_transactions_req_t* const req, find_transactions_res_t* res) {
    return RC_OK;
  }
  virtual retcode_t iota_client_get_new_address(
      iota_client_service_t const* const serv, flex_trit_t const* const seed,
      address_opt_t const addr_opt, hash243_queue_t* out_addresses) {
    return RC_OK;
  }
};

class APIMock : public IotaAPI {
 public:
  virtual ~APIMock() {}
  MOCK_METHOD3(iota_client_get_transactions_to_approve,
               retcode_t(const iota_client_service_t* const service,
                         const get_transactions_to_approve_req_t* const req,
                         get_transactions_to_approve_res_t* res));
  MOCK_METHOD2(iota_client_get_tips,
               retcode_t(const iota_client_service_t* const service,
                         get_tips_res_t* res));
  MOCK_METHOD3(iota_client_find_transactions,
               retcode_t(const iota_client_service_t* const service,
                         const find_transactions_req_t* const req,
                         find_transactions_res_t* res));
  MOCK_METHOD4(iota_client_get_new_address,
               retcode_t(iota_client_service_t const* const serv,
                         flex_trit_t const* const seed,
                         address_opt_t const addr_opt,
                         hash243_queue_t* out_addresses));
};
