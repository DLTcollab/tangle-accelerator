from common import *
import json
import unittest
import time
import logging


# Note: We change the GET request to POST flavor, so the error
# message between MQTT and HTTP connections are not the same.
class GetTransactionsObjectSingle(unittest.TestCase):

    # 81 trytes transaction hash (pass)
    @test_logger
    def test_81_trytes_hash(self):
        res = API("/transaction/", get_data=self.query_string[0])
        self._verify_pass(res)

    # 20 trytes transaction hash (fail)
    @test_logger
    def test_20_trytes_hash(self):
        res = API("/transaction/", get_data=self.query_string[1])
        if CONNECTION_METHOD == "mqtt":
            self.assertEqual(STATUS_CODE_500, res["status_code"])
        else:
            self.assertEqual(STATUS_CODE_400, res["status_code"])

    # 100 trytes transaction hash (fail)
    @test_logger
    def test_100_trytes_hash(self):
        res = API("/transaction/", get_data=self.query_string[2])
        if CONNECTION_METHOD == "mqtt":
            self.assertEqual(STATUS_CODE_404, res["status_code"])
        else:
            self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Unicode transaction hash (fail)
    @test_logger
    def test_unicode_hash(self):
        res = API("/transaction/", get_data=self.query_string[3])
        if CONNECTION_METHOD == "mqtt":
            self.assertEqual(STATUS_CODE_500, res["status_code"])
        else:
            self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Null Transaction hash (fail)
    @unittest.skipIf(CONNECTION_METHOD == "mqtt",
                     "MQTT connection does not have GET/POST method")
    @test_logger
    def test_null_hash(self):
        res = API("/transaction/", get_data=self.query_string[4])
        self.assertEqual(STATUS_CODE_405,
                         res["status_code"])  # Method not allowed

    # Time statistics
    @test_logger
    def test_time_statistics(self):
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API("/transaction/", get_data=self.query_string[0])
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "find transaction objects")

    @classmethod
    @test_logger
    def setUpClass(cls):
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
        cls.sent_txn = json.loads(sent_txn_obj["content"])
        cls.post_field = ["hashes"]
        cls.response_field = []
        cls.query_string = [
            cls.sent_txn["hash"],
            gen_rand_trytes(20),
            gen_rand_trytes(100), "工程師批哩趴啦的生活", ""
        ]

    def _verify_pass(self, res):
        self.assertEqual(STATUS_CODE_200, res["status_code"])
        res_txn = json.loads(res["content"])
        self.assertEqual(self.sent_txn, res_txn)
