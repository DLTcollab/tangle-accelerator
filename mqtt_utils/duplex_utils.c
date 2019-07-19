#include "duplex_utils.h"
#include <errno.h>
#include <string.h>
#include "pub_utils.h"
#include "sub_utils.h"

rc_mosq_retcode_t duplex_config_init(struct mosquitto **config_mosq, mosq_config_t *config_cfg) {
  rc_mosq_retcode_t ret = RC_MOS_OK;

  init_mosq_config(config_cfg, client_duplex);
  mosquitto_lib_init();

  if (generate_client_id(config_cfg)) {
    return RC_MOS_INIT_ERROR;
  }

  init_check_error(config_cfg, client_pub);

  *config_mosq = mosquitto_new(config_cfg->general_config->id, true, NULL);
  if (!config_mosq) {
    switch (errno) {
      case ENOMEM:
        fprintf(stderr, "Error: Out of memory.\n");
        break;
      case EINVAL:
        fprintf(stderr, "Error: Invalid id.\n");
        break;
    }
    return RC_MOS_INIT_ERROR;
  }

  if (mosq_opts_set(*config_mosq, config_cfg)) {
    return RC_MOS_INIT_ERROR;
  }
}

rc_mosq_retcode_t gossip_channel_set(mosq_config_t *channel_cfg, char *host, char *sub_topic, char *pub_topic) {
  rc_mosq_retcode_t ret = RC_MOS_OK;

  channel_cfg->general_config->host = strdup(host);
  channel_cfg->general_config->client_type = client_pub;
  if (cfg_add_topic(channel_cfg, client_sub, sub_topic)) {
    ret = RC_MOS_CHANNEL_SETTING;
    goto done;
  }
  if (cfg_add_topic(channel_cfg, client_pub, pub_topic)) {
    ret = RC_MOS_CHANNEL_SETTING;
  }

done:
  return ret;
}

rc_mosq_retcode_t gossip_message_set(mosq_config_t *channel_cfg, char *message) {
  rc_mosq_retcode_t ret = RC_MOS_OK;

  channel_cfg->pub_config->message = strdup(message);
  channel_cfg->pub_config->msglen = strlen(channel_cfg->pub_config->message);
  channel_cfg->pub_config->pub_mode = MSGMODE_CMD;

  return ret;
}

rc_mosq_retcode_t duplex_loop(struct mosquitto *loop_mosq, mosq_config_t *loop_cfg) {
  rc_mosq_retcode_t ret = MOSQ_ERR_SUCCESS;

  loop_cfg->general_config->client_type = client_sub;
  ret = mosq_client_connect(loop_mosq, loop_cfg);
  if (ret) {
    goto done;
  }

  ret = mosquitto_loop_forever(loop_mosq, -1, 1);
  if (ret) {
    goto done;
  }

  loop_cfg->general_config->client_type = client_pub;
  ret = mosq_client_connect(loop_mosq, loop_cfg);
  if (ret) {
    goto done;
  }
  ret = publish_loop(loop_mosq, loop_cfg);

done:
  return ret;
}