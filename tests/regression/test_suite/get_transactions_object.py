from common import *
import json
import unittest
import time
import logging


class GetTransactionsObject(unittest.TestCase):

    # 81 trytes transaction hash (pass)
    @test_logger
    def test_81_trytes_hash(self):
        res = API("/transaction/object",
                  post_data=map_field(self.post_field, [self.query_string[0]]))
        self._verify_pass(res, 0)

    # Multiple 81 trytes transaction hash (pass)
    @test_logger
    def test_mult_81_trytes_hash(self):
        res = API("/transaction/object",
                  post_data=map_field(self.post_field, [self.query_string[1]]))
        self._verify_pass(res, 1)

    # 20 trytes transaction hash (fail)
    @test_logger
    def test_20_trytes_hash(self):
        res = API("/transaction/object",
                  post_data=map_field(self.post_field, [self.query_string[2]]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # 100 trytes transaction hash (fail)
    @test_logger
    def test_100_trytes_hash(self):
        res = API("/transaction/object",
                  post_data=map_field(self.post_field, [self.query_string[3]]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # Unicode transaction hash (fail)
    @test_logger
    def test_unicode_hash(self):
        res = API("/transaction/object",
                  post_data=map_field(self.post_field, [self.query_string[4]]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # Null Transaction hash (fail)
    @test_logger
    def test_null_hash(self):
        res = API("/transaction/object",
                  post_data=map_field(self.post_field, [self.query_string[5]]))
        self.assertEqual(STATUS_CODE_500, res["status_code"])

    # Time statistics
    @test_logger
    def test_time_statistics(self):
        time_cost = []
        post_data_json = json.dumps({"hashes": self.query_string[0]})
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/transaction/object", post_data=post_data_json)
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "find transaction objects")

    @classmethod
    def setUpClass(cls):
        sent_txn_tmp = []
        for i in range(3):
            tx_post_data = {
                "value": 0,
                "message": gen_rand_trytes(27),
                "tag": gen_rand_trytes(30),
                "address": gen_rand_trytes(81)
            }
            tx_post_data_json = json.dumps(tx_post_data)
            sent_txn_obj = API("/transaction/", post_data=tx_post_data_json)

            logging.debug(f"sent_transaction_obj = {sent_txn_obj}")

            unittest.TestCase().assertEqual(STATUS_CODE_200,
                                            sent_txn_obj["status_code"])
            sent_txn_obj_json = json.loads(sent_txn_obj["content"])
            sent_txn_tmp.append(sent_txn_obj_json)
        cls.sent_txn = [[sent_txn_tmp[0]], [sent_txn_tmp[1], sent_txn_tmp[2]]]
        cls.post_field = ["hashes"]
        cls.response_field = []
        cls.query_string = [[sent_txn_tmp[0]["hash"]],
                            [sent_txn_tmp[1]["hash"], sent_txn_tmp[2]["hash"]],
                            gen_rand_trytes(19),
                            gen_rand_trytes(100), "工程師批哩趴啦的生活", ""]

    def _verify_pass(self, res, idx):
        expected_txns = self.sent_txn[idx]
        self.assertEqual(STATUS_CODE_200, res["status_code"])
        res_txn = json.loads(res["content"])
        for txn in expected_txns:
            self.assertIn(txn, res_txn)
