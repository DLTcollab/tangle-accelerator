/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "shell.h"
#include "cipher.h"
#include "endpoint/platform/impl.h"

// Legato headers
#include "interfaces.h"
#include "legato.h"

// Linux headers
#include <errno.h>    // Error integer and strerror() function
#include <fcntl.h>    // Contains file controls like O_RDWR
#include <termios.h>  // Contains POSIX terminal control definitions
#include <unistd.h>   // write(), read(), close()

// cURL headers
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/ta_errors.h"

static int sh_ok(char **argv);
static int sh_help(char **argv);
static int sh_ecm(char **argv);
static int sh_radio(char **argv);
static int sh_ping(char **argv);
static int sh_dns(char **argv);
static int sh_route(char **argv);
static int sh_at(char **argv);
static int sh_transaction(char **argv);
static int sh_connect(char **argv);
static int sh_setup_private_key(char **argv);

static int sh_launch(char **args);
static int open_serialport(char *dev, speed_t baudrate);

/* List of builtin commands */
static command builtin_cmd[] = {
    {"ok", "OK", sh_ok},
    {"help", "print the help message", sh_help},
    {"ecm", "enable/disable ssh login [on/off]", sh_ecm},
    {"radio", "show modem connection status", sh_radio},
    {"ping", "ping the FQDN or ip host [FQDN or ip]", sh_ping},
    {"dns", "show the current dns address", sh_dns},
    {"route", "show the current route", sh_route},
    {"AT", "Change to AT shell [Command]", sh_at},
    {"trans", "Send the test transaction [host] [port]", sh_transaction},
    {"connect", "Connect modem to network", sh_connect},
    {"set-private-key", "Setup private key into secure storage [private-key]", sh_setup_private_key},
};

static char *test_post_data =
    "{\"value\": 0, \"message\": "
    "\"ZBCDKDTCFDTCSCEAQCMDEAHDPCBDVC9DTCRAPCRCRCTC9DTCFDPCHDCDFD\", "
    "\"tag\": \"POWEREDBYTANGLEACCELERATOR9\", \"address\": "
    "\"POWEREDBYTANGLEACCELERATOR999999999999999999999999999999999999999999"
    "9999999999999\"}";

#define BUILTN_COMMAND_LEN NUM_ARRAY_MEMBERS(builtin_cmd)

// UART file descriptor
static int uart_fd;
// AT device file descriptor
static int at_fd;

/**
   @brief Write the message to file descriptor
   @param[in] fd File descriptor
   @param[in] msg Message to write
   @return
   - SC_OK on success
   - SC_ENDPOINT_UART_WRITE_ERROR on failed
 */
status_t uart_write(int fd, char *msg) {
  if (write(fd, msg, strlen(msg)) == -1) {
    LE_ERROR("Failed when writing message to UART: %s", msg);
    return SC_ENDPOINT_UART_WRITE_ERROR;
  }
  LE_INFO("Write to host: %s", msg);
  tcdrain(fd);
  return SC_OK;
}

#define READ_BUFFER_SIZE 4096

/**
   @brief Read the message to file descriptor
   @param[in] fd File descriptor
   @return
   - char* on success(should be freed after usage)
   - NULL on failed
 */
static char *uart_read(int fd) {
  fd_set rset;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500;
  while (true) {
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    select(fd + 1, &rset, NULL, NULL, &tv);

    if (FD_ISSET(fd, &rset)) {
      char read_buffer[READ_BUFFER_SIZE];
      memset(read_buffer, 0, READ_BUFFER_SIZE);
      if (read(fd, &read_buffer, READ_BUFFER_SIZE) == -1) {
        LE_ERROR("Failed when reading from UART");
        return NULL;
      }
      LE_INFO("Command: %s", read_buffer);
      return strndup(read_buffer, READ_BUFFER_SIZE);
    }
  }
}

/**
   @brief Builtin command: print ok
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_ok(char **argv) {
  printf("OK\n");
  return 1;
}

/**
   @brief Builtin command: print help
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_help(char **argv) {
  command *ptr = builtin_cmd;
  for (int i = 0; i < BUILTN_COMMAND_LEN; i++) {
    printf("%s: %s\n", ptr[i].name, ptr[i].desc);
  }
  return 1;
}

/**
   @brief Builtin command: enable/disable ecm ssh login interface
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_ecm(char **argv) {
  if (argv[1] != NULL) {
    if (strncmp(argv[1], "on", strlen("on")) == 0) {
      // enable ssh
      char *args[] = {"/sbin/ifconfig", "ecm0", "192.168.2.2", "netmask", "255.255.255.0", "up", NULL};
      sh_launch(args);
    } else if (strncmp(argv[1], "off", strlen("off")) == 0) {
      // disable ssh
      char *ecm_addr[] = {"/sbin/ifconfig", "ecm0", "0.0.0.0", NULL};
      sh_launch(ecm_addr);
      char *ecm_down[] = {"/sbin/ifconfig", "ecm0", "down", NULL};
      sh_launch(ecm_down);
    }
  } else {
    printf("only the [on/off] arguments can be used\n");
  }
  return 1;
}

/**
   @brief Builtin command: show radio status
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_radio(char **argv) {
  char *data_info[] = {"/legato/systems/current/bin/cm", "data", NULL};
  sh_launch(data_info);
  char *radio_info[] = {"/legato/systems/current/bin/cm", "radio", NULL};
  sh_launch(radio_info);
  char *cm_info[] = {"/legato/systems/current/bin/cm", "info", NULL};
  sh_launch(cm_info);
  return 1;
}

/**
   @brief Builtin command: ping the host
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_ping(char **argv) {
  char *args[] = {"ping", argv[1], "-c", "4", NULL};
  return sh_launch(args);
}

/**
   @brief Builtin command: show the dns setting
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_dns(char **argv) {
  char *args[] = {"cat", "/etc/resolv.conf", NULL};
  return sh_launch(args);
}

/**
   @brief Builtin command: show the route setting
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_route(char **argv) {
  char *args[] = {"/sbin/route", "-n", NULL};
  return sh_launch(args);
}

/**
   @brief Builtin command: send the at command to AT device
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_at(char **argv) {
  if (argv[1] != NULL) {
    size_t at_len = strlen(argv[1]);
    char *at_cmd = malloc(at_len + 1);
    memset(at_cmd, 0, at_len + 1);
    memcpy(at_cmd, argv[1], at_len);
    at_cmd[at_len] = '\n';
    uart_write(at_fd, at_cmd);
    char *msg = uart_read(at_fd);
    uart_write(uart_fd, msg);
    free(msg);
    free(at_cmd);
  }
  return 1;
}

/**
   @brief Send a transfer test to host
   @param[in] host Host to send
   @param[in] port Port to send
   @return Always returns 1, to continue executing
 */
static int send_transaction_test(char *host, char *port) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    size_t url_len = strlen(host) + strlen(port) + 21;

    char *url = malloc(url_len);
    if (url == NULL) {
      LE_ERROR("Failed to allocated memory");
      return 1;
    }

    snprintf(url, url_len, "http://%s:%s/transaction", host, port);

    LE_INFO("Send transaction: %s", url);
    if (url != NULL) {
      curl_easy_setopt(curl, CURLOPT_URL, url);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, test_post_data);

      /* Perform the request, res will get the return code */
      res = curl_easy_perform(curl);
      /* Check for errors */
      if (res != CURLE_OK) fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
      free(url);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 1;
}

/**
   @brief Builtin command: send a test transfer
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_transaction(char **argv) {
  if (argv[1] == NULL || argv[2] == NULL) {
    fprintf(stderr, "Should enter host and port\n");
    return 1;
  }

  LE_INFO("Host: %s, Port: %s", argv[1], argv[2]);
  return send_transaction_test(argv[1], argv[2]);
}

/**
   @brief Builtin command: connect to LTE network
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_connect(char **argv) {
  char *args[] = {"/legato/systems/current/bin/cm", "data", "connect", "&", NULL};
  return sh_launch(args);
}

/**
   @brief Builtin command: Setup or update private key of endpoint device
   @param[in] argv List of args
   @return Always returns 1, to continue executing
 */
static int sh_setup_private_key(char **argv) {
  char *private_key = argv[1];
  if (private_key == NULL) {
    fprintf(stderr, "The private key should be entered\n");
    return 1;
  }

  if (strlen(private_key) != AES_CBC_KEY_SIZE) {
    fprintf(stderr, "The private key length should be %d bytes of string\n", AES_CBC_KEY_SIZE);
    return 1;
  }

  if (set_device_key(private_key) != SC_OK) {
    fprintf(stderr, "Failed to set private key\n");
    return 1;
  }

  fprintf(stdout, "Finish to update new device key\n");
  return 1;
}

/**
   @brief Launch program
   @param[in] args Null terminated list of arguments
   @return 1 if the shell should continue running, 0 if it should terminate
 */
static int sh_launch(char **args) {
  pid_t pid;
  int status;

  switch (pid = fork()) {
    case 0:
      // Child process
      if (execvp(args[0], args) == -1) {
        printf("%s: %s\n", args[0], strerror(errno));
      }
      exit(EXIT_FAILURE);
    case -1:
      printf("Fork error when execute command: %s", args[0]);
      LE_ERROR("Stop to execute the endpoint shell: Fork error");
      return 0;  // Return 0 to stop the shell
    default:
      // Parent process
      do {
        waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
      break;
  }
  return 1;
}

/**
   @brief Execute shell built-in or launch program
   @param[in] args Null terminated list of arguments
   @return 1 if the shell should continue running, 0 if it should terminate
 */
static int sh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  command *ptr = builtin_cmd;
  for (i = 0; i < BUILTN_COMMAND_LEN; i++) {
    if (strcmp(args[0], ptr[i].name) == 0) {
      return (*ptr[i].cmd)(args);
    }
  }

  fprintf(stderr, "No such command: %s\n", args[0]);
  return 1;
}

/**
   @brief Read a line of input from UART
   @return The line from UART
 */
static char *sh_read_line(void) { return uart_read(uart_fd); }

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/**
   @brief Split a line into tokens
   @param[in] line The line.
   @return Null-terminated array of tokens.
 */
static char **sh_split_line(char *line) {
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
static void sh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    uart_write(uart_fd, "> ");
    line = sh_read_line();
    args = sh_split_line(line);
    status = sh_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Initialize the UART interface
 */
static int sh_init(void) {
  uart_fd = open_serialport(SHELL_USING_DEVICE, UART_BAUDRATE);
  uart_write(uart_fd, "Engineer mode start\n");
  dup2(uart_fd, 1);
  dup2(uart_fd, 2);

  at_fd = open_serialport(ATCMD_USING_DEVICE, AT_BAUDRATE);
  return 0;
}

/**
   @brief Set the file descriptor attributes
   @param[in] fd File descriptor
   @param[in] speed Baudrate
   @return #status_t
 */
static status_t set_interface_attribs(int fd, speed_t speed) {
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    LE_ERROR("Error from tcgetattr: %s\n", strerror(errno));
    return SC_ENDPOINT_UART_SET_ATTR;
  }

  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    LE_ERROR("Error from tcsetattr: %s\n", strerror(errno));
    return SC_ENDPOINT_UART_SET_ATTR;
  }
  return SC_OK;
}

/**
   @brief Open the UART serial device
   @param[in] dev UART device
   @param[in] baudrate Baudrate
   @return file descriptor
 */
static int open_serialport(char *dev, speed_t baudrate) {
  int fd;

  fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    perror("Open serial port");
  }

  set_interface_attribs(fd, baudrate);
  return fd;
}

static void release_uart(void) {
  close(uart_fd);
  close(at_fd);
}

COMPONENT_INIT {
  atexit(release_uart);
  sh_init();
  sh_loop();
  exit(EXIT_SUCCESS);
}
