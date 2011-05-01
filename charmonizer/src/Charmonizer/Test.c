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

#define CHAZ_USE_SHORT_NAMES

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "Charmonizer/Test.h"

static void
S_TestBatch_destroy(TestBatch *batch);

static void
S_TestBatch_run_test(TestBatch *batch);

#define PRINT_SUPPLIED_MESS(_pattern, _args) \
    va_start(_args, _pattern); \
    vprintf(_pattern, _args); \
    va_end(_args); \
    printf("\n");

void
Test_init(void) {
    /* Unbuffer stdout. */
    int check_val = setvbuf(stdout, NULL, _IONBF, 0);
    if (check_val != 0) {
        fprintf(stderr, "Failed when trying to unbuffer stdout\n");
    }
}

TestBatch*
Test_new_batch(const char *batch_name, unsigned num_tests,
               TestBatch_test_func_t test_func) {
    TestBatch *batch = (TestBatch*)malloc(sizeof(TestBatch));

    /* Assign. */
    batch->num_tests       = num_tests;
    batch->name            = strdup(batch_name);
    batch->test_func       = test_func;

    /* Initialize. */
    batch->test_num        = 0;
    batch->num_passed      = 0;
    batch->num_failed      = 0;
    batch->num_skipped     = 0;
    batch->destroy         = S_TestBatch_destroy;
    batch->run_test        = S_TestBatch_run_test;

    return batch;
}

void
Test_plan(TestBatch *batch) {
    printf("1..%u\n", batch->num_tests);
}

static void
S_TestBatch_destroy(TestBatch *batch) {
    free(batch->name);
    free(batch);
}

static void
S_TestBatch_run_test(TestBatch *batch) {
    /* Print start. */
    PLAN(batch);

    /* Run the batch. */
    batch->test_func(batch);
}

void
Test_test_true(TestBatch *batch, int value, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Test condition and pass or fail. */
    if (value) {
        printf("ok %u - ", batch->test_num);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - ", batch->test_num);
        batch->num_failed++;
    }

    PRINT_SUPPLIED_MESS(pat, args);
}

void
Test_test_false(TestBatch *batch, int value, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Test condition and pass or fail. */
    if (value == 0) {
        printf("ok %u - ", batch->test_num);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - ", batch->test_num);
        batch->num_failed++;
    }

    PRINT_SUPPLIED_MESS(pat, args);
}

void
Test_test_str_eq(TestBatch *batch, const char *got, const char *expected,
                 const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Test condition and pass or fail. */
    if (strcmp(expected, got) == 0) {
        printf("ok %u - ", batch->test_num);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - Expected '%s', got '%s'\n    ", batch->test_num,
               expected, got);
        batch->num_failed++;
    }

    PRINT_SUPPLIED_MESS(pat, args);
}


void
Test_pass(TestBatch *batch, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Indicate pass, update pass counter. */
    printf("ok %u - ", batch->test_num);
    batch->num_passed++;

    PRINT_SUPPLIED_MESS(pat, args);
}

void
Test_fail(TestBatch *batch, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Indicate failure, update pass counter. */
    printf("not ok %u - ", batch->test_num);
    batch->num_failed++;

    PRINT_SUPPLIED_MESS(pat, args);
}

void
Test_test_int_eq(TestBatch *batch, long got, long expected,
                 const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    if (expected == got) {
        printf("ok %u - ", batch->test_num);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - Expected '%ld', got '%ld'\n    ", batch->test_num,
               expected, got);
        batch->num_failed++;
    }

    PRINT_SUPPLIED_MESS(pat, args);
}

void
Test_test_float_eq(TestBatch *batch, double got, double expected,
                   const char *pat, ...) {
    va_list args;
    double diff = expected / got;

    /* Increment test number. */
    batch->test_num++;

    /* Evaluate condition and pass or fail. */
    if (diff > 0.00001) {
        printf("ok %u - ", batch->test_num);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - Expected '%f', got '%f'\n    ", batch->test_num,
               expected, got);
        batch->num_failed++;
    }

    PRINT_SUPPLIED_MESS(pat, args);
}

void
Test_skip(TestBatch *batch, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Indicate that test is being skipped, update pass counter. */
    printf("ok %u # SKIP ", batch->test_num);
    batch->num_skipped++;

    PRINT_SUPPLIED_MESS(pat, args);
}

void
Test_report_skip_remaining(TestBatch *batch, const char *pat, ...) {
    va_list args;
    unsigned remaining = batch->num_tests - batch->test_num;

    /* Indicate that tests are being skipped, update skip counter. */
    printf("# Skipping all %u remaining tests: ", remaining);
    PRINT_SUPPLIED_MESS(pat, args);
    while (batch->test_num < batch->num_tests) {
        SKIP(batch, "");
    }
}


