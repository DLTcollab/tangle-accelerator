import time
import re
import sys
import json
import string
import random
import logging
import requests
import statistics
import subprocess
import argparse
import paho.mqtt.publish as publish
import paho.mqtt.subscribe as subscribe
from multiprocessing import Pool
from multiprocessing.context import TimeoutError

TIMES_TOTAL = 100
TIMEOUT = 100  # [sec]
MQTT_RECV_TIMEOUT = 30
STATUS_CODE_500 = "500"
STATUS_CODE_405 = "405"
STATUS_CODE_404 = "404"
STATUS_CODE_400 = "400"
STATUS_CODE_200 = "200"
STATUS_CODE_ERR = "-1"
EMPTY_REPLY = "000"
LEN_TAG = 27
LEN_ADDR = 81
LEN_MSG_SIGN = 2187
TRYTE_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ9"
URL = ""
DEVICE_ID = None
CONNECTION_METHOD = None


def parse_cli_arg():
    global URL
    global CONNECTION_METHOD
    global DEVICE_ID
    rand_device_id = ''.join(
        random.choice(string.printable[:62]) for _ in range(32))
    parser = argparse.ArgumentParser('Regression test runner program')
    parser.add_argument('-u',
                        '--url',
                        dest='raw_url',
                        default="localhost:8000")
    parser.add_argument('-d', '--debug', dest="debug", action="store_true")
    parser.add_argument('--nostat', dest="no_stat", action="store_true")
    parser.add_argument('--mqtt', dest="enable_mqtt", action="store_true")
    parser.add_argument('--device_id',
                        dest="device_id",
                        default=rand_device_id)
    args = parser.parse_args()

    # Determine whether to use full time statistic or not
    if args.no_stat:
        global TIMES_TOTAL
        TIMES_TOTAL = 2
    # Determine connection method
    if args.enable_mqtt:
        CONNECTION_METHOD = "mqtt"
        URL = "localhost"
    else:
        CONNECTION_METHOD = "http"
        URL = "http://" + args.raw_url
    # Run with debug mode or not
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    # Configure connection destination
    DEVICE_ID = args.device_id


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

    def decorate(*args, **kwargs):
        bg_color = "\033[48;5;38m"
        fg_color = "\033[38;5;16m"
        clear_color = "\033[0m"
        logger.info(f"{bg_color}{fg_color}{name}{clear_color}")
        res = f(*args, **kwargs)
        return res

    return decorate


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


# Simulate random field to mqtt since we cannot put the information in the route
def add_random_field(post):
    return data


def route_http_to_mqtt(query, get_data, post_data):
    data = {}
    if get_data: query += get_data
    if post_data:
        data.update(json.loads(post_data))
    if query[-1] == "/": query = query[:-1]  # Remove trailing slash

    # api_generate_address
    r = re.search("/address$", query)
    if r is not None:
        return query, data

    # api_find_transactions_by_tag
    r = re.search(f"/tag/(?P<tag>[\x00-\xff]*?)/hashes$", query)
    if r is not None:
        tag = r.group("tag")
        data.update({"tag": tag})
        query = "/tag/hashes"
        return query, data

    # api_find_transactions_object_by_tag
    r = re.search(f"/tag/(?P<tag>[\x00-\xff]*?)$", query)
    if r is not None:
        tag = r.group("tag")
        data.update({"tag": tag})
        query = "/tag/object"
        return query, data

    # api_find_transacion_object
    r = re.search(f"/transaction/object$", query)
    if r is not None:
        query = "/transaction/object"
        return query, data

    r = re.search(f"/transaction/(?P<hash>[\u0000-\uffff]*?)$", query)
    if r is not None:
        hash = r.group("hash")
        data.update({"hash": hash})
        query = f"/transaction"
        return query, data

    # api_send_transfer
    r = re.search(f"/transaction$", query)
    if r is not None:
        query = "/transaction/send"
        return query, data

    # api_get_tips
    r = re.search(f"/tips$", query)
    if r is not None:
        query = "/tips/all"
        return query, data

    # api_get_tips_pair
    r = re.search(f"/tips/pair$", query)
    if r is not None:
        return query, data

    # api_send_trytes
    r = re.search(f"/tryte$", query)
    if r is not None:
        return query, data

    # Error, cannot identify route (return directly from regression test)
    return None, None


def API(get_query, get_data=None, post_data=None):
    global CONNECTION_METHOD
    assert CONNECTION_METHOD != None
    if CONNECTION_METHOD == "http":
        return _API_http(get_query, get_data, post_data)
    elif CONNECTION_METHOD == "mqtt":
        query, data = route_http_to_mqtt(get_query, get_data, post_data)
        if (query, data) == (None, None):
            msg = {
                "message":
                "Cannot identify route, directly return from regression test",
                "status_code": STATUS_CODE_400
            }
            logging.debug(msg)
            return msg

        return _API_mqtt(query, data)


def _API_http(get_query, get_data, post_data):
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


def _subscribe(get_query):
    add_slash = ""
    if get_query[-1] != "/": add_slash = "/"
    topic = f"root/topics{get_query}{add_slash}{DEVICE_ID}"
    logging.debug(f"Subscribe topic: {topic}")

    return subscribe.simple(topics=topic, hostname=URL, qos=1).payload


def _API_mqtt(get_query, data):
    global URL, DEVICE_ID
    data.update({"device_id": DEVICE_ID})

    # Put subscriber in a thread since it is a blocking function
    with Pool() as p:
        payload = p.apply_async(_subscribe, [get_query])
        topic = f"root/topics{get_query}"
        logging.debug(f"Publish topic: {topic}, data: {data}")

        # Prevents publish execute earlier than subscribe
        time.sleep(0.1)

        # Publish requests
        publish.single(topic, json.dumps(data), hostname=URL, qos=1)
        msg = {}
        try:
            res = payload.get(MQTT_RECV_TIMEOUT)
            msg = json.loads(res)

            if type(msg) is dict and "message" in msg.keys():
                content = msg["message"]
                if content == "Internal service error":
                    msg.update({"status_code": STATUS_CODE_500})
                elif content == "Request not found":
                    msg.update({"status_code": STATUS_CODE_404})
                elif content == "Invalid path" or content == "Invalid request header":
                    msg.update({"status_code": STATUS_CODE_400})
                else:
                    msg.update({"status_code": STATUS_CODE_200})
            else:
                msg = {
                    "content": json.dumps(msg),
                    "status_code": STATUS_CODE_200
                }
        except TimeoutError:
            msg = {
                "content": "Time limit exceed",
                "status_code": STATUS_CODE_ERR
            }

    logging.debug(f"Modified response: {msg}")
    return msg
