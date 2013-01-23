/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** Clownfish::CFC::Test - An object to collect results from test runs and
 * print summaries in multiple formats.
 */

#ifndef H_CFCTEST
#define H_CFCTEST

#include <time.h>

#ifdef CFC_USE_TEST_MACROS
  #define OK      CFCTest_test_true
  #define STR_EQ  CFCTest_test_string_equals
  #define INT_EQ  CFCTest_test_int_equals
#endif

typedef struct CFCTest CFCTest;

typedef struct CFCTestBatch {
    const char *name;
    int num_planned;
    void (*run)(CFCTest *test);
} CFCTestBatch;

struct CFCClass;
struct CFCFunction;
struct CFCMethod;
struct CFCParamList;
struct CFCParcel;
struct CFCParser;
struct CFCType;
struct CFCVariable;

/** Create a new test object.
 *
 * @param formatter_name Name of the built-in formatter that should be used
 * to create the output. Supported values are "tap" for TAP output and
 * "clownfish" for the generic Clownfish format.
 * @return a new test object.
 */
CFCTest*
CFCTest_new(const char *formatter_name);

CFCTest*
CFCTest_init(CFCTest *self, const char *formatter_name);

void
CFCTest_destroy(CFCTest *self);

/** Run all test batches.
 *
 * @return true if all tests were successful.
 */
int
CFCTest_run_all(CFCTest *self);

/** Run a test batch by name.
 *
 * @param name Name of the test batch.
 * @return true if all tests in the batch were successful.
 */
int
CFCTest_run_batch(CFCTest *self, const char *name);

/* Collect result of a test.
 *
 * @param cond Test condition. True if the test succeeded, false if it failed.
 * @param fmt printf-like format string describing the test.
 */
void
CFCTest_test_true(CFCTest *self, int cond, const char *fmt, ...);

/* Test strings for equality and collect result.
 *
 * @param result Result string to be tested.
 * @param expected Expected result string.
 * @param fmt printf-like format string describing the test.
 */
void
CFCTest_test_string_equals(CFCTest *self, const char *result,
                           const char *expected, const char *fmt, ...);

/* Test integers for equality and collect result.
 *
 * @param result Result integer to be tested.
 * @param expected Expected result integer.
 * @param fmt printf-like format string describing the test.
 */
void
CFCTest_test_int_equals(CFCTest *self, long result, long expected,
                        const char *fmt, ...);

/* Skip tests.
 *
 * @param num Number of tests to skip.
 */
void
CFCTest_skip(CFCTest *self, int num);

/* Finish testing.
 *
 * @return true if tests were run and all tests were successful.
 */
int
CFCTest_finish(CFCTest *self);

/* Utility functions for tests. */

struct CFCParcel*
CFCTest_parse_parcel(CFCTest *test, struct CFCParser *parser, const char *src);

struct CFCType*
CFCTest_parse_type(CFCTest *test, struct CFCParser *parser, const char *src);

struct CFCVariable*
CFCTest_parse_variable(CFCTest *test, struct CFCParser *parser,
                       const char *src);

struct CFCParamList*
CFCTest_parse_param_list(CFCTest *test, struct CFCParser *parser,
                         const char *src);

struct CFCFunction*
CFCTest_parse_function(CFCTest *test, struct CFCParser *parser,
                       const char *src);

struct CFCMethod*
CFCTest_parse_method(CFCTest *test, struct CFCParser *parser, const char *src);

struct CFCClass*
CFCTest_parse_class(CFCTest *test, struct CFCParser *parser, const char *src);

time_t
CFCTest_get_file_mtime(const char *path);

void
CFCTest_set_file_times(const char *path, time_t time);

/* Test batch structs. */

extern const CFCTestBatch CFCTEST_BATCH_CLASS;
extern const CFCTestBatch CFCTEST_BATCH_C_BLOCK;
extern const CFCTestBatch CFCTEST_BATCH_DOCU_COMMENT;
extern const CFCTestBatch CFCTEST_BATCH_FILE;
extern const CFCTestBatch CFCTEST_BATCH_FILE_SPEC;
extern const CFCTestBatch CFCTEST_BATCH_FUNCTION;
extern const CFCTestBatch CFCTEST_BATCH_HIERARCHY;
extern const CFCTestBatch CFCTEST_BATCH_METHOD;
extern const CFCTestBatch CFCTEST_BATCH_PARAM_LIST;
extern const CFCTestBatch CFCTEST_BATCH_PARCEL;
extern const CFCTestBatch CFCTEST_BATCH_PARSER;
extern const CFCTestBatch CFCTEST_BATCH_SYMBOL;
extern const CFCTestBatch CFCTEST_BATCH_TYPE;
extern const CFCTestBatch CFCTEST_BATCH_UTIL;
extern const CFCTestBatch CFCTEST_BATCH_VARIABLE;
extern const CFCTestBatch CFCTEST_BATCH_VERSION;

#endif /* H_CFCTEST */

