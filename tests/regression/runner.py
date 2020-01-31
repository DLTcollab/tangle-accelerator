#!/usr/bin/env python3

# Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
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
    ver = sys.version_info
    if ver.major < 3 or (ver.major == 3 and ver.minor < 6):
        raise Exception("Must be using Python 3.6 or greater")

    parse_cli_arg()

    suite_path = os.path.join(os.path.dirname(__file__), "test_suite")
    sys.path.append(suite_path)
    for module in os.listdir(suite_path):
        if module[-3:] == ".py":
            mod = __import__(module[:-3], locals(), globals())
            suite = unittest.TestLoader().loadTestsFromModule(mod)
            result = unittest.TextTestRunner().run(suite)
            if not result.wasSuccessful():
                exit(1)
