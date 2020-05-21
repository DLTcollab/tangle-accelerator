# Permanode 

The permanode solution is based on ScyllaDB, which is a NoSQL distributed database. `tangle-accelerator` implements a ScyllaDB backend to store the IOTA Tangle ledger and make it queriable.

The ScyllaDB backend in `tangle-accelerator` supports the following APIs:
 - insert_transactions
 - get_trytes
 - find_transactions_by_bundle
 - find_transactions_by_address
 - find_transactions_by_tag
 - find_transactions_approvees
 - get_inclusion_status

Read [docs/build.md] for more information about enabling the external storage.

## Listener

The listener subscribes newly confirmed transactions from IRI and adds inserting tasks into the task queue of the thread pool.

Here are configurations and CLI options you need to specify:

* `--iri_host`: Listening IRI host for ZMQ events and querying trytes.
* `--db_host`: Connecting ScyllaDB host name.
* `--thread_num`: Workers number in the thread pool to handle receiving transactions.

Build command:

`bazel build //storage:scylladb_listener`

## Importer 

The importer reads confirmed transactions from historical transaction files dumped from IRI and adds inserting tasks into the task queue of thread pool.

The historical transaction files must consist of lines with the format: `TRANSACTION_HASH,TRYTES,SNAPSHOT_INDEX`

Here are configurations and CLI options you need to specify:

* `--db_host`: Connecting ScyllaDB host name.
* `--file`: A file consist of historical transaction file paths.
* `--thread_num`: Worker's number in the thread pool to handle receiving transactions.

Build command:

`bazel build //storage:scylladb_importer`
