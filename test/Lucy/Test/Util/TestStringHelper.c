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

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#define TESTLUCY_USE_SHORT_NAMES

#include "Lucy/Test/Util/TestStringHelper.h"
#include "Lucy/Util/StringHelper.h"

#include "Clownfish/Class.h"
#include "Clownfish/String.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"

TestStringHelper*
TestStrHelp_new() {
    return (TestStringHelper*)Class_Make_Obj(TESTSTRINGHELPER);
}

static void
test_overlap(TestBatchRunner *runner) {
    size_t result;
    result = StrHelp_overlap("", "", 0, 0);
    TEST_UINT_EQ(runner, result, 0, "two empty strings");
    result = StrHelp_overlap("", "foo", 0, 3);
    TEST_UINT_EQ(runner, result, 0, "first string is empty");
    result = StrHelp_overlap("foo", "", 3, 0);
    TEST_UINT_EQ(runner, result, 0, "second string is empty");
    result = StrHelp_overlap("foo", "foo", 3, 3);
    TEST_UINT_EQ(runner, result, 3, "equal strings");
    result = StrHelp_overlap("foo bar", "foo", 7, 3);
    TEST_UINT_EQ(runner, result, 3, "first string is longer");
    result = StrHelp_overlap("foo", "foo bar", 3, 7);
    TEST_UINT_EQ(runner, result, 3, "second string is longer");
    result = StrHelp_overlap("bar", "baz", 3, 3);
    TEST_UINT_EQ(runner, result, 2, "different byte");
}


static void
test_to_base36(TestBatchRunner *runner) {
    char buffer[StrHelp_MAX_BASE36_BYTES];
    StrHelp_to_base36(UINT64_MAX, buffer);
    TEST_STR_EQ(runner, "3w5e11264sgsf", buffer, "base36 UINT64_MAX");
    StrHelp_to_base36(1, buffer);
    TEST_STR_EQ(runner, "1", buffer, "base36 1");
    TEST_INT_EQ(runner, buffer[1], 0, "base36 NULL termination");
}

static void
test_back_utf8_char(TestBatchRunner *runner) {
    char buffer[4];
    char *buf = buffer + 1;
    uint32_t len = Str_encode_utf8_char(0x263A, buffer);
    char *end = buffer + len;
    TEST_TRUE(runner, StrHelp_back_utf8_char(end, buffer) == buffer,
              "back_utf8_char");
    TEST_TRUE(runner, StrHelp_back_utf8_char(end, buf) == NULL,
              "back_utf8_char returns NULL rather than back up beyond start");
    TEST_TRUE(runner, StrHelp_back_utf8_char(buffer, buffer) == NULL,
              "back_utf8_char returns NULL when end == start");

    int32_t code_point;
    for (code_point = 0; code_point <= 0x10FFFF; code_point++) {
        uint32_t size = Str_encode_utf8_char(code_point, buffer);
        char *start = buffer;
        char *end   = start + size;

        if (StrHelp_back_utf8_char(end, start) != start) {
            break;
        }
    }
    if (code_point == 0x110000) {
        PASS(runner, "back_utf8_char works for code points 0 - 0x10FFFF");
    }
    else {
        FAIL(runner, "Failed back_utf8_char at 0x%.1X", (unsigned)code_point);
    }
}

void
TestStrHelp_Run_IMP(TestStringHelper *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 14);
    test_overlap(runner);
    test_to_base36(runner);
    test_back_utf8_char(runner);
}



