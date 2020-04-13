# Endpoint
The endpoint is one of the components provided by Tangle-accelerator, running on a resource-constrained network connectivity module. The embedded devices can send messages to blockchain network (Tangle) with a connectivity module loaded endpoint. The message would be transmitted to connectivity module through UART. Message would be encrypted and send to tangle.

# Streaming Message Channel Implementation
The encrypted message would be sent to Tangle with a streaming message channel API. The streaming message channel API would ensure the order of messages in the channel. The user who wants to fetch/send message to Tangle needs to provide `data_id`, `key` and `protocol` to identify a specific message.
A message sent by endpoint needs to be encrypted locally, which avoids message being peeked and modified. 

# How to use
```
$ git clone --recursive https://github.com/DLTcollab/tangle-accelerator  
$ make -j$(nproc)
```
## build legato app
```
$ make legato
```
