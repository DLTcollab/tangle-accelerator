#!/usr/bin/env python3

# Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
# All Rights Reserved.
# This is free software; you can redistribute it and/or modify it under the
# terms of the MIT license. A copy of the license can be found in the file
# "LICENSE" at the root of this distribution.

# Run in Python3
from common import *
import os
import sys
import unittest
import logging

# Run all the API Test here
if __name__ == '__main__':
    if DEBUG_FLAG == True:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)
    parse_cli_arg()

    suite_path = os.path.join(os.path.dirname(__file__), "test_suite")
    sys.path.append(suite_path)
    for module in os.listdir(suite_path):
        if module[-3:] == ".py":
            mod = __import__(module[:-3], locals(), globals())
            suite = unittest.TestLoader().loadTestsFromModule(mod)
            unittest.TextTestRunner().run(suite)
