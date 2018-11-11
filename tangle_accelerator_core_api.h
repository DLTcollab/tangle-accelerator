#ifndef TANGLE_ACCELERATOR_CORE_API_H_
#define TANGLE_ACCELERATOR_CORE_API_H_

#include "./common_core.h"
#include "serializer/json_serialization.h"
#include "types/types.h"

int handle_request(const iota_client_service_t* const service,
                   const char* const obj);
int api_generate_address(const iota_client_service_t* const service,
                         const char* const obj, char* json_result);
int api_get_tips(const iota_client_service_t* const service,
                 const char* const obj, char* json_result);
int api_send_transfer(const iota_client_service_t* const service,
                      const char* const obj, char* json_result);

#endif  // TANGLE_ACCELERATOR_CORE_API_H_
