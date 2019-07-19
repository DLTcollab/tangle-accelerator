#include "client_common.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WITH_SOCKS
static int mosquitto__parse_socks_url(mosq_config_t *cfg, char *url);
#endif

void init_mosq_config(mosq_config_t *cfg, client_type_t client_type) {
  memset(cfg, 0, sizeof(mosq_config_t));
  cfg->general_config = (mosq_general_config_t *)malloc(sizeof(mosq_general_config_t));
  cfg->property_config = (mosq_property_config_t *)malloc(sizeof(mosq_property_config_t));

#ifdef WITH_TLS
  cfg->tls_config = (mosq_tls_config_t *)malloc(sizeof(mosq_tls_config_t));
#endif

#ifdef WITH_SOCKS
  cfg->socks_config = (mosq_socks_config_t *)malloc(sizeof(mosq_socks_config_t));
#endif

  if (client_type == client_pub || client_type == client_duplex) {
    cfg->pub_config = (mosq_pub_config_t *)malloc(sizeof(mosq_pub_config_t));
    cfg->pub_config->repeat_delay.tv_sec = 0;
    cfg->pub_config->repeat_delay.tv_usec = 0;
    cfg->pub_config->first_publish = true;
    cfg->pub_config->disconnect_sent = false;
    cfg->pub_config->ready_for_repeat = false;
  }
  if (client_type = client_sub || client_type == client_duplex) {
    cfg->sub_config = (mosq_sub_config_t *)malloc(sizeof(mosq_sub_config_t));
  }

  cfg->general_config->port = -1;
  cfg->general_config->max_inflight = 20;
  cfg->general_config->keepalive = 60;
  cfg->general_config->clean_session = true;
  cfg->general_config->protocol_version = MQTT_PROTOCOL_V311;
  cfg->general_config->client_type = client_type;
}

void mosq_config_cleanup(mosq_config_t *cfg) {
  if (cfg->general_config->client_type == client_pub || cfg->general_config->client_type == client_duplex) {
    free(cfg->pub_config->message);
    free(cfg->pub_config->topic);
    free(cfg->pub_config->response_topic);
  }
  if (cfg->general_config->client_type = client_sub || cfg->general_config->client_type == client_duplex) {
    if (cfg->sub_config->topics) {
      for (int i = 0; i < cfg->sub_config->topic_count; i++) {
        free(cfg->sub_config->topics[i]);
      }
      free(cfg->sub_config->topics);
    }
    if (cfg->sub_config->filter_outs) {
      for (int i = 0; i < cfg->sub_config->filter_out_count; i++) {
        free(cfg->sub_config->filter_outs[i]);
      }
      free(cfg->sub_config->filter_outs);
    }
    if (cfg->sub_config->unsub_topics) {
      for (int i = 0; i < cfg->sub_config->unsub_topic_count; i++) {
        free(cfg->sub_config->unsub_topics[i]);
      }
      free(cfg->sub_config->unsub_topics);
    }
  }
  free(cfg->general_config->id);
  free(cfg->general_config->id_prefix);
  free(cfg->general_config->host);
  free(cfg->general_config->bind_address);
  free(cfg->general_config->username);
  free(cfg->general_config->password);
  free(cfg->general_config->will_topic);
  free(cfg->general_config->will_payload);

#ifdef WITH_TLS
  free(cfg->tls_config->cafile);
  free(cfg->tls_config->capath);
  free(cfg->tls_config->certfile);
  free(cfg->tls_config->keyfile);
  free(cfg->tls_config->ciphers);
  free(cfg->tls_config->tls_alpn);
  free(cfg->tls_config->tls_version);
  free(cfg->tls_config->tls_engine);
  free(cfg->tls_config->tls_engine_kpass_sha1);
  free(cfg->tls_config->keyform);
#ifdef FINAL_WITH_TLS_PSK
  free(cfg->tls_config->psk);
  free(cfg->tls_config->psk_identity);
#endif
#endif

#ifdef WITH_SOCKS
  free(cfg->socks_config->socks5_host);
  free(cfg->socks_config->socks5_username);
  free(cfg->socks_config->socks5_password);
#endif
  mosquitto_property_free_all(&cfg->property_config->connect_props);
  mosquitto_property_free_all(&cfg->property_config->publish_props);
  mosquitto_property_free_all(&cfg->property_config->subscribe_props);
  mosquitto_property_free_all(&cfg->property_config->unsubscribe_props);
  mosquitto_property_free_all(&cfg->property_config->disconnect_props);
  mosquitto_property_free_all(&cfg->property_config->will_props);
}

rc_mosq_retcode_t cfg_add_topic(mosq_config_t *cfg, client_type_t client_type, char *topic) {
  if (mosquitto_validate_utf8(topic, strlen(topic))) {
    fprintf(stderr, "Error: Malformed UTF-8 in topic argument.\n\n");
    return RC_MOS_ADD_TOPIC;
  }
  if (client_type == client_pub || client_type == client_duplex) {
    if (mosquitto_pub_topic_check(topic) == MOSQ_ERR_INVAL) {
      fprintf(stderr, "Error: Invalid publish topic '%s', does it contain '+' or '#'?\n", topic);
      return RC_MOS_ADD_TOPIC;
    }
    cfg->pub_config->topic = strdup(topic);
  } else if (client_type == client_duplex) {
    if (mosquitto_pub_topic_check(topic) == MOSQ_ERR_INVAL) {
      fprintf(stderr, "Error: Invalid response topic '%s', does it contain '+' or '#'?\n", topic);
      return RC_MOS_ADD_TOPIC;
    }
    cfg->pub_config->response_topic = strdup(topic);
  } else {
    if (mosquitto_sub_topic_check(topic) == MOSQ_ERR_INVAL) {
      fprintf(stderr, "Error: Invalid subscription topic '%s', are all '+' and '#' wildcards correct?\n", topic);
      return RC_MOS_ADD_TOPIC;
    }
    cfg->sub_config->topic_count++;
    cfg->sub_config->topics = realloc(cfg->sub_config->topics, cfg->sub_config->topic_count * sizeof(char *));
    if (!cfg->sub_config->topics) {
      fprintf(stderr, "Error: Out of memory.\n");
      return RC_MOS_ADD_TOPIC;
    }
    cfg->sub_config->topics[cfg->sub_config->topic_count - 1] = strdup(topic);
  }
  return RC_MOS_OK;
}

rc_mosq_retcode_t mosq_opts_set(struct mosquitto *mosq, mosq_config_t *cfg) {
#if defined(WITH_TLS) || defined(WITH_SOCKS)
  mosq_retcode_t ret;
#endif

  mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, cfg->general_config->protocol_version);

  if (cfg->general_config->will_topic &&
      mosquitto_will_set_v5(mosq, cfg->general_config->will_topic, cfg->general_config->will_payloadlen,
                            cfg->general_config->will_payload, cfg->general_config->will_qos,
                            cfg->general_config->will_retain, cfg->property_config->will_props)) {
    fprintf(stderr, "Error: Problem setting will.\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
  cfg->property_config->will_props = NULL;

  if (cfg->general_config->username &&
      mosquitto_username_pw_set(mosq, cfg->general_config->username, cfg->general_config->password)) {
    fprintf(stderr, "Error: Problem setting username and password.\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
#ifdef WITH_TLS
  if (cfg->tls_config->cafile || cfg->tls_config->capath) {
    ret = mosquitto_tls_set(mosq, cfg->tls_config->cafile, cfg->tls_config->capath, cfg->tls_config->certfile,
                            cfg->tls_config->keyfile, NULL);
    if (ret) {
      if (ret == MOSQ_ERR_INVAL) {
        fprintf(stderr, "Error: Problem setting TLS options: File not found.\n");
      } else {
        fprintf(stderr, "Error: Problem setting TLS options: %s.\n", mosquitto_strerror(ret));
      }
      mosquitto_lib_cleanup();
      return ret;
    }
  }
  if (cfg->tls_config->insecure && mosquitto_tls_insecure_set(mosq, true)) {
    fprintf(stderr, "Error: Problem setting TLS insecure option.\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
  if (cfg->tls_config->tls_engine && mosquitto_string_option(mosq, MOSQ_OPT_TLS_ENGINE, cfg->tls_config->tls_engine)) {
    fprintf(stderr, "Error: Problem setting TLS engine, is %s a valid engine?\n", cfg->tls_config->tls_engine);
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
  if (cfg->tls_config->keyform && mosquitto_string_option(mosq, MOSQ_OPT_TLS_KEYFORM, cfg->tls_config->keyform)) {
    fprintf(stderr, "Error: Problem setting key form, it must be one of 'pem' or 'engine'.\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
  if (cfg->tls_config->tls_engine_kpass_sha1 &&
      mosquitto_string_option(mosq, MOSQ_OPT_TLS_ENGINE_KPASS_SHA1, cfg->tls_config->tls_engine_kpass_sha1)) {
    fprintf(stderr, "Error: Problem setting TLS engine key pass sha, is it a 40 character hex string?\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
  if (cfg->tls_config->tls_alpn && mosquitto_string_option(mosq, MOSQ_OPT_TLS_ALPN, cfg->tls_config->tls_alpn)) {
    fprintf(stderr, "Error: Problem setting TLS ALPN protocol.\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
#ifdef FINAL_WITH_TLS_PSK
  if (cfg->tls_config->psk && mosquitto_tls_psk_set(mosq, cfg->tls_config->psk, cfg->tls_config->psk_identity, NULL)) {
    fprintf(stderr, "Error: Problem setting TLS-PSK options.\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
#endif
  if ((cfg->tls_config->tls_version || cfg->tls_config->ciphers) &&
      mosquitto_tls_opts_set(mosq, 1, cfg->tls_config->tls_version, cfg->tls_config->ciphers)) {
    fprintf(stderr, "Error: Problem setting TLS options, check the options are valid.\n");
    mosquitto_lib_cleanup();
    return RC_MOS_OPT_SET;
  }
#endif
  mosquitto_max_inflight_messages_set(mosq, cfg->general_config->max_inflight);
#ifdef WITH_SOCKS
  if (cfg->socks_config->socks5_host) {
    ret = mosquitto_socks5_set(mosq, cfg->socks_config->socks5_host, cfg->socks_config->socks5_port,
                               cfg->socks_config->socks5_username, cfg->socks_config->socks5_password);
    if (ret) {
      mosquitto_lib_cleanup();
      return ret;
    }
  }
#endif
  return RC_MOS_OK;
}

rc_mosq_retcode_t generate_client_id(mosq_config_t *cfg) {
  if (cfg->general_config->id_prefix) {
    cfg->general_config->id = malloc(strlen(cfg->general_config->id_prefix) + 10);
    if (!cfg->general_config->id) {
      fprintf(stderr, "Error: Out of memory.\n");
      mosquitto_lib_cleanup();
      return RC_MOS_GEN_ID;
    }
    snprintf(cfg->general_config->id, strlen(cfg->general_config->id_prefix) + 10, "%s%d",
             cfg->general_config->id_prefix, getpid());
  }
  return RC_MOS_OK;
}

rc_mosq_retcode_t mosq_client_connect(struct mosquitto *mosq, mosq_config_t *cfg) {
  char *err;
  rc_mosq_retcode_t ret = MOSQ_ERR_SUCCESS;
  int port;

  if (cfg->general_config->port < 0) {
#ifdef WITH_TLS
    if (cfg->tls_config->cafile || cfg->tls_config->capath
#ifdef FINAL_WITH_TLS_PSK
        || cfg->tls_config->psk
#endif
    ) {
      port = 8883;
    } else
#endif
    {
      port = 1883;
    }
  } else {
    port = cfg->general_config->port;
  }

#ifdef WITH_SRV
  if (cfg->use_srv) {
    ret = mosquitto_connect_srv(mosq, cfg->general_config->host, cfg->general_config->keepalive,
                                cfg->general_config->bind_address);
  } else {
    ret = mosquitto_connect_bind_v5(mosq, cfg->general_config->host, port, cfg->general_config->keepalive,
                                    cfg->general_config->bind_address, cfg->property_config->connect_props);
  }
#else
  ret = mosquitto_connect_bind_v5(mosq, cfg->general_config->host, port, cfg->general_config->keepalive,
                                  cfg->general_config->bind_address, cfg->property_config->connect_props);
#endif
  if (ret > 0) {
    {
      if (ret == MOSQ_ERR_ERRNO) {
        err = strerror(errno);
        fprintf(stderr, "Error: %s\n", err);
      } else {
        fprintf(stderr, "Unable to connect (%s).\n", mosquitto_strerror(ret));
      }
    }
    mosquitto_lib_cleanup();
    ret = RC_CLIENT_CONNTECT;
  }
  return ret;
}

#ifdef WITH_SOCKS
/* Convert %25 -> %, %3a, %3A -> :, %40 -> @ */
static int mosquitto__urldecode(char *str) {
  int i, j;
  int len;
  if (!str) return 0;

  if (!strethr(str, '%')) return 0;

  len = strlen(str);
  for (i = 0; i < len; i++) {
    if (str[i] == '%') {
      if (i + 2 >= len) {
        return EXIT_FAILURE;
      }
      if (str[i + 1] == '2' && str[i + 2] == '5') {
        str[i] = '%';
        len -= 2;
        for (j = i + 1; j < len; j++) {
          str[j] = str[j + 2];
        }
        str[j] = '\0';
      } else if (str[i + 1] == '3' && (str[i + 2] == 'A' || str[i + 2] == 'a')) {
        str[i] = ':';
        len -= 2;
        for (j = i + 1; j < len; j++) {
          str[j] = str[j + 2];
        }
        str[j] = '\0';
      } else if (str[i + 1] == '4' && str[i + 2] == '0') {
        str[i] = ':';
        len -= 2;
        for (j = i + 1; j < len; j++) {
          str[j] = str[j + 2];
        }
        str[j] = '\0';
      } else {
        return EXIT_FAILURE;
      }
    }
  }
  return 0;
}

static int mosquitto__parse_socks_url(mosq_config_t *cfg, char *url) {
  char *str;
  size_t i;
  char *username = NULL, *password = NULL, *host = NULL, *port = NULL;
  char *username_or_host = NULL;
  size_t start;
  size_t len;
  bool have_auth = false;
  int port_int;

  if (!strncmp(url, "socks5h://", strlen("socks5h://"))) {
    str = url + strlen("socks5h://");
  } else {
    fprintf(stderr, "Error: Unsupported proxy protocol: %s\n", url);
    return EXIT_FAILURE;
  }

  // socks5h://username:password@host:1883
  // socks5h://username:password@host
  // socks5h://username@host:1883
  // socks5h://username@host
  // socks5h://host:1883
  // socks5h://host

  start = 0;
  for (i = 0; i < strlen(str); i++) {
    if (str[i] == ':') {
      if (i == start) {
        goto cleanup;
      }
      if (have_auth) {
        /* Have already seen a @ , so this must be of form
         * socks5h://username[:password]@host:port */
        if (host) {
          /* Already seen a host, must be malformed. */
          goto cleanup;
        }
        len = i - start;
        host = malloc(len + 1);
        if (!host) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(host, &(str[start]), len);
        host[len] = '\0';
        start = i + 1;
      } else if (!username_or_host) {
        /* Haven't seen a @ before, so must be of form
         * socks5h://host:port or
         * socks5h://username:password@host[:port] */
        len = i - start;
        username_or_host = malloc(len + 1);
        if (!username_or_host) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(username_or_host, &(str[start]), len);
        username_or_host[len] = '\0';
        start = i + 1;
      }
    } else if (str[i] == '@') {
      if (i == start) {
        goto cleanup;
      }
      have_auth = true;
      if (username_or_host) {
        /* Must be of form socks5h://username:password@... */
        username = username_or_host;
        username_or_host = NULL;

        len = i - start;
        password = malloc(len + 1);
        if (!password) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(password, &(str[start]), len);
        password[len] = '\0';
        start = i + 1;
      } else {
        /* Haven't seen a : yet, so must be of form
         * socks5h://username@... */
        if (username) {
          /* Already got a username, must be malformed. */
          goto cleanup;
        }
        len = i - start;
        username = malloc(len + 1);
        if (!username) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(username, &(str[start]), len);
        username[len] = '\0';
        start = i + 1;
      }
    }
  }

  /* Deal with remainder */
  if (i > start) {
    len = i - start;
    if (host) {
      /* Have already seen a @ , so this must be of form
       * socks5h://username[:password]@host:port */
      port = malloc(len + 1);
      if (!port) {
        fprintf(stderr, "Error: Out of memory.\n");
        goto cleanup;
      }
      memcpy(port, &(str[start]), len);
      port[len] = '\0';
    } else if (username_or_host) {
      /* Haven't seen a @ before, so must be of form
       * socks5h://host:port */
      host = username_or_host;
      username_or_host = NULL;
      port = malloc(len + 1);
      if (!port) {
        fprintf(stderr, "Error: Out of memory.\n");
        goto cleanup;
      }
      memcpy(port, &(str[start]), len);
      port[len] = '\0';
    } else {
      host = malloc(len + 1);
      if (!host) {
        fprintf(stderr, "Error: Out of memory.\n");
        goto cleanup;
      }
      memcpy(host, &(str[start]), len);
      host[len] = '\0';
    }
  }

  if (!host) {
    fprintf(stderr, "Error: Invalid proxy.\n");
    goto cleanup;
  }

  if (mosquitto__urldecode(username)) {
    goto cleanup;
  }
  if (mosquitto__urldecode(password)) {
    goto cleanup;
  }
  if (port) {
    port_int = atoi(port);
    if (port_int < 1 || port_int > 65535) {
      fprintf(stderr, "Error: Invalid proxy port %d\n", port_int);
      goto cleanup;
    }
    free(port);
  } else {
    port_int = 1080;
  }

  cfg->socks_config->socks5_username = username;
  cfg->socks_config->socks5_password = password;
  cfg->socks_config->socks5_host = host;
  cfg->socks_config->socks5_port = port_int;

  return 0;
cleanup:
  if (username_or_host) free(username_or_host);
  if (username) free(username);
  if (password) free(password);
  if (host) free(host);
  if (port) free(port);
  return EXIT_FAILURE;
}
#endif
