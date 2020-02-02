import sys
import json
import random
import logging
import requests
import statistics
import subprocess
import argparse

TIMES_TOTAL = 100
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
TRYTE_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ9"
URL = ""


def parse_cli_arg():
    global URL
    parser = argparse.ArgumentParser('Regression test runner program')
    parser.add_argument('-u',
                        '--url',
                        dest='raw_url',
                        default="localhost:8000")
    parser.add_argument('-d', '--debug', dest="debug", action="store_true")
    parser.add_argument('--nostat', dest="no_stat", action="store_true")
    args = parser.parse_args()

    if args.no_stat:
        global TIMES_TOTAL
        TIMES_TOTAL = 2
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    URL = "http://" + args.raw_url


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
        trytes = trytes + TRYTE_ALPHABET[random.randint(0, 26)]
    return trytes


def test_logger(f):
    logger = logging.getLogger(f.__module__)
    name = f.__name__

    def decorate(instance):
        logger.debug(f"Testing case = {name}")
        return instance

    return decorate(f)


def valid_trytes(trytes, trytes_len):
    if len(trytes) != trytes_len:
        return False

    for char in trytes:
        if char not in TRYTE_ALPHABET:
            return False

    return True


def map_field(key, value):
    ret = {}
    for k, v in zip(key, value):
        ret.update({k: v})
    return json.dumps(ret)


def API(get_query, get_data=None, post_data=None):
    global URL
    command = "curl {} -X POST -H 'Content-Type: application/json' -w \", %{{http_code}}\" -d '{}'"
    try:
        response = {}
        if get_data is not None:
            command = str(URL + get_query + get_data)
            r = requests.get(command, timeout=TIMEOUT)
            response = {"content": r.text, "status_code": str(r.status_code)}

        elif post_data is not None:
            command = command.format(URL + get_query, post_data)
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
        logging.error(URL, "Timeout!")
        logging.error('\n    ' + repr(sys.exc_info()))
        return None
    if not response:
        response = None

    logging.debug(f"Command = {command}, response = {response}")

    return response
