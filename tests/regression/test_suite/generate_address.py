from common import *
import json
import unittest
import time
import logging


class GenerateAddress(unittest.TestCase):

    # Without additional GET parameter (pass)
    @test_logger
    def test_normal(self):
        res = API("/address/", get_data=self.query_string[0])
        res_json = json.loads(res["content"])

        self.assertEqual(STATUS_CODE_200, res["status_code"])
        self.assertTrue(valid_trytes(res_json[0], LEN_ADDR))

    # Ascii string (fail)
    @test_logger
    def test_ascii_string(self):
        res = API("/address/", get_data=self.query_string[1])
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Unicode string (fail)
    @test_logger
    def test_unicode_string(self):
        res = API("/address/", get_data=self.query_string[2])
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Time statistics
    @test_logger
    def test_time_statistics(self):
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/address/", get_data="")
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "generate_address")

    @classmethod
    def setUpClass(cls):
        rand_tag_27 = gen_rand_trytes(27)
        cls.query_string = ["", rand_tag_27, "飛天義大利麵神教"]
