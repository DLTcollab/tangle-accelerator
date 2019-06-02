#include "fill_nines.h"

status_t fill_nines(char* new_str, const char* const old_str,
                    size_t new_str_len) {
  if (!new_str || !old_str || new_str_len != NUM_TRYTES_TAG) {
    return SC_SERIALIZER_NULL;
  }

  int old_str_len = strlen(old_str);
  strncpy(new_str, old_str, old_str_len);

  int diff = new_str_len - old_str_len;
  if (diff) {
    memset((new_str + old_str_len), '9', diff);
  } else {
    return SC_UTILS_WRONG_REQUEST_OBJ;
  }
  new_str[new_str_len] = '\0';

  return SC_OK;
}
