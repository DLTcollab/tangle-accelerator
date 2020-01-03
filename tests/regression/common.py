import sys
import json
import random
import logging
import requests
import statistics
import subprocess

DEBUG_FLAG = False
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
    if len(sys.argv) == 2:
        raw_url = sys.argv[1]
    elif len(sys.argv) == 4:
        raw_url = sys.argv[1]
        if sys.argv[2] == 'Y':
            DEBUG_FLAG = True

        # the 3rd arg is the option which determine if use the debugging mode of statistical tests
        if sys.argv[3] == 'Y':
            TIMES_TOTAL = 2
    else:
        raw_url = "localhost:8000"
    URL = "http://" + raw_url


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
    try:
        response = {}
        if get_data is not None:
            r = requests.get(str(URL + get_query + get_data), timeout=TIMEOUT)
            response = {"content": r.text, "status_code": str(r.status_code)}

        elif post_data is not None:
            command = "curl " + str(
                URL + get_query
            ) + " -X POST -H 'Content-Type: application/json' -w \", %{http_code}\" -d '" + str(
                post_data) + "'"
            logging.debug("curl command = " + command)
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

    return response
