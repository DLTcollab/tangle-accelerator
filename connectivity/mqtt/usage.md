## General Usage on MQTT Protocol support
The format of MQTT request is the same as http request, but MQTT request has one more field is `Device ID`.

## Topic vs API table

| topic                   | API                                | HTTP Method   |
| -------------           |:-------------:                     | -------------:|
| address                 | api_generate_address               | GET           |
| tag/hashes              | api_find_transactions_by_tag       | GET           |
| tag/object              | api_find_transactions_obj_by_tag   | GET           |
| transaction             | api_find_transaction_object_single | GET           |
| transaction/object      | api_find_transaction_objects       | POST          |
| transaction/send        | api_send_transfer                  | POST          |
| tips/all                | api_get_tips                       | GET           |
| tips/pair               | api_get_tips_pair                  | GET           |
| tryte                   | api_send_trytes                    | POST          |

## API request format
APIs in POST method have almost the same format as MQTT requests have, there are one more field, `device_id` in MQTT requests.
However, APIs in GET method would in a more different format, so the following are the examples of the requests of these APIs.

### api_generate_address
```json
{"device_id":"<device_id>"}
```
### api_find_transactions_by_tag
```json
{"device_id":"<device_id>", "tag":"<tag>"}
```
### api_find_transactions_obj_by_tag
```json
{"device_id":"<device_id>", "tag":"<tag>"}
```
### api_find_transaction_object_single
```json
{"device_id":"<device_id>", "hash":"<transaction hash>"}
```
### api_find_transaction_objects
```json
{"device_id":"<device_id>", "hash":["<transaction hash 1>", "<transaction hash 2>", "..."]}
```
### api_get_tips
```json
{"device_id":"<device_id>"}
```
### api_get_tips_pair
```json
{"device_id":"<device_id>"}
```
### api_send_trytes
```json
{"device_id":"<device_id>", "trytes":"<trytes>"}
```

## Examples
Here is an example which uses mosquitto client to publish requests.
```
mosquitto_pub -h <Broker IP> -t root/topics/tips/all -m "{\"device_id\":\"<device_id>\"}"
```
