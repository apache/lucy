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

/* Charmonizer/Test.h - test Charmonizer's output.
 */

#ifndef H_CHAZ_TEST
#define H_CHAZ_TEST

#ifdef __cplusplus
extern "C" {
#endif

#include "Charmonizer/Core/Defines.h"

typedef struct chaz_TestBatch chaz_TestBatch;

/* Begin a test run.
 */
chaz_TestBatch*
chaz_Test_start(unsigned num_tests);

/* End a test run.  Returns true if all tests were run and there were no
 * failures, false otherwise.
 */
int
chaz_Test_finish(void);

/* Destructor.
 */
typedef void
(*chaz_TestBatch_destroy_t)(chaz_TestBatch *batch);

/* This function, which is unique to each TestBatch, actually runs the test
 * sequence.
 */
typedef void
(*chaz_TestBatch_test_func_t)(chaz_TestBatch *batch);

/* Print a header, execute the test sequence, print a report.
 */
typedef void
(*chaz_TestBatch_run_test_t)(chaz_TestBatch *batch);

struct chaz_TestBatch {
    char         *name;
    unsigned      test_num;
    unsigned      num_tests;
    unsigned      num_passed;
    unsigned      num_failed;
    unsigned      num_skipped;
    chaz_TestBatch_destroy_t      destroy;
    chaz_TestBatch_test_func_t    test_func;
    chaz_TestBatch_run_test_t     run_test;
};

/* Unbuffer stdout.  Perform any other setup needed.
 */
void
chaz_Test_init(void);

/* Constructor for TestBatch.
 */
chaz_TestBatch*
chaz_Test_new_batch(const char *batch_name, unsigned num_tests,
                    chaz_TestBatch_test_func_t test_func);

/* Note: maybe add line numbers later.
 */
#define CHAZ_TEST_PLAN              chaz_Test_plan
#define CHAZ_TEST_TEST_STR_EQ       chaz_Test_test_str_eq

/* Print a message indicating that a test was skipped and update batch.
 */
#define CHAZ_TEST_SKIP(batch, message) \
    chaz_Test_skip(batch, message)

/* Print a message indicating that all remaining tests will be skipped and
 * return.
 */
#define CHAZ_TEST_SKIP_REMAINING(batch, message) \
    do { \
        chaz_Test_report_skip_remaining(batch, message); \
        return; \
    } while (0)

void
chaz_Test_plan(chaz_TestBatch *batch);

#define CHAZ_TEST_OK(_expression, _message) \
	chaz_Test_ok(chaz_Test_current, (_expression), (_message))
void
chaz_Test_ok(chaz_TestBatch *batch, int expression, const char *message);

#define CHAZ_TEST_STR_EQ(_got, _expected, _message) \
	chaz_Test_str_eq(chaz_Test_current, (_got), (_expected), (_message))
void
chaz_Test_str_eq(chaz_TestBatch *batch, const char *got,
                 const char *expected, const char *message);

#define CHAZ_TEST_PASS(_message) \
	chaz_Test_pass(chaz_Test_current, (_message))
void
chaz_Test_pass(chaz_TestBatch *batch, const char *message);

#define CHAZ_TEST_FAIL(_message) \
	chaz_Test_fail(chaz_Test_current, (_message))
void
chaz_Test_fail(chaz_TestBatch *batch, const char *message);

#define CHAZ_TEST_LONG_EQ(_got, _expected, _message) \
	chaz_Test_long_eq(chaz_Test_current, (_got), (_expected), (_message))
void
chaz_Test_long_eq(chaz_TestBatch *batch, long got, long expected,
                  const char *message);

#define CHAZ_TEST_DOUBLE_EQ(_got, _expected, _slop, _message) \
    chaz_Test_double_eq(chaz_Test_current, (_got), (_expected), (_slop), \
                        (_message))
void
chaz_Test_double_eq(chaz_TestBatch *batch, double got, double expected,
                    double slop, const char *message);

void
chaz_Test_skip(chaz_TestBatch *batch, const char *message);

void
chaz_Test_report_skip_remaining(chaz_TestBatch* batch,
                                const char *message);

/* Global TestBatch implicitly accessed by testing macros. */
extern chaz_TestBatch *chaz_Test_current;

#ifdef CHAZ_USE_SHORT_NAMES
  #define Test_start                   chaz_Test_start
  #define Test_finish                  chaz_Test_finish
  #define OK                           CHAZ_TEST_OK
  #define STR_EQ                       CHAZ_TEST_STR_EQ
  #define LONG_EQ                      CHAZ_TEST_LONG_EQ
  #define DOUBLE_EQ                    CHAZ_TEST_DOUBLE_EQ
  #define PASS                         CHAZ_TEST_PASS
  #define FAIL                         CHAZ_TEST_FAIL
  #define TestBatch_destroy_t          chaz_TestBatch_destroy_t
  #define TestBatch_test_func_t        chaz_TestBatch_test_func_t
  #define TestBatch_run_test_t         chaz_TestBatch_run_test_t
  #define TestBatch                    chaz_TestBatch
  #define Test_init                    chaz_Test_init
  #define Test_new_batch               chaz_Test_new_batch
  #define Test_plan                    chaz_Test_plan
  #define PLAN                         CHAZ_TEST_PLAN
  #define Test_str_eq                  chaz_Test_str_eq
  #define Test_long_eq                 chaz_Test_long_eq
  #define Test_double_eq               chaz_Test_double_eq
  #define Test_pass                    chaz_Test_pass
  #define Test_fail                    chaz_Test_fail
  #define Test_skip                    chaz_Test_skip
  #define SKIP                         CHAZ_TEST_SKIP
  #define Test_report_skip_remaining   chaz_Test_report_skip_remaining
  #define SKIP_REMAINING               CHAZ_TEST_SKIP_REMAINING
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_TEST */



