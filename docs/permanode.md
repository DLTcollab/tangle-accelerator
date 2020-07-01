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

See [docs/build.md](docs/build.md) for more information about enabling the external storage.
