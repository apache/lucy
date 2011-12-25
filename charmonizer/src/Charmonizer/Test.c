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
#include <string.h>
#include "Charmonizer/Test.h"

struct chaz_TestBatch {
    unsigned test_num;
    unsigned num_tests;
    unsigned num_passed;
    unsigned num_failed;
    unsigned num_skipped;
};

chaz_TestBatch*
chaz_Test_current = NULL;

void
chaz_Test_init(void) {
    /* Unbuffer stdout. */
    int check_val = setvbuf(stdout, NULL, _IONBF, 0);
    if (check_val != 0) {
        fprintf(stderr, "Failed when trying to unbuffer stdout\n");
    }
}

chaz_TestBatch*
chaz_Test_new_batch(unsigned num_tests) {
    chaz_TestBatch *batch = (chaz_TestBatch*)malloc(sizeof(chaz_TestBatch));
    if (!batch) {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    /* Assign. */
    batch->num_tests       = num_tests;

    /* Initialize. */
    batch->test_num        = 0;
    batch->num_passed      = 0;
    batch->num_failed      = 0;
    batch->num_skipped     = 0;

    return batch;
}

chaz_TestBatch*
chaz_Test_start(unsigned num_tests) {
    if (chaz_Test_current) {
        fprintf(stderr, "Already started testing\n");
        return NULL;
    }
    chaz_Test_init();
    chaz_Test_current = chaz_Test_new_batch(num_tests);
    CHAZ_TEST_PLAN(chaz_Test_current);
    return chaz_Test_current;
}

int
chaz_Test_finish(void) {
    int remainder = chaz_Test_current->num_tests
                    - chaz_Test_current->num_passed
                    - chaz_Test_current->num_skipped;
    free(chaz_Test_current);
    chaz_Test_current = NULL;
    return !remainder;
}

void
chaz_Test_plan(chaz_TestBatch *batch) {
    printf("1..%u\n", batch->num_tests);
}

void
chaz_Test_ok(chaz_TestBatch *batch, int value, const char *message) {
    /* Increment test number. */
    batch->test_num++;

    /* Test condition and pass or fail. */
    if (value) {
        printf("ok %u - %s\n", batch->test_num, message);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - %s\n", batch->test_num, message);
        batch->num_failed++;
    }
}

void
chaz_Test_str_eq(chaz_TestBatch *batch, const char *got,
                 const char *expected, const char *message) {
    /* Increment test number. */
    batch->test_num++;

    /* Test condition and pass or fail. */
    if (strcmp(expected, got) == 0) {
        printf("ok %u - %s\n", batch->test_num, message);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - Expected '%s', got '%s'\n    # %s\n",
               batch->test_num, expected, got, message);
        batch->num_failed++;
    }
}


void
chaz_Test_pass(chaz_TestBatch *batch, const char *message) {
    /* Increment test number. */
    batch->test_num++;

    /* Indicate pass, update pass counter. */
    printf("ok %u - %s\n", batch->test_num, message);
    batch->num_passed++;
}

void
chaz_Test_fail(chaz_TestBatch *batch, const char *message) {
    /* Increment test number. */
    batch->test_num++;

    /* Indicate failure, update pass counter. */
    printf("not ok %u - %s\n", batch->test_num, message);
    batch->num_failed++;
}

void
chaz_Test_long_eq(chaz_TestBatch *batch, long got, long expected,
                  const char *message) {
    /* Increment test number. */
    batch->test_num++;

    if (expected == got) {
        printf("ok %u - %s\n", batch->test_num, message);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - Expected '%ld', got '%ld'\n    # %s",
               batch->test_num, expected, got, message);
        batch->num_failed++;
    }
}

void
chaz_Test_double_eq(chaz_TestBatch *batch, double got, double expected,
                    double slop, const char *message) {
    double diff = expected - got;
    if (diff < 0) {
        diff = 0 - diff;
    }

    /* Increment test number. */
    batch->test_num++;

    /* Evaluate condition and pass or fail. */
    if (diff < slop) {
        printf("ok %u - %s\n", batch->test_num, message);
        batch->num_passed++;
    }
    else {
        printf("not ok %u - Expected '%f', got '%f'\n    # %s\n",
               batch->test_num, expected, got, message);
        batch->num_failed++;
    }
}

void
chaz_Test_skip(chaz_TestBatch *batch, const char *message) {
    /* Increment test number. */
    batch->test_num++;

    /* Indicate that test is being skipped, update pass counter. */
    printf("ok %u # SKIP %s\n", batch->test_num, message);
    batch->num_skipped++;
}

void
chaz_Test_skip_remaining(chaz_TestBatch *batch, const char *message) {
    unsigned remaining = batch->num_tests - batch->test_num;

    /* Indicate that tests are being skipped, update skip counter. */
    printf("# Skipping all %u remaining tests: %s\n", remaining, message);
    while (batch->test_num < batch->num_tests) {
        chaz_Test_skip(batch, "");
    }
}


