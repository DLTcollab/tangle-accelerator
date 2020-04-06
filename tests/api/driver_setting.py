#!/usr/bin/env python3

import os
import sys
import random
import logging
# User who wants to run this script should use command `pip install pyota` to install module `iota`
from iota import ProposedTransaction
from iota import Address
from iota import Tag
from iota import TryteString
from iota import Hash
from iota import Transaction
from iota import Iota

payload = "I am a very foolish fond old man, Fourscore and upward, not an hour more nor less. And to deal plainly I fear I am not in my perfect mind."
sent_transaction_number = 2
TRYTE_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ9"
URL = ""
BAZEL_TEST_ARGS = ""
BAZEL_BUILD_CMDS = ""


# Send a transaction with IRI core API.
def parse_cli_arg():
    global URL
    global BAZEL_BUILD_CMDS
    global BAZEL_TEST_ARGS

    logging.basicConfig(level=logging.DEBUG)

    URL = f"http://{sys.argv[1]}"
    BAZEL_BUILD_CMDS = sys.argv[2]
    BAZEL_TEST_ARGS = sys.argv[3]


def gen_rand_trytes(tryte_len):
    trytes = ""
    for i in range(tryte_len):
        trytes = trytes + TRYTE_ALPHABET[random.randint(0, 26)]
    return trytes


if __name__ == '__main__':
    parse_cli_arg()
    api = Iota(f'{URL}', testnet=True)
    txn_array = []
    tag = gen_rand_trytes(27)
    address = []
    message = []
    for i in range(sent_transaction_number):
        address.append(gen_rand_trytes(81))
        message.append(TryteString.from_unicode(f'{payload}, number: {i}'))

        tx_elt = ProposedTransaction(address=Address(address[i]),
                                     tag=tag,
                                     message=message[i],
                                     value=0)
        txn_array.append(api.send_transfer(transfers=[tx_elt]))
        curr_hash = txn_array[i]['bundle'].tail_transaction.hash
        logging.info(
            f'hash: {curr_hash}, address: {address[i]}, tag: {tag}, message: {message[i]}'
        )

    # Get raw transaction trytes
    txn_trytes = []
    txn_hashes = []
    txn_tags = []
    for i in range(sent_transaction_number):
        txn_trytes.append(
            txn_array[i]['bundle'].tail_transaction.as_tryte_string())
        txn_hashes.append(txn_array[i]['bundle'].tail_transaction.hash)
        txn_tags.append(txn_array[i]['bundle'].tail_transaction.tag)

    #==============Fetch transaction==============#
    get_trytes_res = api.get_trytes(txn_hashes)

    if get_trytes_res == None:
        logging.error("None responded transactions")
        exit(1)
    else:
        length = len(get_trytes_res['trytes'])
        for i in range(length):
            txn = Transaction.from_tryte_string(get_trytes_res['trytes'][i])
            if txn.tag != tag:
                logging.error(f"Error expect: {tag}\nactual: {txn.tag[0:27]}")
                exit(1)
            if not (address[i] in txn.address):
                logging.error(
                    f"address Error expect: {address[i]}\nactual: {txn.address[0:81]}"
                )
                exit(1)
            if not (message[i] in txn.signature_message_fragment):
                logging.error(
                    f"message Error expect: {message[i]}\nactual: {txn.signature_message_fragment[0:81]}"
                )
                exit(1)

            if sent_transaction_number == length:
                logging.info("Successful")
            else:
                logging.info("Failed")
                exit(1)
    ret = os.system(
        f"{BAZEL_BUILD_CMDS} {BAZEL_TEST_ARGS} --test_arg --tag={txn_tags[0]} --test_arg --hash={txn_hashes[0]} --test_arg --hash={txn_hashes[1]}"
    )
    if ret == 0:
        exit(0)
    else:
        exit(1)
