#ifndef DUPLEX_CALLBACK_H
#define DUPLEX_CALLBACK_H
#include "client_common.h"

rc_mosq_retcode_t duplex_callback_func_set(struct mosquitto *mosq, mosq_config_t *cfg);

#endif