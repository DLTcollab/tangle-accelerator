from common import *
import json
import unittest
import time
import logging


class GetTips(unittest.TestCase):

    # Without additional GET parameter (pass)
    def test_normal(self):
        res = API("/tips/", get_data=self.query_string[0])
        self.assertEqual(STATUS_CODE_200, res["status_code"])
        tips_hashes = json.loads(res["content"])

        for tx_hash in tips_hashes:
            self.assertTrue(valid_trytes(tx_hash, LEN_ADDR))

    # Ascii string (fail)
    def test_ascii_string(self):
        res = API("/tips/", get_data=self.query_string[1])
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Unicode string (fail)
    def test_unicode_string(self):
        res = API("/tips/", get_data=self.query_string[2])
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Time statistics
    def test_time_statistics(self):
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/tips/", get_data="")
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "get_tips")

    @classmethod
    def setUpClass(cls):
        rand_tag_27 = gen_rand_trytes(27)
        cls.query_string = ["", rand_tag_27, "飛天義大利麵神教"]
