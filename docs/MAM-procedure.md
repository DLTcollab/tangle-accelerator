# MAM Procedure

Tangle-accelerator allows clients send MAM message and fetch MAM message through it.

## Channel Encryption

Clients can encrypt a Channel in the first message they send to the Channel. All the messages under this Channel will be encrypted. This encryption system can serve as grouping different sets of subscribers. This encryption system, is based on the cryptographic algorithm provided by MAM, which are PSK and NTRU.

### PSK

PSK is the abbreviation of Pre-Shared Key. It is a symmetric encryption algorithm. Subscribers share the same PSK keys have the same access authority for the messages on the MAM channel.
However, the next Channel would not inherit the PSK keys in the last Channel.
