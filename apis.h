#ifndef APIS_H_
#define APIS_H_

#include "./common_core.h"
#include "serializer/serializer.h"
#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int handle_request(const iota_client_service_t* const service,
                   const char* const obj);
int api_generate_address(const iota_client_service_t* const service,
                         char** json_result);
int api_get_tips(const iota_client_service_t* const service,
                 const char* const obj, char** json_result);
int api_send_transfer(const iota_client_service_t* const service,
                      const char* const obj, char** json_result);
int api_get_transaction_msg(const iota_client_service_t* const service,
                            const char* const obj, char** json_result);
int api_find_transactions_by_tag(const iota_client_service_t* const service,
                                 const char* const obj, char** json_result);

#ifdef __cplusplus
}
#endif

#endif  // APIS_H_
