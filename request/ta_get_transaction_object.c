#include "ta_get_transaction_object.h"

ta_get_transaction_object_req_t* ta_get_transaction_object_req_new() {
    ta_get_transaction_object_req_t *req = (ta_get_transaction_object_req_t*) malloc(sizeof(ta_get_transaction_object_req_t));
    if (req != NULL)
    {
        utarray_new(req->hashes, &ut_str_icd);
        return req;
    }
    return NULL;
}

status_t ta_get_transaction_object_req_push_address(ta_get_transaction_object_req_t *req, const char* const addr) {
    if (req ==NULL || addr == NULL) {
        return SC_TA_NULL;
    }

    if (req->hashes == NULL) {
        utarray_new(req->hashes, &ut_str_icd);
        if (req->hashes == NULL) {
            return SC_TA_OOM;
        }
    }
    utarray_push_back(req->hashes, &addr);
    
    return SC_OK;
}

void ta_get_transaction_object_req_free(ta_get_transaction_object_req_t** req) {
    utarray_free((*req)->hashes);
    free((*req));
    *req = NULL;
}
