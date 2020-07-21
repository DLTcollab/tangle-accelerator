# Transaction reattacher

`Transaction reattacher` is a service that helps persistent pending transactions to be re-attached to the Tangle. A persistent transaction is a transaction that does not be confirmed more than 30 minutes.

When enabling the external database for transaction reattachment, `Tangle-Accelerator`  will store transactions issued by API [Send Transfer Message](https://github.com/DLTcollab/tangle-accelerator/wiki/Send-Transfer-Message).

`Transaction reattacher` will periodically read pending transactions from a specific ScyllaDB cluster, and get the latest inclusion status of those transactions from an IOTA full node. `Reattacher` will update the newest inclusion status to the ScyllaDB cluster. For persistent transactions, `reattacher` performs reattachment, which will do tips selection and PoW for the original bundle, and reattach it to the Tangle. After reattachment, `reattacher` will update the new transaction hash to the ScyllaDB cluster.

See [docs/build.md](docs/build.md) for more information about enabling transaction reattachment.

## Build Instructions

`bazel build //reattacher`

The reattacher support following options :

* `DB_HOST`: binding address of ScyllDB cluster
* `NODE_HOST`: binding address of IOTA full node
* `NODE_PORT`: port of IOTA full node

If you do not specify `DB_HOST` or `NODE_HOST`, the address will be set as `localhost`.
