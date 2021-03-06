# Transaction Buffer

## Introduction

Failed transaction sending requests would be buffered in the redis server.

## Structure

There are three lists used.

### 1. buffer list

    This list stores all the unsent failed requests. The key (name) of the list is stored in the `buffer_list_name` field of `ta_cache_t`object.
    Each element in this list is the UUID of the corresponding request.

### 2. done list

    This list stores all the sent failed requests. The key (name) of the list is stored in the `complete_list_name` field of `ta_cache_t`object.
    Each element in this list is the UUID of the corresponding request.

### 3. bundle list

    This list represents a bundle. In tangle-accelerator, we store each bundle generated by each request. A bundle in [`iota_common`](https://github.com/iotaledger/iota_common) is basically a list of transactions. Therefore, the data structure in the redis server is the same as the implementation in `iota_common`. It is a list of transaction objects in `flex_trit_t`, and the key (name) of this list is the UUID of the corresponding request.
