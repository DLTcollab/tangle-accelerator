#ifndef DUPLEX_UTILS_H
#define DUPLEX_UTILS_H

#include "client_common.h"
#include "third_party/mosquitto/lib/mosquitto.h"

rc_mosq_retcode_t duplex_config_init(struct mosquitto **config_mosq, mosq_config_t *config_cfg);
rc_mosq_retcode_t gossip_channel_set(mosq_config_t *channel_cfg, char *host, char *sub_topic, char *pub_topic);
rc_mosq_retcode_t gossip_message_set(mosq_config_t *channel_cfg, char *message);
rc_mosq_retcode_t duplex_loop(struct mosquitto *loop_mosq, mosq_config_t *loop_cfg);
#endif