#include "iota_api_mock.hh"
#include "cclient/iota_client_core_api.h"
extern APIMock APIMockObj;

retcode_t iota_client_get_transactions_to_approve(
    const iota_client_service_t* const service,
    const get_transactions_to_approve_req_t* const req,
    get_transactions_to_approve_res_t* res) {
  get_transactions_to_approve_res_set_trunk(res, TRITS_81_1);
  get_transactions_to_approve_res_set_branch(res, TRITS_81_2);
  return APIMockObj.iota_client_get_transactions_to_approve(service, req, res);
}
