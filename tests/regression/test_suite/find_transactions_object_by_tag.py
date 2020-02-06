from common import *
import json
import unittest
import time
import logging


class FindTransactionsObjectsByTag(unittest.TestCase):

    # Normal 27 trytes tag (pass)
    @test_logger
    def test_27_trytes(self):
        res = API(f"/tag/{self.query_string[0]}", get_data="")
        self._verify_pass(res)

    # Normal 27 trytes tag with trailing slash (pass)
    @test_logger
    def test_with_trailing_slash(self):
        res = API(f"/tag/{self.query_string[0]}/", get_data="")
        self._verify_pass(res)

    # Empty tag (fail)
    @test_logger
    def test_empty_tag(self):
        res = API(f"/tag/{self.query_string[1]}", get_data="")
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Random 26-length trytes with one non-tryte character (fail)
    @test_logger
    def test_non_trytes_character(self):
        res = API(f"/tag/{self.query_string[2]}", get_data="")
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Tag with null-terminating character (pass)
    @test_logger
    def test_null_terminating_character(self):
        res = API(f"/tag/{self.query_string[3]}", get_data="")
        self._verify_pass(res)

    # Normal trytes tag with null-terminating character with random trytes tag (pass)
    @test_logger
    def test_null_between_two_trytes(self):
        res = API(f"/tag/{self.query_string[4]}", get_data="")
        self._verify_pass(res)

    # Chinese (unicode) character tag (fail)
    @test_logger
    def test_unicode_characters(self):
        res = API(f"/tag/{self.query_string[5]}", get_data="")
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Tag length larger than 27 (fail)
    @test_logger
    def test_tag_too_long(self):
        res = API(f"/tag/{self.query_string[6]}", get_data="")
        self.assertEqual(STATUS_CODE_400, res["status_code"])

    # Time statistics
    @test_logger
    def test_time_statistics(self):
        time_cost = []
        for i in range(TIMES_TOTAL):
            start_time = time.time()
            API(f"/tag/{self.query_string[0]}", get_data="")
            time_cost.append(time.time() - start_time)

        eval_stat(time_cost, "find transactions objects by tag")

    @classmethod
    def setUpClass(cls):
        rand_trytes_26 = gen_rand_trytes(26)
        rand_tag = gen_rand_trytes(LEN_TAG)
        rand_addr = gen_rand_trytes(LEN_ADDR)
        rand_msg = gen_rand_trytes(30)
        rand_len = random.randrange(28, 50)
        rand_len_trytes = gen_rand_trytes(rand_len)
        FindTransactionsObjectsByTag()._send_transaction(
            rand_msg, rand_tag, rand_addr)
        cls.query_string = [
            rand_tag, "", f"{rand_trytes_26}@", f"{rand_tag}\x00",
            f"{rand_tag}\x00{rand_tag}", "一二三四五", rand_len_trytes
        ]

    def _send_transaction(self, msg, tag, addr):
        post_field = ["value", "message", "tag", "address"]
        query = [0, msg, tag, addr]
        res = API("/transaction/", post_data=map_field(post_field, query))
        # We don't use _verify_pass here since this function is tested in another test suite
        self.assertEqual(STATUS_CODE_200, res["status_code"])

    def _verify_pass(self, res):
        self.assertEqual(STATUS_CODE_200, res["status_code"])
        res_json = json.loads(res["content"])[0]

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
