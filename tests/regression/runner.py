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
if len(sys.argv) == 2:
    raw_url = sys.argv[1]
elif len(sys.argv) == 3:
    raw_url = sys.argv[1]
    # if DEBUG_FLAG field == `Y`, then starts debugging mode
    if sys.argv[2] == 'y' or sys.argv[2] == 'Y':
        DEBUG_FLAG = True
else:
    raw_url = "localhost:8000"
url = "http://" + raw_url
headers = {'content-type': 'application/json'}

# Utils:
TIMEOUT = 100  # [sec]
MSG_STATUS_CODE_405 = "[405] Method Not Allowed"
CURL_EMPTY_REPLY = "000"

if DEBUG_FLAG == True:
    TIMES_TOTAL = 2
else:
    TIMES_TOTAL = 100
TIMES_TOTAL = 100
LEN_TAG = 27
LEN_ADDR = 81
LEN_MSG_SIGN = 2187
tryte_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ9"


def eval_stat(time_cost, func_name):
    avg = statistics.mean(time_cost)
    var = statistics.variance(time_cost)
    print("Average Elapsed Time of " + str(func_name) + ":" + str(avg) +
          " sec")
    print("With the range +- " + str(2 * var) +
          "sec, including 95% of API call time consumption")


class Time_Consumption():
    def test_mam_send_msg(self):
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
        payload = "Who are we? Just a speck of dust within the galaxy?"
        post_data = {"message": payload}
        post_data_json = json.dumps(post_data)
        response = API("/mam/", post_data=post_data_json)

        res_split = response.split(", ")
        res_json = json.loads(res_split[0])
        bundle_hash = res_json["bundle_hash"]

        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/mam/", get_data=bundle_hash)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "mam recv message")


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
        if get_data is not None:
            r = requests.get(str(url + get_query + get_data), timeout=TIMEOUT)
            if r.status_code == 405:
                response = MSG_STATUS_CODE_405
            else:
                response = r.text

        elif post_data is not None:
            command = "curl " + str(
                url + get_query
            ) + " -X POST -H 'Content-Type: application/json' -w \", %{http_code}\" -d '" + str(
                post_data) + "'"
            p = subprocess.Popen(command,
                                 shell=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            out, err = p.communicate()
            response = str(out.decode('ascii'))
        else:
            print("Wrong request method")
            response = None

    except BaseException:
        print(url, "Timeout!")
        print('\n    ' + repr(sys.exc_info()))
        return None
    if not response:
        response = ""
    time.sleep(2)
    return response


class Regression_Test(unittest.TestCase):
    def test_mam_send_msg(self):
        """ cmd
            0. English char only msg [success]
            1. ASCII symbols msg [success]
            2. Chinese msg [failed] curl response: "curl: (52) Empty reply from server"
            3. Japanese msg [failed] curl response: "curl: (52) Empty reply from server"
            4. Empty msg [failed]
            5. Non-JSON, plain text msg [failed]
            6. JSON msg with wrong key (not "message") [failed]
        """
        test_cases = [
            "ToBeOrNotToBe", "I met my soulmate. She didnt", "當工程師好開心阿",
            "今夜は月が綺麗ですね", "", "Non-JSON, plain text msg",
            "JSON msg with wrong key"
        ]

        pass_case = [0, 1]
        for i in range(len(test_cases)):
            if i not in pass_case:
                test_cases[i].encode(encoding='utf-8')

        response = []
        for i in range(len(test_cases)):
            print("testing case = " + str(test_cases[i]))
            if i == 5:
                post_data_json = test_cases[i]
            elif i == 6:
                post_data = {"notkey": test_cases[i]}
                post_data_json = json.dumps(post_data)
            else:
                post_data = {"message": test_cases[i]}
                post_data_json = json.dumps(post_data)
            response.append(API("/mam/", post_data=post_data_json))

        for i in range(len(response)):
            if i in pass_case:
                res_split = response[i].split(", ")
                res_json = json.loads(res_split[0])
                self.assertTrue(is_addr_trytes(res_json["channel"]))
                self.assertTrue(is_addr_trytes(res_json["bundle_hash"]))
            else:
                self.assertTrue(CURL_EMPTY_REPLY in response[i])

    def test_mam_recv_msg(self):
        """ cmd
            1. Correct exist MAMv2 msg [success]
            2. Empty msg [failed] empty parameter causes http error 405
            3. Unicode msg [failed] {\"message\":\"Internal service error\"}
            4. Not existing bundle hash (address)
        """
        test_cases = [
            "BDIQXTDSGAWKCEPEHLRBSLDEFLXMX9ZOTUZW9JAIGZBFKPICXPEO9LLVTNIFGFDWWHEQNZXJZ9F9HTXD9",
            "", "生れてすみません"
        ]
        expect_cases = [
            "{\"message\":\"jjjjjjjjj\"}", MSG_STATUS_CODE_405,
            MSG_STATUS_CODE_405
        ]

        response = []
        for t_case in test_cases:
            print("testing case = " + t_case)
            response.append(API("/mam/", get_data=t_case))

        self.assertEqual(len(expect_cases), len(test_cases))
        for i in range(len(test_cases)):
            self.assertTrue(expect_cases[i] in response[i])

    def test_send_transfer(self):
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
        test_cases = [
            [420, rand_msg, rand_tag, rand_addr],
            [0, rand_msg, rand_tag, rand_addr],
            [
                "生而為人, 我很抱歉".encode(encoding='utf-8'), rand_msg, rand_tag,
                rand_addr
            ], [0, "生而為人, 我很抱歉".encode(encoding='utf-8'), rand_tag, rand_addr],
            [0, rand_msg, "生而為人, 我很抱歉".encode(encoding='utf-8'), rand_addr],
            [-5, rand_msg, rand_tag, rand_addr],
            [None, rand_msg, rand_tag, rand_addr],
            [0, None, rand_tag, rand_addr], [0, rand_msg, None, rand_addr],
            [0, rand_msg, rand_tag, None], [0, rand_msg, rand_tag, "我思故我在"]
        ]

        response = []
        for i in range(len(test_cases)):
            logging.debug("testing case = " + str(test_cases[i]))
            post_data = {
                "value": test_cases[i][0],
                "message": test_cases[i][1],
                "tag": test_cases[i][2],
                "address": test_cases[i][3]
            }
            logging.debug("post_data = " + repr(post_data))
            post_data_json = json.dumps(post_data)
            response.append(API("/transaction/", post_data=post_data_json))

        if DEBUG_FLAG == True:
            for i in range(len(response)):
                logging.debug("send transfer i = " + str(i) + ", response = " +
                              response[i])

        pass_case = [0, 1, 2]
        for i in range(len(response)):
            if i in pass_case:
                logging.debug("i = " + str(i) + ", response = " + response[i])
                res_split = response[i].split(", ")
                res_json = json.loads(res_split[0])

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
                self.assertTrue(EMPTY_REPLY in response[i]
                                or MSG_STATUS_CODE_404 in response[i])

        # Time Statistics
        time_cost = []
        rand_msg = gen_rand_trytes(30)
        rand_tag = gen_rand_trytes(27)
        rand_addr = gen_rand_trytes(81)
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            post_data = "{\"value\":0,\"message\":\"" + str(
                rand_msg) + "\",\"tag\":\"" + str(
                    rand_tag) + "\",\"address\":\"" + str(rand_addr) + "\"}"
            API("/transaction/", post_data=post_data)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "send transfer")


"""
    API List
        mam_recv_msg: GET
        mam_send_msg: POST
        Find transactions by tag
        Get transaction object
        Find transaction objects by tag
        Get transaction object
        Find transaction objects by tag
        Fetch pair tips which base on GetTransactionToApprove
        Fetch all tips
        Generate an unused address
        send transfer
        Client bad request
"""

# Run all the API Test here
if __name__ == '__main__':
    unittest.main(argv=['first-arg-is-ignored'], exit=False)

    # Run all the Time_Consumption() tests
    f = Time_Consumption()
    public_method_names = [
        method for method in dir(f) if callable(getattr(f, method))
        if not method.startswith('_')
    ]  # 'private' methods start from _
    for method in public_method_names:
        getattr(f, method)()  # call

    if not unittest.TestResult().errors:
        exit(1)
