/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

/* To disable some checks of sanitizer tools to certain Bazel tests, one way is
 * redirecting test source files to empty_test.c in the Bazel BULID file. Using
 * Bazel `select()` function with user-defined `config_setting` to achieve the
 * ignorement of sanitrizer tools. For exmaple, command `bazel test //tests/test_scylladb`
 * will perform tests in `test_scylladb.c` only when users add `--define db=enable`
 * at build time. Otherwise, the tests will be redirecting to this file.
 */
int main(void) { return 0; }