#include <served/methods.hpp>
#include <served/plugins.hpp>
#include <served/served.hpp>
#include "accelerator/apis.h"
#include "accelerator/config.h"
#include "accelerator/errors.h"
#include "cJSON.h"

void set_options_method_header(served::response& res) {
  res.set_header("Access-Control-Allow-Origin", "*");
  res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  res.set_header("Access-Control-Allow-Headers",
                 "Origin, Content-Type, Accept");
  res.set_header("Access-Control-Max-Age", "86400");
}

status_t set_response_content(status_t ret, char** json_result) {
  status_t http_ret;
  if (ret == SC_OK) {
    return SC_HTTP_OK;
  }

  cJSON* json_obj = cJSON_CreateObject();
  if ((ret & SC_ERROR_MASK) == 0x03) {
    http_ret = SC_NOT_FOUND;
    cJSON_AddStringToObject(json_obj, "message", "Request not found");
  } else if ((ret & SC_ERROR_MASK) == 0x07) {
    http_ret = SC_BAD_REQUEST;
    cJSON_AddStringToObject(json_obj, "message", "Invalid request header");
  } else {
    http_ret = SC_INTERNAL_SERVICE_ERROR;
    cJSON_AddStringToObject(json_obj, "message", "Internal service error");
  }
  *json_result = cJSON_PrintUnformatted(json_obj);
  return http_ret;
}

int main(int, char const**) {
  served::multiplexer mux;
  mux.use_after(served::plugin::access_log);

  iota_client_service_t service;
  service.http.path = "/";
  service.http.content_type = "application/json";
  service.http.accept = "application/json";
  service.http.host = IRI_HOST;
  service.http.port = IRI_PORT;
  service.http.api_version = 1;
  service.serializer_type = SR_JSON;
  iota_client_core_init(&service);
  iota_client_extended_init();

  mux.handle("/mam/{bundle:[A-Z9]{81}}")
      .method(served::method::OPTIONS,
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result = NULL;

        ret = api_receive_mam_message(&service, req.params["bundle"].c_str(),
                                      &json_result);
        ret = set_response_content(ret, &json_result);

        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_status(ret);
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_find_transactions_by_tag(&service, req.params["tag"].c_str(),
                                           &json_result);
        ret = set_response_content(ret, &json_result);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_get_transaction_object(&service, req.params["tx"].c_str(),
                                         &json_result);
        ret = set_response_content(ret, &json_result);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_find_transactions_obj_by_tag(
            &service, req.params["tag"].c_str(), &json_result);
        ret = set_response_content(ret, &json_result);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_get_tips_pair(&service, &json_result);
        ret = set_response_content(ret, &json_result);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_get_tips(&service, &json_result);
        ret = set_response_content(ret, &json_result);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        ret = api_generate_address(&service, &json_result);
        ret = set_response_content(ret, &json_result);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .post([&](served::response& res, const served::request& req) {
        status_t ret = SC_OK;
        char* json_result;

        if (req.header("content-type").find("application/json") ==
            std::string::npos) {
          cJSON* json_obj = cJSON_CreateObject();
          cJSON_AddStringToObject(json_obj, "message",
                                  "Invalid request header");
          json_result = cJSON_PrintUnformatted(json_obj);

          res.set_status(SC_BAD_REQUEST);
          cJSON_Delete(json_obj);
        } else {
          ret = api_send_transfer(&service, req.body().c_str(), &json_result);
          ret = set_response_content(ret, &json_result);
          res.set_status(ret);
        }

        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
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
              [&](served::response& res, const served::request& req) {
                set_options_method_header(res);
              })
      .get([](served::response& res, const served::request&) {
        cJSON* json_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(json_obj, "message", "Invalid path");
        const char* json = cJSON_PrintUnformatted(json_obj);

        res.set_status(SC_BAD_REQUEST);
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        res << json;

        cJSON_Delete(json_obj);
      });

  std::cout << "Starting..." << std::endl;
  served::net::server server(TA_HOST, TA_PORT, mux);
  server.run(TA_THREAD_COUNT);

  iota_client_extended_destroy();
  iota_client_core_destroy(&service);
  return 0;
}
