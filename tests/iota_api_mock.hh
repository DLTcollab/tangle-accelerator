#include "cclient/iota_client_core_api.h"
#include "cclient/iota_client_extended_api.h"
#include "gmock/gmock.h"
#include "serializer/test/test_serializer.h"

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
