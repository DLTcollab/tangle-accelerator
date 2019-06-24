#!/usr/bin/env python3

# Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
# All Rights Reserved.
# This is free software; you can redistribute it and/or modify it under the
# terms of the MIT license. A copy of the license can be found in the file
# "LICENSE" at the root of this distribution.

# Run in Python3
import json
import requests
import sys
import subprocess
import unittest
import statistics
import time
import random
import logging

DEBUG_FLAG = False
TIMES_TOTAL = 100
if len(sys.argv) == 2:
    raw_url = sys.argv[1]
elif len(sys.argv) == 4:
    raw_url = sys.argv[1]
    if sys.argv[2] == 'Y':
        DEBUG_FLAG = True

    # the 3rd arg is the option which determine if use the debugging mode of statistical tests
    if sys.argv[3] == 'Y':
        TIMES_TOTAL = 2
else:
    raw_url = "localhost:8000"
url = "http://" + raw_url
headers = {'content-type': 'application/json'}

# Utils:
TIMEOUT = 100  # [sec]
STATUS_CODE_500 = "500"
STATUS_CODE_405 = "405"
STATUS_CODE_404 = "404"
STATUS_CODE_400 = "400"
STATUS_CODE_200 = "200"
EMPTY_REPLY = "000"
LEN_TAG = 27
LEN_ADDR = 81
LEN_MSG_SIGN = 2187
tryte_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ9"


def eval_stat(time_cost, func_name):
    avg = statistics.mean(time_cost)
    var = statistics.variance(time_cost)
    print("Average Elapsed Time of `" + str(func_name) + "`:" + str(avg) +
          " sec")
    print("With the range +- " + str(2 * var) +
          "sec, including 95% of API call time consumption")


def fill_nines(trytes, output_len):
    out_str = trytes + "9" * (output_len - len(trytes))

    return out_str


def gen_rand_trytes(tryte_len):
    trytes = ""
    for i in range(tryte_len):
        trytes = trytes + tryte_alphabet[random.randint(0, 26)]
    return trytes


def valid_trytes(trytes, trytes_len):
    if len(trytes) != trytes_len:
        return False

    for char in trytes:
        if char not in tryte_alphabet:
            return False

    return True


def API(get_query, get_data=None, post_data=None):
    try:
        response = {}
        if get_data is not None:
            r = requests.get(str(url + get_query + get_data), timeout=TIMEOUT)
            response = {"content": r.text, "status_code": str(r.status_code)}

        elif post_data is not None:
            command = "curl " + str(
                url + get_query
            ) + " -X POST -H 'Content-Type: application/json' -w \", %{http_code}\" -d '" + str(
                post_data) + "'"
            logging.debug("curl command = " + command)
            p = subprocess.Popen(command,
                                 shell=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            out, err = p.communicate()
            curl_response = str(out.decode('ascii'))
            response = {
                "content": curl_response.split(", ")[0],
                "status_code": curl_response.split(", ")[1]
            }
        else:
            logging.error("Wrong request method")
            response = None

    except BaseException:
        logging.error(url, "Timeout!")
        logging.error('\n    ' + repr(sys.exc_info()))
        return None
    if not response:
        response = None

    return response


class Regression_Test(unittest.TestCase):
    def test_mam_send_msg(self):
        logging.debug(
            "\n================================mam send msg================================"
        )
        # cmd
        #    0. English char only msg [success]
        #    1. ASCII symbols msg [success]
        #    2. Chinese msg [failed] curl response: "curl: (52) Empty reply from server"
        #    3. Japanese msg [failed] curl response: "curl: (52) Empty reply from server"
        #    4. Empty msg [failed]
        #    5. Non-JSON, plain text msg [failed]
        #    6. JSON msg with wrong key (not "message") [failed]
        query_string = [
            "ToBeOrNotToBe", "I met my soulmate. She didnt", "當工程師好開心阿",
            "今夜は月が綺麗ですね", "", "Non-JSON, plain text msg",
            "JSON msg with wrong key"
        ]

        pass_case = [0, 1, 4]
        for i in range(len(query_string)):
            if i not in pass_case:
                query_string[i].encode(encoding='utf-8')

        response = []
        for i in range(len(query_string)):
            logging.debug("testing case = " + repr(query_string[i]))
            if i == 5:
                post_data_json = query_string[i]
            elif i == 6:
                post_data = {"notkey": query_string[i]}
                post_data_json = json.dumps(post_data)
            else:
                post_data = {"message": query_string[i]}
                post_data_json = json.dumps(post_data)
            response.append(API("/mam/", post_data=post_data_json))

        for i in range(len(response)):
            logging.debug("send msg i = " + str(i) + ", res = " +
                          response[i]["content"] + ", status code = " +
                          response[i]["status_code"])

        for i in range(len(response)):
            if i in pass_case:
                res_json = json.loads(response[i]["content"])
                self.assertTrue(valid_trytes(res_json["channel"], LEN_ADDR))
                self.assertTrue(valid_trytes(res_json["bundle_hash"],
                                             LEN_ADDR))
            else:
                self.assertEqual(STATUS_CODE_500, response[i]["status_code"])

        # Time Statistics
        payload = "Who are we? Just a speck of dust within the galaxy?"
        post_data = {"message": payload}
        post_data_json = json.dumps(post_data)

        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/mam/", post_data=post_data_json)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "mam send message")

    def test_mam_recv_msg(self):
        logging.debug(
            "\n================================mam recv msg================================"
        )
        # cmd
        #    0. Correct exist MAMv2 msg [success]
        #    1. Empty msg [failed] empty parameter causes http error 405
        #    2. Unicode msg [failed] {\"message\":\"Internal service error\"}
        #    3. Not existing bundle hash (address)
        query_string = [
            "BDIQXTDSGAWKCEPEHLRBSLDEFLXMX9ZOTUZW9JAIGZBFKPICXPEO9LLVTNIFGFDWWHEQNZXJZ9F9HTXD9",
            "", "生れてすみません"
        ]

        expect_cases = ["\"message\":\"ToBeOrNotToBe\""]

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + t_case)
            response.append(API("/mam/", get_data=t_case))

        pass_case = [0]
        for i in range(len(query_string)):
            if i in pass_case:
                self.assertTrue(expect_cases[i] in response[i]["content"])
            else:
                self.assertEqual(STATUS_CODE_405, response[i]["status_code"])

        # Time Statistics
        # send a MAM message and use it as the the message to be searched
        payload = "Who are we? Just a speck of dust within the galaxy?"
        post_data = {"message": payload}
        post_data_json = json.dumps(post_data)
        response = API("/mam/", post_data=post_data_json)

        res_json = json.loads(response["content"])
        bundle_hash = res_json["bundle_hash"]

        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/mam/", get_data=bundle_hash)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "mam recv message")

    def test_send_transfer(self):
        logging.debug(
            "\n================================send transfer================================"
        )
        # cmd
        #    0. positive value, tryte maessage, tryte tag, tryte address
        #    1. zero value, tryte message, tryte tag, tryte address
        #    2. chinese value, tryte message, tryte tag, tryte address
        #    3. zero value, chinese message, tryte tag, tryte address
        #    4. zero value, tryte message, chinese tag, tryte address
        #    5. negative value, tryte maessage, tryte tag, tryte address
        #    6. no value, tryte maessage, tryte tag, tryte address
        #    7. zero value, no maessage, tryte tag, tryte address
        #    8. zero value, tryte maessage, no tag, tryte address
        #    9. zero value, tryte maessage, tryte tag, no address
        #    10. zero value, tryte maessage, tryte tag, unicode address
        rand_msg = gen_rand_trytes(30)
        rand_tag = gen_rand_trytes(27)
        rand_addr = gen_rand_trytes(81)
        query_string = [[420, rand_msg, rand_tag, rand_addr],
                        [0, rand_msg, rand_tag, rand_addr],
                        ["生而為人, 我很抱歉", rand_msg, rand_tag, rand_addr],
                        [0, "生而為人, 我很抱歉", rand_tag, rand_addr],
                        [0, rand_msg, "生而為人, 我很抱歉", rand_addr],
                        [-5, rand_msg, rand_tag, rand_addr],
                        [None, rand_msg, rand_tag, rand_addr],
                        [0, None, rand_tag, rand_addr],
                        [0, rand_msg, None, rand_addr],
                        [0, rand_msg, rand_tag, None],
                        [0, rand_msg, rand_tag, "我思故我在"]]

        response = []
        for i in range(len(query_string)):
            logging.debug("testing case = " + repr(query_string[i]))
            post_data = {
                "value": query_string[i][0],
                "message": query_string[i][1],
                "tag": query_string[i][2],
                "address": query_string[i][3]
            }
            logging.debug("post_data = " + repr(post_data))
            post_data_json = json.dumps(post_data)
            response.append(API("/transaction/", post_data=post_data_json))

        for i in range(len(response)):
            logging.debug("send transfer i = " + str(i) + ", res = " +
                          response[i]["content"] + ", status code = " +
                          response[i]["status_code"])

        pass_case = [0, 1, 2, 3, 5, 6, 7, 8, 9, 10]
        for i in range(len(response)):
            if i in pass_case:
                res_json = json.loads(response[i]["content"])

                # we only send zero tx at this moment
                self.assertEqual(0, res_json["value"])
                self.assertTrue(valid_trytes(res_json["tag"], LEN_TAG))
                self.assertTrue(valid_trytes(res_json["address"], LEN_ADDR))
                self.assertTrue(
                    valid_trytes(res_json["trunk_transaction_hash"], LEN_ADDR))
                self.assertTrue(
                    valid_trytes(res_json["branch_transaction_hash"],
                                 LEN_ADDR))
                self.assertTrue(valid_trytes(res_json["bundle_hash"],
                                             LEN_ADDR))
                self.assertTrue(valid_trytes(res_json["hash"], LEN_ADDR))
                self.assertTrue(
                    valid_trytes(res_json["signature_and_message_fragment"],
                                 LEN_MSG_SIGN))
            elif i == 4:
                self.assertEqual(STATUS_CODE_500, response[i]["status_code"])
            else:
                self.assertEqual(STATUS_CODE_404, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        rand_msg = gen_rand_trytes(30)
        rand_tag = gen_rand_trytes(27)
        rand_addr = gen_rand_trytes(81)
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            post_data = {
                "value": 0,
                "message": rand_msg,
                "tag": rand_tag,
                "address": rand_addr
            }
            post_data_json = json.dumps(post_data)
            API("/transaction/", post_data=post_data_json)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "send transfer")

    def test_find_transactions_by_tag(self):
        logging.debug(
            "\n================================find transactions by tag================================"
        )
        # cmd
        #    0. 27 trytes tag
        #    1. 20 trytes tag
        #    2. 30 trytes tag
        #    3. unicode trytes tag
        #    4. Null trytes tag
        rand_tag_27 = gen_rand_trytes(27)
        rand_tag_20 = gen_rand_trytes(20)
        rand_tag_30 = gen_rand_trytes(30)
        query_string = [rand_tag_27, rand_tag_20, rand_tag_30, "半導體絆倒你", None]

        rand_msg = gen_rand_trytes(30)
        rand_addr = gen_rand_trytes(81)
        transaction_response = []
        for i in range(3):
            post_data = {
                "value": 0,
                "message": rand_msg,
                "tag": query_string[i],
                "address": rand_addr
            }
            post_data_json = json.dumps(post_data)
            transaction_response.append(
                API("/transaction/", post_data=post_data_json))

        for i in range(len(transaction_response)):
            logging.debug("find transactions by tag i = " + str(i) +
                          ", tx_res = " + transaction_response[i]["content"] +
                          ", status code = " +
                          transaction_response[i]["status_code"])

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + repr(t_case))
            if t_case != None:
                response.append(API("/tag/", get_data=(t_case + "/hashes")))
            else:
                response.append(API("/tag/", get_data="/hashes"))

        for i in range(len(response)):
            logging.debug("find transactions by tag i = " + str(i) +
                          ", res = " + response[i]["content"] +
                          ", status code = " + response[i]["status_code"])
        pass_case = [0, 1]
        for i in range(len(response)):
            if i in pass_case:
                expected_tx_json = json.loads(
                    transaction_response[i]["content"])
                res_json = json.loads(response[i]["content"])

                self.assertEqual(expected_tx_json["hash"],
                                 res_json["hashes"][0])
            else:
                self.assertEqual(STATUS_CODE_400, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/tag/", get_data=(rand_tag_27 + "/hashes"))
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "find transactions by tag")

    def test_get_transactions_object(self):
        logging.debug(
            "\n================================get transactions object================================"
        )
        # cmd
        #    0. 81 trytes transaction hash
        #    1. 20 trytes transaction hash
        #    2. 100 trytes transaction hash
        #    3. unicode transaction hash
        #    4. Null transaction hash
        rand_tag = gen_rand_trytes(27)
        rand_msg = gen_rand_trytes(30)
        rand_addr = gen_rand_trytes(81)
        post_data = {
            "value": 0,
            "message": rand_msg,
            "tag": rand_tag,
            "address": rand_addr
        }
        post_data_json = json.dumps(post_data)
        sent_transaction_obj = API("/transaction/", post_data=post_data_json)

        logging.debug("sent_transaction_obj = " +
                      ", sent_transaction_obj[0] = " +
                      sent_transaction_obj["content"] +
                      ", sent_transaction_obj[1] = " +
                      sent_transaction_obj["status_code"])
        sent_transaction_obj_json = json.loads(sent_transaction_obj["content"])
        sent_transaction_hash = sent_transaction_obj_json["hash"]
        query_string = [
            sent_transaction_hash, sent_transaction_hash[0:19],
            sent_transaction_hash + gen_rand_trytes(19), "工程師批哩趴啦的生活", ""
        ]

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + repr(t_case))
            response.append(API("/transaction/", get_data=t_case))

        for i in range(len(response)):
            logging.debug("get transactions object i = " + str(i) +
                          ", res = " + repr(response[i]["content"]) +
                          ", status code = " +
                          repr(response[i]["status_code"]))

        pass_case = [0]
        for i in range(len(response)):
            if i in pass_case:
                res_json = json.loads(response[i]["content"])
                self.assertEqual(sent_transaction_hash, res_json["hash"])
            else:
                self.assertEqual(STATUS_CODE_405, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/transaction/", get_data=sent_transaction_hash)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "get transactions object")

    def test_find_transactions_obj_by_tag(self):
        logging.debug(
            "\n================================find transactions object by tag================================"
        )

        # cmd
        #    0. 27 trytes tag
        #    1. 20 trytes tag
        #    2. multiple transactions share the same 27 trytes tag
        #    3. 30 trytes tag
        #    4. unicode trytes tag
        #    5. Null trytes tag
        rand_tag_27 = gen_rand_trytes(27)
        rand_tag_27_multi = gen_rand_trytes(27)
        rand_tag_20 = gen_rand_trytes(20)
        rand_tag_30 = gen_rand_trytes(30)
        query_string = [
            rand_tag_27, rand_tag_27_multi, rand_tag_20, rand_tag_30, "半導體絆倒你",
            ""
        ]

        rand_msg = gen_rand_trytes(30)
        rand_addr = gen_rand_trytes(81)
        transaction_response = []
        for i in range(3):
            post_data = {
                "value": 0,
                "message": rand_msg,
                "tag": query_string[i],
                "address": rand_addr
            }
            post_data_json = json.dumps(post_data)

            if i == 2:
                transaction_response.append([
                    API("/transaction/", post_data=post_data_json),
                    API("/transaction/", post_data=post_data_json)
                ])
            else:
                transaction_response.append(
                    [API("/transaction/", post_data=post_data_json)])

        for i in range(len(transaction_response)):
            logging.debug("find transactions obj by tag i = " + str(i) +
                          ", tx_res = " + repr(transaction_response[i]))

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + repr(t_case))
            response.append(API("/tag/", get_data=t_case))

        for i in range(len(response)):
            logging.debug("find transactions obj by tag i = " + str(i) +
                          ", res = " + repr(response[i]))
        pass_case = [0, 1, 2]
        for i in range(len(response)):
            if i in pass_case:
                res_tx_json = json.loads(response[i]["content"])
                res_json = []
                expected_tx_json = []
                for j in range(len(transaction_response[i])):
                    res_json.append(res_tx_json["transactions"][j])
                    expected_tx_json.append(
                        json.loads(transaction_response[i][j]["content"]))

                for j in range(len(expected_tx_json)):
                    case_examined_equal = False
                    for k in range(len(res_json)):
                        if expected_tx_json[j]["hash"] == res_json[k]["hash"]:
                            self.assertEqual(
                                expected_tx_json[j]
                                ["signature_and_message_fragment"],
                                res_json[k]["signature_and_message_fragment"])
                            self.assertEqual(expected_tx_json[j]["address"],
                                             res_json[k]["address"])
                            self.assertEqual(expected_tx_json[j]["value"],
                                             res_json[k]["value"])
                            self.assertEqual(
                                expected_tx_json[j]["obsolete_tag"],
                                res_json[k]["obsolete_tag"])
                            self.assertEqual(expected_tx_json[j]["timestamp"],
                                             res_json[k]["timestamp"])
                            self.assertEqual(expected_tx_json[j]["last_index"],
                                             res_json[k]["last_index"])
                            self.assertEqual(
                                expected_tx_json[j]["bundle_hash"],
                                res_json[k]["bundle_hash"])
                            self.assertEqual(
                                expected_tx_json[j]["trunk_transaction_hash"],
                                res_json[k]["trunk_transaction_hash"])
                            self.assertEqual(
                                expected_tx_json[j]["branch_transaction_hash"],
                                res_json[k]["branch_transaction_hash"])
                            self.assertEqual(expected_tx_json[j]["tag"],
                                             res_json[k]["tag"])
                            self.assertEqual(
                                expected_tx_json[j]["attachment_timestamp"],
                                res_json[k]["attachment_timestamp"])
                            self.assertEqual(
                                expected_tx_json[j]
                                ["attachment_timestamp_lower_bound"],
                                res_json[k]
                                ["attachment_timestamp_lower_bound"])
                            self.assertEqual(
                                expected_tx_json[j]
                                ["attachment_timestamp_upper_bound"],
                                res_json[k]
                                ["attachment_timestamp_upper_bound"])
                            self.assertEqual(expected_tx_json[j]["nonce"],
                                             res_json[k]["nonce"])
                            case_examined_equal = True
                            break
                    self.assertTrue(case_examined_equal)
            else:
                self.assertEqual(STATUS_CODE_400, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/tag/", get_data=rand_tag_27)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "find transactions obj by tag")

    def test_get_tips(self):
        logging.debug(
            "\n================================get_tips================================"
        )
        # cmd
        #    0. call get_tips normally
        #    1. call get_tips with unwanted ascii string
        #    2. call get_tips with unwanted unicode string
        rand_tag_27 = gen_rand_trytes(27)
        query_string = ["", rand_tag_27, "飛天義大利麵神教"]

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + repr(t_case))
            response.append(API("/tips/", get_data=t_case))

        for i in range(len(response)):
            logging.debug("get_tips i = " + str(i) + ", res = " +
                          repr(response[i]["content"]) + ", status code = " +
                          repr(response[i]["status_code"]))

        pass_case = [0]
        for i in range(len(response)):
            if i in pass_case:
                res_json = json.loads(response[i]["content"])
                tips_hashes_array = res_json["hashes"]

                for tx_hashes in tips_hashes_array:
                    self.assertTrue(valid_trytes(tx_hashes, LEN_ADDR))
            else:
                # At this moment, api get_tips allow whatever string follow after /tips/
                self.assertEqual(STATUS_CODE_200, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/tips/", get_data="")
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "get_tips")

    def test_get_tips_pair(self):
        logging.debug(
            "\n================================get_tips_pair================================"
        )
        # cmd
        #    0. call get_tips normally
        #    1. call get_tips with unwanted ascii string
        #    2. call get_tips with unwanted unicode string
        rand_tag_27 = gen_rand_trytes(27)
        query_string = ["", rand_tag_27, "飛天義大利麵神教"]

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + repr(t_case))
            response.append(API("/tips/pair/", get_data=t_case))

        for i in range(len(response)):
            logging.debug("get_tips i = " + str(i) + ", res = " +
                          repr(response[i]["content"]) + ", status code = " +
                          repr(response[i]["status_code"]))

        pass_case = [0]
        for i in range(len(response)):
            if i in pass_case:
                res_json = json.loads(response[i]["content"])

                self.assertTrue(
                    valid_trytes(res_json["trunkTransaction"], LEN_ADDR))
                self.assertTrue(
                    valid_trytes(res_json["branchTransaction"], LEN_ADDR))
            else:
                # At this moment, api get_tips allow whatever string follow after /tips/pair
                self.assertEqual(STATUS_CODE_200, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/tips/pair/", get_data="")
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "get_tips_pair")

    def test_generate_address(self):
        logging.debug(
            "\n================================generate_address================================"
        )
        # cmd
        #    0. call generate_address normally
        #    1. call generate_address with unwanted ascii string
        #    2. call generate_address with unwanted unicode string
        rand_tag_81 = gen_rand_trytes(81)
        query_string = ["", rand_tag_81, "飛天義大利麵神教"]

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + repr(t_case))
            response.append(API("/address/", get_data=t_case))

        for i in range(len(response)):
            logging.debug("generate_address i = " + str(i) + ", res = " +
                          repr(response[i]["content"]) + ", status code = " +
                          repr(response[i]["status_code"]))

        pass_case = [0]
        for i in range(len(response)):
            if i in pass_case:
                res_json = json.loads(response[i]["content"])
                addr_list = res_json["address"]

                self.assertTrue(valid_trytes(addr_list[0], LEN_ADDR))
            else:
                # At this moment, api generate_address allow whatever string follow after /generate_address/
                self.assertEqual(STATUS_CODE_200, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/address", get_data="")
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "generate_address")

    def test_send_trytes(self):
        logging.debug(
            "\n================================send_trytes================================"
        )
        # cmd
        #    0. single 2673 trytes legal transaction object
        #    1. multiple 2673 ligal trytes transaction object
        #    2. single 200 trytes illegal transaction object
        #    3. single single 3000 trytes illegal transaction object
        #    4. single unicode illegal transaction object
        #    5. empty trytes list
        #    6. empty not trytes list object
        rand_trytes = []
        for i in range(2):
            all_9_context = fill_nines("", 2673 - 81 * 3)
            tips_response = API("/tips/pair/", get_data="")
            res_json = json.loads(tips_response["content"])

            rand_trytes.append(all_9_context + res_json["trunkTransaction"] +
                               res_json["branchTransaction"] +
                               fill_nines("", 81))

        query_string = [[rand_trytes[0]], [rand_trytes[0], rand_trytes[1]],
                        [gen_rand_trytes(200)], [gen_rand_trytes(3000)],
                        ["逼類不司"], [""], ""]

        response = []
        for i in range(len(query_string)):
            logging.debug("testing case = " + repr(query_string[i]))
            post_data = {"trytes": query_string[i]}
            logging.debug("post_data = " + repr(post_data))
            post_data_json = json.dumps(post_data)
            response.append(API("/tryte", post_data=post_data_json))

        for i in range(len(response)):
            logging.debug("send_trytes i = " + str(i) + ", res = " +
                          response[i]["content"] + ", status code = " +
                          response[i]["status_code"])

        pass_case = [0, 1]
        for i in range(len(response)):
            logging.debug("send_trytes i = " + str(i) + ", res = " +
                          response[i]["content"] + ", status code = " +
                          response[i]["status_code"])
            if i in pass_case:
                res_json = json.loads(response[i]["content"])

                self.assertEqual(query_string[i], res_json["trytes"])
            else:
                self.assertEqual(STATUS_CODE_500, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        post_data = {"trytes": [rand_trytes[0]]}
        post_data_json = json.dumps(post_data)
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/tryte", post_data=post_data_json)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "send trytes")


"""
    API List
        mam_recv_msg: GET
        mam_send_msg: POST
        Find transactions by tag: GET
        Get transaction object
        Find transaction objects by tag
        Get transaction object
        Find transaction objects by tag
        Fetch pair tips which base on GetTransactionToApprove
        Fetch all tips
        Generate an unused address
        send transfer: POST
        Client bad request
"""

# Run all the API Test here
if __name__ == '__main__':
    if DEBUG_FLAG == True:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    unittest.main(argv=['first-arg-is-ignored'], exit=True)
