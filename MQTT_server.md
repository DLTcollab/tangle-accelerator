# Introduction
The structure of this information deisseminating network is based on MQTT protocol which consists of three components. `Connectivity Endpoint`, `tangle-accelerator` and `Parser`

### 1. Connectivity Endpoint
This is a client implemented with any possible lightweight computing node (such as Raspberry Pi) and a communication module (i.e. NB-IoT module). There are some modems allowing users to send MQTT message with AT-command-like command, so for these kinds of modems, they don't need to run a `mosquitto`-dependent programs on the lightweight computing node.
For the `Connectivity Endpoint` which uses a modem provides AT-command-like commands, few things they need to do are choosing the right topic (we use different topics to simulate RESTful methodology) and serializing the data/message into the demanded format which contains `Device ID` for TA requests. After the message is serialized into the demanded format, we can send this message with modem provided command as simple as we send a http request with Python.

### 2. tangle-accelerator
TA (tangle-accelerator) plays a role of server which receives requests from communication module and processes the requests. However, under the structure of this MQTT information deisseminating network, both `tangle-accelerator` and `Connectivity Endpoint` are MQTT client. We must take care that `tangle-accelerator` is not a broker under this topology of MQTT information disseminating network.
`tangle-accelerator` runs as a MQTT subscriber and publisher at the same time which listen to the requests on several different topics and respond the requests according to respective request result. We treat each topic as different URL path of http protocol does.

### 3. Parser
The `Parser` plays a role of both subscriber and publisher simultaneously; thus, we will implement this `Parser`, which is a duplex client by modifying `mosquitto`'s client source code.
Sometimes, it might necessitate parsing MQTT messages, since the messages which are sent from multifarious modems may vary from one to another. In order to support a wide range of modems from different manufactures, we can use a duplex client to revise the messages which are sent from modems into the regulated format, then send the message in regulated format to a specific topic which contains only requests follow TA's request format.

## Communication structure
`<root>` is the host operator defined root path. It can be used if we want to choose a certain host of a cluster.

#### 1. Communication Endpoint sends message
Communication endpoint would send message on topic `<root>/<API>/raw/<parser type ID>`
On this topic, the messages are in raw types whose formats depend on the modems users chose.
And the sub-topic, `<parser type ID>`, aims to pass the message to a appropriate parser (the work that parser should 
do is corresponding to multifarious manufacturers or even different modems).

The message should contain the `Device ID` of the `Communication Endpoint`, since the `Device ID` will be used in returning data for `tangle-accelerator`. Once the message is published, Communication Endpoint will start to listen the topic, `<root>/<API>/<Device ID>`. The resonse will be published into this topic
#### 2. Parser parses raw message
Parser parses messages came from topic `<root>/<API>/raw/<parser type ID>` into regulted format of TA.
#### 3. Parser sends neat messages
Parser sends parsed, neat, regulated format messages to topic `<root>/<API>`, and the messages contain `Device ID` of source devices as well.
#### 3. TA receives the message
TA receive the message from topic `<root>/<API>`, and then it starts to process the requests.
The responses will be sent onto topic `<root>/<API>/<Device ID>`.