from common import *
import json
import unittest
import time
import logging


class SendTransfer(unittest.TestCase):

    # Positive value, tryte message, tryte tag, tryte address (pass)
    @test_logger
    def test_normal(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[0]))
        self._verify_pass(res)

    # Zero value, tryte message, tryte tag, tryte address (pass)
    @test_logger
    def test_zero_value(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[1]))
        self._verify_pass(res)

    # Chinese value, tryte message, tryte tag, tryte address (pass)
    @test_logger
    def test_chinese_value(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[2]))
        self._verify_pass(res)

    # Zero value, chinese message, tryte tag, tryte address (fail)
    @test_logger
    def test_chinese_message(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[3]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # Zero value, tryte message, chinese tag, tryte address (fail)
    @test_logger
    def test_chinese_tag(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[4]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # Negative value, tryte message, tryte tag, tryte address (pass)
    @test_logger
    def test_negative_value(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[5]))
        self._verify_pass(res)

    # No value, tryte message, tryte tag, tryte address (pass)
    @test_logger
    def test_no_value(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[6]))
        self._verify_pass(res)

    # Zero value, no message, tryte tag, tryte address (pass)
    @test_logger
    def test_no_message(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[7]))
        self._verify_pass(res)

    # Zero value, tryte message, no tag, tryte address (pass)
    @test_logger
    def test_no_tag(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[8]))
        self._verify_pass(res)

    # Zero value, tryte message, tryte tag, no address (pass)
    @test_logger
    def test_no_address(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[9]))
        self._verify_pass(res)

    # Zero value, tryte message, tryte tag, unicode address (fail)
    @test_logger
    def test_unicode_address(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[10]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])
    
    # Zero value, tryte message, invalid tag, tryte address (fail)
    @test_logger
    def test_invalid_tag(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[11]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])
    
    # Zero value, tryte message, tryte tag, invalid address (fail)
    @test_logger
    def test_invalid_address(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[12]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # Zero value, tryte message, invalid tag, invalid address (fail)
    @test_logger
    def test_invalid_tag_and_address(self):
        res = API("/transaction/",
                  post_data=map_field(self.post_field, self.query_string[13]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # Time statistics
    @test_logger
    def test_time_statistics(self):
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

    @classmethod
    @test_logger
    def setUpClass(cls):
        rand_msg = gen_rand_trytes(30)
        rand_tag = gen_rand_trytes(27)
        rand_addr = gen_rand_trytes(81)
        rand_invalid_tag = gen_rand_trytes(999)
        rand_invalid_addr = gen_rand_trytes(999)
        cls.post_field = ["value", "message", "tag", "address"]
        cls.query_string = [[420, rand_msg, rand_tag, rand_addr],
                            [0, rand_msg, rand_tag, rand_addr],
                            ["生而為人, 我很抱歉", rand_msg, rand_tag, rand_addr],
                            [0, "生而為人, 我很抱歉", rand_tag, rand_addr],
                            [0, rand_msg, "生而為人, 我很抱歉", rand_addr],
                            [-5, rand_msg, rand_tag, rand_addr],
                            [None, rand_msg, rand_tag, rand_addr],
                            [0, None, rand_tag, rand_addr],
                            [0, rand_msg, None, rand_addr],
                            [0, rand_msg, rand_tag, None],
                            [0, rand_msg, rand_tag, "我思故我在"],
                            [0, rand_msg, "ololaola", rand_addr],
                            [0, rand_msg, rand_tag, "dio"],
                            [0, rand_msg, "olaolaola", "dio"],
                           ]

    def _verify_pass(self, res):
        self.assertEqual(STATUS_CODE_200, res["status_code"])
        res_json = json.loads(res["content"])

        # We only send zero tx at this moment
        self.assertEqual(0, res_json["value"])
        self.assertTrue(valid_trytes(res_json["tag"], LEN_TAG))
        self.assertTrue(valid_trytes(res_json["address"], LEN_ADDR))
        self.assertTrue(
            valid_trytes(res_json["trunk_transaction_hash"], LEN_ADDR))
        self.assertTrue(
            valid_trytes(res_json["branch_transaction_hash"], LEN_ADDR))
        self.assertTrue(valid_trytes(res_json["bundle_hash"], LEN_ADDR))
        self.assertTrue(valid_trytes(res_json["hash"], LEN_ADDR))
        self.assertTrue(
            valid_trytes(res_json["signature_and_message_fragment"],
                         LEN_MSG_SIGN))
