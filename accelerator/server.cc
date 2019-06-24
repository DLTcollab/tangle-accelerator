/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <served/methods.hpp>
#include <served/plugins.hpp>
#include <served/served.hpp>
#include "accelerator/apis.h"
#include "accelerator/config.h"
#include "accelerator/errors.h"
#include "cJSON.h"
#include "utils/logger_helper.h"

#define MAIN_LOGGER_ID "main"

static ta_core_t ta_core;
static logger_id_t logger_id;

void set_method_header(served::response& res, http_method_t method) {
  res.set_header("Server", ta_core.info.version);
  res.set_header("Access-Control-Allow-Origin", "*");

  switch (method) {
    case HTTP_METHOD_OPTIONS:
      res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      res.set_header("Access-Control-Allow-Headers", "Origin, Content-Type, Accept");
      res.set_header("Access-Control-Max-Age", "86400");
      break;
    default:
      res.set_header("Content-Type", "application/json");
      break;
  }
}

status_t set_response_content(status_t ret, char** json_result) {
  status_t http_ret;
  if (ret == SC_OK) {
    return SC_HTTP_OK;
  }

  cJSON* json_obj = cJSON_CreateObject();
  switch (ret) {
    case SC_CCLIENT_NOT_FOUND:
    case SC_MAM_NOT_FOUND:
      http_ret = SC_HTTP_NOT_FOUND;
      cJSON_AddStringToObject(json_obj, "message", "Request not found");
      break;
    case SC_CCLIENT_JSON_KEY:
    case SC_MAM_NO_PAYLOAD:
      http_ret = SC_HTTP_BAD_REQUEST;
      cJSON_AddStringToObject(json_obj, "message", "Invalid request header");
      break;
    default:
      http_ret = SC_HTTP_INTERNAL_SERVICE_ERROR;
      cJSON_AddStringToObject(json_obj, "message", "Internal service error");
      break;
  }
  *json_result = cJSON_PrintUnformatted(json_obj);
  return http_ret;
}

int main(int argc, char* argv[]) {
  served::multiplexer mux;
  mux.use_after(served::plugin::access_log);

  if (logger_helper_init() != RC_OK) {
    return EXIT_FAILURE;
  }
  logger_id = logger_helper_enable(MAIN_LOGGER_ID, LOGGER_DEBUG, true);

  // Initialize configurations with default value
  if (ta_config_default_init(&ta_core.info, &ta_core.tangle, &ta_core.cache, &ta_core.service) != SC_OK) {
    return EXIT_FAILURE;
  }

  // Initialize configurations with CLI value
  if (ta_config_cli_init(&ta_core, argc, argv) != SC_OK) {
    return EXIT_FAILURE;
  }

  if (ta_config_set(&ta_core.cache, &ta_core.service) != SC_OK) {
    log_critical(logger_id, "[%s:%d] Configure failed %s.\n", __func__, __LINE__, MAIN_LOGGER_ID);
    return EXIT_FAILURE;
  }

  mux.handle("/mam/{bundle:[A-Z9]{81}}")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result = NULL;

        ret = api_receive_mam_message(&ta_core.service, req.params["bundle"].c_str(), &json_result);
        ret = set_response_content(ret, &json_result);

        set_method_header(res, HTTP_METHOD_GET);
        res.set_status(ret);
        res << json_result;
      });

  mux.handle("/mam")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .post([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        if (req.header("content-type").find("application/json") == std::string::npos) {
          cJSON* json_obj = cJSON_CreateObject();
          cJSON_AddStringToObject(json_obj, "message", "Invalid request header");
          json_result = cJSON_PrintUnformatted(json_obj);

          res.set_status(SC_HTTP_BAD_REQUEST);
          cJSON_Delete(json_obj);
        } else {
          ret = api_mam_send_message(&ta_core.tangle, &ta_core.service, req.body().c_str(), &json_result);
          ret = set_response_content(ret, &json_result);
          res.set_status(ret);
        }

        set_method_header(res, HTTP_METHOD_POST);
        res << json_result;
      });

  /**
   * @method {get} /tag/:tag Find transactions by tag
   *
   * @param {String} tag Must be 27 trytes long
   *
   * @return {String[]} hashes List of transaction hashes
   */
  mux.handle("/tag/{tag:[A-Z9]{1,27}}/hashes")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_find_transactions_by_tag(&ta_core.service, req.params["tag"].c_str(), &json_result);
        ret = set_response_content(ret, &json_result);
        set_method_header(res, HTTP_METHOD_GET);
        res.set_status(ret);
        res << json_result;
      });

  /**
   * @method {get} /transaction/:tx Get transaction object
   *
   * @param {String} tx Must be 81 trytes long transaction hash
   *
   * @return {String[]} object Info of enitre transaction object
   */
  mux.handle("/transaction/{tx:[A-Z9]{81}}")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_get_transaction_object(&ta_core.service, req.params["tx"].c_str(), &json_result);
        ret = set_response_content(ret, &json_result);
        set_method_header(res, HTTP_METHOD_GET);
        res.set_status(ret);
        res << json_result;
      });

  /**
   * @method {get} /tag/:tag Find transaction objects by tag
   *
   * @param {String} tag Must be 27 trytes long
   *
   * @return {String[]} transactions List of transaction objects
   */
  mux.handle("/tag/{tag:[A-Z9]{1,27}}")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_find_transactions_obj_by_tag(&ta_core.service, req.params["tag"].c_str(), &json_result);
        ret = set_response_content(ret, &json_result);
        set_method_header(res, HTTP_METHOD_GET);
        res.set_status(ret);
        res << json_result;
      });

  /**
   * @method {get} /tips Fetch pair tips which base on GetTransactionToApprove
   *
   * @return {String[]} tips Pair of transaction hashes
   */
  mux.handle("/tips/pair")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_get_tips_pair(&ta_core.tangle, &ta_core.service, &json_result);
        ret = set_response_content(ret, &json_result);
        set_method_header(res, HTTP_METHOD_GET);
        res.set_status(ret);
        res << json_result;
      });

  /**
   * @method {get} /tips Fetch all tips
   *
   * @return {String[]} tips List of transaction hashes
   */
  mux.handle("/tips")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_get_tips(&ta_core.service, &json_result);
        ret = set_response_content(ret, &json_result);
        set_method_header(res, HTTP_METHOD_GET);
        res.set_status(ret);
        res << json_result;
      });

  /**
   * @method {get} /address Generate an unused address
   *
   * @return {String} hash of address hashes
   */
  mux.handle("/address")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_generate_address(&ta_core.tangle, &ta_core.service, &json_result);
        ret = set_response_content(ret, &json_result);
        set_method_header(res, HTTP_METHOD_GET);
        res.set_status(ret);
        res << json_result;
      });

  /**
   * @method {post} /transaction send transfer
   *
   * @return {String} transaction object
   */
  mux.handle("/transaction")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .post([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        if (req.header("content-type").find("application/json") == std::string::npos) {
          cJSON* json_obj = cJSON_CreateObject();
          cJSON_AddStringToObject(json_obj, "message", "Invalid request header");
          json_result = cJSON_PrintUnformatted(json_obj);

          res.set_status(SC_HTTP_BAD_REQUEST);
          cJSON_Delete(json_obj);
        } else {
          ret = api_send_transfer(&ta_core.tangle, &ta_core.service, req.body().c_str(), &json_result);
          ret = set_response_content(ret, &json_result);
          res.set_status(ret);
        }

        set_method_header(res, HTTP_METHOD_POST);
        res << json_result;
      });

  /**
   * @method {post} /tryte send trytes
   *
   * @return {String} transaction object
   */
  mux.handle("/tryte")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .post([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        if (req.header("content-type").find("application/json") == std::string::npos) {
          cJSON* json_obj = cJSON_CreateObject();
          cJSON_AddStringToObject(json_obj, "message", "Invalid request header");
          json_result = cJSON_PrintUnformatted(json_obj);

          res.set_status(SC_HTTP_BAD_REQUEST);
          cJSON_Delete(json_obj);
        } else {
          ret = api_send_trytes(&ta_core.tangle, &ta_core.service, req.body().c_str(), &json_result);
          ret = set_response_content(ret, &json_result);
          res.set_status(ret);
        }

        set_method_header(res, HTTP_METHOD_POST);
        res << json_result;
      });

  /**
   * @method {get} {*} Client bad request
   * @method {options} {*} Get server information
   *
   * @return {String} message Error message
   */
  mux.handle("{*}")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) { set_method_header(res, HTTP_METHOD_OPTIONS); })
      .get([](served::response& res, const served::request&) {
        cJSON* json_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(json_obj, "message", "Invalid path");
        const char* json = cJSON_PrintUnformatted(json_obj);

        res.set_status(SC_HTTP_BAD_REQUEST);
        set_method_header(res, HTTP_METHOD_GET);
        res << json;

        cJSON_Delete(json_obj);
      });

  std::cout << "Starting..." << std::endl;
  served::net::server server(ta_core.info.host, ta_core.info.port, mux);
  server.run(ta_core.info.thread_count);

  ta_config_destroy(&ta_core.service);
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    return EXIT_FAILURE;
  }
  return 0;
}
