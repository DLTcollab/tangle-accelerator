# Run in Python3
import json
import requests
import sys
import subprocess
import unittest

if len(sys.argv) == 2:
    raw_url = sys.argv[1]
else:
    raw_url = "localhost:8000"
url = "http://" + raw_url
headers = {'content-type': 'application/json'}

# Utils:
TIMEOUT = 100  # [sec]
MSG_STATUS_CODE_405 = "[405] Method Not Allowed"
CURL_EMPTY_REPLY = "000"


def is_addr_trytes(trytes):
    tryte_alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ9"
    if len(trytes) != 81:
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
            # print("err = " + str(err))
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
            "QBFXQETKSHDYPFUDO9ILVCAVQIXOHXKCECZYFLPBNVIX9JUXQZJE9URQEEUWPWYZOIACTCGZX9IDIODCA",
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
