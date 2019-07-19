#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include <stdio.h>
#include <sys/time.h>
#include "third_party/mosquitto/lib/mosquitto.h"

/* pub_client.c modes */
#define MSGMODE_CMD 1

#define EXIT_FAILURE 1

#define HOST "140.116.82.61"
#define TOPIC "NB/test/room1"
#define TOPIC_RES "NB/test/room2"
#define MESSAGE "this is the testing message, so say hi to me"

typedef enum client_type_s { client_pub, client_sub, client_duplex } client_type_t;

typedef enum mosq_err_t mosq_retcode_t;  // typedef the original enum
// out own return error code for MQTT layer
typedef enum mosq_retcode_s {
  RC_MOS_OK,
  RC_MOS_INIT_ERROR,
  RC_MOS_CRASH,
  RC_MOS_CHANNEL_SETTING,
  RC_MOS_MESSAGE_SETTING,
  RC_MOS_ADD_TOPIC,
  RC_MOS_OPT_SET,
  RC_MOS_GEN_ID,
  RC_CLIENT_CONNTECT,
} rc_mosq_retcode_t;

typedef struct mosq_general_config_s {
  char *id;
  char *id_prefix;
  int protocol_version;
  int keepalive;
  char *host;
  int port;
  int qos;
  int last_mid;
  bool retain;
  client_type_t client_type;
#ifdef WITH_SRV
  bool use_srv;
#endif
  bool debug;
  unsigned int max_inflight;
  char *username;
  char *password;
  char *will_topic;
  char *will_payload;
  long will_payloadlen;
  int will_qos;
  bool will_retain;
  char *bind_address;
  bool clean_session;
} mosq_general_config_t;

typedef struct mosq_pub_config_s {
  int mid_sent;                   /* pub, rr */
  int pub_mode;                   /* pub, rr */
  char *message;                  /* pub, rr */
  long msglen;                    /* pub, rr */
  char *topic;                    /* pub, rr */
  struct timeval repeat_delay;    /* pub */
  struct timeval next_publish_tv; /* pub, rr */
  bool first_publish;             /* pub, rr */
  bool disconnect_sent;           /* pub, rr */
  bool ready_for_repeat;          /* pub, rr */
  bool have_topic_alias;          /* pub */
  char *response_topic;           /* rr */
} mosq_pub_config_t;

typedef struct mosq_sub_config_s {
  char **topics;         /* sub */
  int topic_count;       /* sub */
  bool exit_after_sub;   /* sub */
  bool no_retain;        /* sub */
  bool retained_only;    /* sub */
  bool remove_retained;  /* sub */
  char **filter_outs;    /* sub */
  int filter_out_count;  /* sub */
  char **unsub_topics;   /* sub */
  int unsub_topic_count; /* sub */
  int timeout;           /* sub */
  int sub_opts;          /* sub */
} mosq_sub_config_t;

typedef struct mosq_property_config_s {
  mosquitto_property *connect_props;
  mosquitto_property *publish_props;
  mosquitto_property *subscribe_props;
  mosquitto_property *unsubscribe_props;
  mosquitto_property *disconnect_props;
  mosquitto_property *will_props;
} mosq_property_config_t;

#ifdef WITH_TLS
typedef struct mosq_tls_config_s {
  char *cafile;
  char *capath;
  char *certfile;
  char *keyfile;
  char *ciphers;
  bool insecure;
  char *tls_alpn;
  char *tls_version;
  char *tls_engine;
  char *tls_engine_kpass_sha1;
  char *keyform;
#ifdef FINAL_WITH_TLS_PSK
  char *psk;
  char *psk_identity;
#endif
} mosq_tls_config_t;
#endif

#ifdef WITH_SOCKS
typedef struct mosq_socks_config_s {
  char *socks5_host;
  int socks5_port;
  char *socks5_username;
  char *socks5_password;
} mosq_socks_config_t;
#endif

typedef struct mosq_config_s {
  mosq_general_config_t *general_config;
  mosq_pub_config_t *pub_config;
  mosq_sub_config_t *sub_config;
  mosq_property_config_t *property_config;
#ifdef WITH_TLS
  mosq_tls_config_t *tls_config;
#endif

#ifdef WITH_SOCKS
  mosq_socks_config_t *socks_config;
#endif
} mosq_config_t;

void mosq_config_cleanup(mosq_config_t *cfg);
void init_mosq_config(mosq_config_t *cfg, client_type_t client_type);
rc_mosq_retcode_t mosq_opts_set(struct mosquitto *mosq, mosq_config_t *cfg);
rc_mosq_retcode_t generate_client_id(mosq_config_t *cfg);
rc_mosq_retcode_t mosq_client_connect(struct mosquitto *mosq, mosq_config_t *cfg);
rc_mosq_retcode_t cfg_add_topic(mosq_config_t *cfg, client_type_t client_type, char *topic);

#endif
