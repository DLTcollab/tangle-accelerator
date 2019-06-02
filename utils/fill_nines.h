#include "accelerator/errors.h"
#include "cclient/types/types.h"

/**
 * @brief Patch input string with nines into assigned length.
 *
 * @param[out] new_str Output patched string
 * @param[in] old_str Input string which needs to be patched
 * @param[in] new_str_len assigned output string length
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t fill_nines(char* new_str, const char* const old_str,
                    size_t new_str_len);
