#ifndef UTILS_POW_H_
#define UTILS_POW_H_

#include <stdint.h>
#include "cclient/types/types.h"
#include "common/model/bundle.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file pow.h
 * @brief Implementation of pow interface
 * @example test_pow.c
 */

/**
 * Initiate pow module
 *
 * @return NULL
 */
void pow_init();

/**
 * Stop interacting with pow module
 *
 * @return NULL
 */
void pow_destroy();

/**
 * Perform PoW and return the result of flex trits
 *
 * @param[in] trits_in Flex trits that does pow
 * @param[in] mwm Maximum weight magnitude
 *
 * @return a flex_trit_t data
 */
flex_trit_t* ta_pow_flex(const flex_trit_t* const trits_in, const uint8_t mwm);

/**
 * Perform PoW to the given bundle
 *
 * @param[in] bundle Bundle that does pow
 * @param[in] trunk Trunk transaction hash
 * @param[in] branch Branch transaction hash
 * @param[in] mwm Maximum weight magnitude
 *
 * @return
 * - zero on success
 * - non-zero on error
 */
retcode_t ta_pow(const bundle_transactions_t* bundle,
                 const flex_trit_t* const trunk,
                 const flex_trit_t* const branch, const uint8_t mwm);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_POW_H_
