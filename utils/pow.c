#include "pow.h"
#include "common/helpers/digest.h"
#include "third_party/dcurl/src/dcurl.h"
#include "utils/time.h"

void pow_init() { dcurl_init(); }

void pow_destroy() { dcurl_destroy(); }

static int8_t* ta_pow_dcurl(int8_t* trytes, int mwm, int threads) {
  return dcurl_entry(trytes, mwm, threads);
}

flex_trit_t* ta_pow_flex(const flex_trit_t* const trits_in, const uint8_t mwm) {
  tryte_t trytes_in[NUM_TRYTES_SERIALIZED_TRANSACTION];
  tryte_t nonce_trytes[NUM_TRYTES_NONCE];

  flex_trits_to_trytes(trytes_in, NUM_TRYTES_SERIALIZED_TRANSACTION, trits_in,
                       NUM_TRITS_SERIALIZED_TRANSACTION,
                       NUM_TRITS_SERIALIZED_TRANSACTION);
  int8_t* ret_trytes = ta_pow_dcurl(trytes_in, mwm, 0);
  memcpy(nonce_trytes,
         ret_trytes + NUM_TRYTES_SERIALIZED_TRANSACTION - NUM_TRYTES_NONCE,
         NUM_TRYTES_NONCE);

  flex_trit_t* nonce_trits =
      (flex_trit_t*)calloc(NUM_TRITS_NONCE, sizeof(flex_trit_t));
  if (!nonce_trits) {
    return NULL;
  }
  flex_trits_from_trytes(nonce_trits, NUM_TRITS_NONCE,
                         (const tryte_t*)nonce_trytes, NUM_TRYTES_NONCE,
                         NUM_TRYTES_NONCE);

  free(ret_trytes);
  return nonce_trits;
}

retcode_t ta_pow(const bundle_transactions_t* bundle,
                 const flex_trit_t* const trunk,
                 const flex_trit_t* const branch, const uint8_t mwm) {
  iota_transaction_t* tx;
  flex_trit_t *nonce, *tx_trits;
  flex_trit_t* ctrunk =
      (flex_trit_t*)calloc(FLEX_TRIT_SIZE_243, sizeof(flex_trit_t));
  size_t cur_idx = 0;

  tx = (iota_transaction_t*)utarray_front(bundle);
  cur_idx = transaction_last_index(tx) + 1;
  memcpy(ctrunk, trunk, FLEX_TRIT_SIZE_243);

  do {
    cur_idx--;

    // set trunk, branch, and attachment timestamp
    transaction_set_trunk(tx, ctrunk);
    transaction_set_branch(tx, branch);
    transaction_set_attachment_timestamp(tx, current_timestamp_ms());
    transaction_set_attachment_timestamp_upper(tx, 3812798742493LL);
    transaction_set_attachment_timestamp_lower(tx, 0);

    tx_trits = transaction_serialize(tx);
    if (tx_trits == NULL) {
      return RC_OOM;
    }

    // get nonce
    nonce = ta_pow_flex(tx_trits, mwm);
    if (nonce == NULL) {
      return RC_OOM;
    }
    transaction_set_nonce(tx, nonce);

    free(ctrunk);
    ctrunk = iota_flex_digest(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION);
    free(nonce);
    free(tx_trits);
  } while (cur_idx != 0);

  free(ctrunk);
  return RC_OK;
}
