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

        pass_case = [0, 1, 2, 5, 6, 7, 8, 9, 10]
        for i in range(len(response)):
            if i in pass_case:
                self.assertEqual(STATUS_CODE_200, response[i]["status_code"])
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
            else:
                self.assertEqual(STATUS_CODE_500, response[i]["status_code"])

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

    def test_get_transactions_object(self):
        logging.debug(
            "\n================================find transaction objects================================"
        )
        # cmd
        #    0. 81 trytes transaction hash
        #    1. multiple 81 trytes transaction hash
        #    2. 20 trytes transaction hash
        #    3. 100 trytes transaction hash
        #    4. unicode transaction hash
        #    5. Null transaction hash
        sent_transaction_tmp = []
        for i in range(3):
            rand_tag = gen_rand_trytes(27)
            rand_msg = gen_rand_trytes(30)
            rand_addr = gen_rand_trytes(81)
            tx_post_data = {
                "value": 0,
                "message": rand_msg,
                "tag": rand_tag,
                "address": rand_addr
            }
            tx_post_data_json = json.dumps(tx_post_data)
            sent_transaction_obj = API("/transaction/",
                                       post_data=tx_post_data_json)

            logging.debug("sent_transaction_obj = " +
                          repr(sent_transaction_obj))
            self.assertEqual(STATUS_CODE_200, sent_transaction_obj["status_code"])
            sent_transaction_obj_json = json.loads(
                sent_transaction_obj["content"])
            sent_transaction_tmp.append(sent_transaction_obj_json)
        sent_transaction = [[sent_transaction_tmp[0]],
                            [sent_transaction_tmp[1], sent_transaction_tmp[2]]]
        query_string = [[sent_transaction_tmp[0]["hash"]],
                        [
                            sent_transaction_tmp[1]["hash"],
                            sent_transaction_tmp[2]["hash"]
                        ],
                        gen_rand_trytes(19),
                        gen_rand_trytes(100), "工程師批哩趴啦的生活", ""]

        response = []
        for t_case in query_string:
            logging.debug("testing case = " + repr(t_case))
            post_data_json = json.dumps({"hashes": t_case})
            response.append(
                API("/transaction/object", post_data=post_data_json))

        for i in range(len(response)):
            logging.debug("response find transaction objects i = " + str(i) +
                          ", " + repr(response[i]))
        pass_case = [0, 1]

        for i in range(len(response)):
            if i in pass_case:
                expect_txs = sent_transaction[i]
                res_txs = json.loads(response[i]["content"])

                for j in range(len(expect_txs)):
                    did_exmine = False
                    for k in range(len(res_txs)):
                        if expect_txs[j]["hash"] == res_txs[k]["hash"]:
                            self.assertEqual(
                                expect_txs[j]
                                ["signature_and_message_fragment"],
                                res_txs[k]["signature_and_message_fragment"])
                            self.assertEqual(expect_txs[j]["address"],
                                             res_txs[k]["address"])
                            self.assertEqual(expect_txs[j]["value"],
                                             res_txs[k]["value"])
                            self.assertEqual(expect_txs[j]["obsolete_tag"],
                                             res_txs[k]["obsolete_tag"])
                            self.assertEqual(expect_txs[j]["timestamp"],
                                             res_txs[k]["timestamp"])
                            self.assertEqual(expect_txs[j]["last_index"],
                                             res_txs[k]["last_index"])
                            self.assertEqual(expect_txs[j]["bundle_hash"],
                                             res_txs[k]["bundle_hash"])
                            self.assertEqual(
                                expect_txs[j]["trunk_transaction_hash"],
                                res_txs[k]["trunk_transaction_hash"])
                            self.assertEqual(
                                expect_txs[j]["branch_transaction_hash"],
                                res_txs[k]["branch_transaction_hash"])
                            self.assertEqual(expect_txs[j]["tag"],
                                             res_txs[k]["tag"])
                            self.assertEqual(
                                expect_txs[j]["attachment_timestamp"],
                                res_txs[k]["attachment_timestamp"])
                            self.assertEqual(
                                expect_txs[j]
                                ["attachment_timestamp_lower_bound"],
                                res_txs[k]["attachment_timestamp_lower_bound"])
                            self.assertEqual(
                                expect_txs[j]
                                ["attachment_timestamp_upper_bound"],
                                res_txs[k]["attachment_timestamp_upper_bound"])
                            self.assertEqual(expect_txs[j]["nonce"],
                                             res_txs[k]["nonce"])
                            did_exmine = True
                            break

                    self.assertTrue(did_exmine)

            else:
                self.assertEqual(STATUS_CODE_500, response[i]["status_code"])

        # Time Statistics
        time_cost = []
        post_data_json = json.dumps({"hashes": query_string[0]})
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/transaction/object", post_data=post_data_json)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "find transaction objects")

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
                tips_hashes_array = json.loads(response[i]["content"])

                for tx_hashes in tips_hashes_array:
                    self.assertTrue(valid_trytes(tx_hashes, LEN_ADDR))
            else:
                self.assertEqual(STATUS_CODE_400, response[i]["status_code"])

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
                self.assertEqual(STATUS_CODE_200, response[i]["status_code"])
                res_json = json.loads(response[i]["content"])

                self.assertTrue(
                    valid_trytes(res_json["trunkTransaction"], LEN_ADDR))
                self.assertTrue(
                    valid_trytes(res_json["branchTransaction"], LEN_ADDR))
            else:
                self.assertEqual(STATUS_CODE_400, response[i]["status_code"])

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

                self.assertTrue(valid_trytes(res_json[0], LEN_ADDR))
            else:
                self.assertEqual(STATUS_CODE_400, response[i]["status_code"])

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
            self.assertEqual(STATUS_CODE_200, tips_response["status_code"])
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
