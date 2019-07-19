#ifndef SUB_UTILS_H
#define SUB_UTILS_H

#include "client_common.h"
#include "third_party/mosquitto/lib/mosquitto.h"

void signal_handler_func(int signum);
void publish_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties);
void message_callback_sub_func(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message,
                               const mosquitto_property *properties);
void connect_callback_sub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties);
void subscribe_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
void log_callback_sub_func(struct mosquitto *mosq, void *obj, int level, const char *str);

#endif