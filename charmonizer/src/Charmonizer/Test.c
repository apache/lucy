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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "Charmonizer/Test.h"

chaz_TestBatch*
chaz_Test_current = NULL;

static void
S_TestBatch_destroy(chaz_TestBatch *batch);

static void
S_TestBatch_run_test(chaz_TestBatch *batch);

#define CHAZ_TEST_PRINT_MESS(_pattern, _args) \
    va_start(_args, _pattern); \
    vprintf(_pattern, _args); \
    va_end(_args); \
    printf("\n");

void
chaz_Test_init(void) {
    /* Unbuffer stdout. */
    int check_val = setvbuf(stdout, NULL, _IONBF, 0);
    if (check_val != 0) {
        fprintf(stderr, "Failed when trying to unbuffer stdout\n");
    }
}

chaz_TestBatch*
chaz_Test_new_batch(const char *batch_name, unsigned num_tests,
                    chaz_TestBatch_test_func_t test_func) {
    chaz_TestBatch *batch = (chaz_TestBatch*)malloc(sizeof(chaz_TestBatch));
    if (!batch) {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }
    batch_name = batch_name ? batch_name : "";

    /* Assign. */
    batch->num_tests       = num_tests;
    batch->test_func       = test_func;
    batch->name            = (char*)malloc(strlen(batch_name) + 1);
    if (!batch->name) {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }
    strcpy(batch->name, batch_name);

    /* Initialize. */
    batch->test_num        = 0;
    batch->num_passed      = 0;
    batch->num_failed      = 0;
    batch->num_skipped     = 0;
    batch->destroy         = S_TestBatch_destroy;
    batch->run_test        = S_TestBatch_run_test;

    return batch;
}

chaz_TestBatch*
chaz_Test_start(unsigned num_tests) {
    if (chaz_Test_current) {
        fprintf(stderr, "Already started testing\n");
        return NULL;
    }
    chaz_Test_init();
    chaz_Test_current = chaz_Test_new_batch(NULL, num_tests, NULL);
    CHAZ_TEST_PLAN(chaz_Test_current);
    return chaz_Test_current;
}

int
chaz_Test_finish(void) {
    int remainder = chaz_Test_current->num_tests
                    - chaz_Test_current->num_passed
                    - chaz_Test_current->num_skipped;
    chaz_Test_current->destroy(chaz_Test_current);
    return !remainder;
}

void
chaz_Test_plan(chaz_TestBatch *batch) {
    printf("1..%u\n", batch->num_tests);
}

static void
S_TestBatch_destroy(chaz_TestBatch *batch) {
    free(batch->name);
    free(batch);
}

static void
S_TestBatch_run_test(chaz_TestBatch *batch) {
    /* Print start. */
    CHAZ_TEST_PLAN(batch);

    /* Run the batch. */
    batch->test_func(batch);
}

void
chaz_Test_test_true(chaz_TestBatch *batch, int value, const char *pat, ...) {
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

    CHAZ_TEST_PRINT_MESS(pat, args);
}

void
chaz_Test_test_false(chaz_TestBatch *batch, int value, const char *pat, ...) {
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

    CHAZ_TEST_PRINT_MESS(pat, args);
}

void
chaz_Test_test_str_eq(chaz_TestBatch *batch, const char *got, const char *expected,
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

    CHAZ_TEST_PRINT_MESS(pat, args);
}


void
chaz_Test_pass(chaz_TestBatch *batch, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Indicate pass, update pass counter. */
    printf("ok %u - ", batch->test_num);
    batch->num_passed++;

    CHAZ_TEST_PRINT_MESS(pat, args);
}

void
chaz_Test_fail(chaz_TestBatch *batch, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Indicate failure, update pass counter. */
    printf("not ok %u - ", batch->test_num);
    batch->num_failed++;

    CHAZ_TEST_PRINT_MESS(pat, args);
}

void
chaz_Test_test_int_eq(chaz_TestBatch *batch, long got, long expected,
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

    CHAZ_TEST_PRINT_MESS(pat, args);
}

void
chaz_Test_test_float_eq(chaz_TestBatch *batch, double got, double expected,
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

    CHAZ_TEST_PRINT_MESS(pat, args);
}

void
chaz_Test_skip(chaz_TestBatch *batch, const char *pat, ...) {
    va_list args;

    /* Increment test number. */
    batch->test_num++;

    /* Indicate that test is being skipped, update pass counter. */
    printf("ok %u # SKIP ", batch->test_num);
    batch->num_skipped++;

    CHAZ_TEST_PRINT_MESS(pat, args);
}

void
chaz_Test_report_skip_remaining(chaz_TestBatch *batch, const char *pat, ...) {
    va_list args;
    unsigned remaining = batch->num_tests - batch->test_num;

    /* Indicate that tests are being skipped, update skip counter. */
    printf("# Skipping all %u remaining tests: ", remaining);
    CHAZ_TEST_PRINT_MESS(pat, args);
    while (batch->test_num < batch->num_tests) {
        CHAZ_TEST_SKIP(batch, "");
    }
}


