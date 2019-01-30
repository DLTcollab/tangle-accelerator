#ifndef UTILS_POW_H_
#define UTILS_POW_H_

#include <stdint.h>
#include "common/model/bundle.h"
#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void pow_init();

void pow_destroy();

flex_trit_t* ta_pow_flex(const flex_trit_t* const trits_in, const uint8_t mwm);

retcode_t ta_pow(const bundle_transactions_t* bundle,
                 const flex_trit_t* const trunk,
                 const flex_trit_t* const branch, const uint8_t mwm);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_POW_H_
