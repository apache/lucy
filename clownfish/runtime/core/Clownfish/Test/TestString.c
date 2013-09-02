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

#include <string.h>
#include <stdio.h>

#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define TESTCFISH_USE_SHORT_NAMES

#include "charmony.h"

#include "Clownfish/Test/TestString.h"

#include "Clownfish/String.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Num.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Clownfish/VTable.h"

static char smiley[] = { (char)0xE2, (char)0x98, (char)0xBA, 0 };
static uint32_t smiley_len = 3;

TestString*
TestStr_new() {
    return (TestString*)VTable_Make_Obj(TESTSTRING);
}

static String*
S_get_str(const char *string) {
    return Str_new_from_utf8(string, strlen(string));
}

static void
test_Cat(TestBatchRunner *runner) {
    String *wanted = Str_newf("a%s", smiley);
    String *source;
    String *got;

    source = S_get_str("");
    got = Str_Immutable_Cat(source, wanted);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Cat");
    DECREF(got);
    DECREF(source);

    source = S_get_str("a");
    got = Str_Immutable_Cat_UTF8(source, smiley, smiley_len);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Cat_UTF8");
    DECREF(got);
    DECREF(source);

    source = S_get_str("a");
    got = Str_Immutable_Cat_Trusted_UTF8(source, smiley, smiley_len);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Cat_Trusted_UTF8");
    DECREF(got);
    DECREF(source);

    DECREF(wanted);
}

static void
test_Mimic_and_Clone(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo");
    String *got    = S_get_str("bar");

    Str_Mimic(got, (Obj*)wanted);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Mimic");
    DECREF(got);

    got = S_get_str("bar");
    Str_Mimic_Str(got, "foo", 3);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Mimic_Str");
    DECREF(got);

    got = Str_Clone(wanted);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Clone");
    DECREF(got);

    DECREF(wanted);
}

static void
test_Find(TestBatchRunner *runner) {
    String *string;
    String *substring = S_get_str("foo");

    string = S_get_str("");
    TEST_TRUE(runner, Str_Find(string, substring) == -1, "Not in empty string");
    DECREF(string);

    string = S_get_str("foo");
    TEST_TRUE(runner, Str_Find(string, substring) == 0, "Find complete string");
    DECREF(string);

    string = S_get_str("afoo");
    TEST_TRUE(runner, Str_Find(string, substring) == 1, "Find after first");
    Str_Set_Size(string, 3);
    TEST_TRUE(runner, Str_Find(string, substring) == -1, "Don't overrun");
    DECREF(string);

    string = S_get_str("afood");
    TEST_TRUE(runner, Str_Find(string, substring) == 1, "Find in middle");
    DECREF(string);

    DECREF(substring);
}

static void
test_Code_Point_At_and_From(TestBatchRunner *runner) {
    uint32_t code_points[] = { 'a', 0x263A, 0x263A, 'b', 0x263A, 'c' };
    uint32_t num_code_points = sizeof(code_points) / sizeof(uint32_t);
    String *string = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    uint32_t i;

    for (i = 0; i < num_code_points; i++) {
        uint32_t from = num_code_points - i - 1;
        TEST_INT_EQ(runner, Str_Code_Point_At(string, i), code_points[i],
                    "Code_Point_At %ld", (long)i);
        TEST_INT_EQ(runner, Str_Code_Point_At(string, from),
                    code_points[from], "Code_Point_From %ld", (long)from);
    }

    DECREF(string);
}

static void
test_SubString(TestBatchRunner *runner) {
    String *string = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    String *wanted = Str_newf("%sb%s", smiley, smiley);
    String *got = Str_SubString(string, 2, 3);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "SubString");
    DECREF(wanted);
    DECREF(got);
    DECREF(string);
}

static void
test_Nip_and_Chop(TestBatchRunner *runner) {
    String *wanted;
    String *string;
    StackString *got;

    wanted = Str_newf("%sb%sc", smiley, smiley);
    string = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    got    = SSTR_WRAP(string);
    SStr_Nip(got, 2);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Nip");
    DECREF(wanted);
    DECREF(string);

    wanted = Str_newf("a%s%s", smiley, smiley);
    string = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    got    = SSTR_WRAP(string);
    SStr_Chop(got, 3);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Chop");
    DECREF(wanted);
    DECREF(string);
}


static void
test_Truncate(TestBatchRunner *runner) {
    String *wanted = Str_newf("a%s", smiley, smiley);
    String *got    = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    Str_Truncate(got, 2);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Truncate");
    DECREF(wanted);
    DECREF(got);
}

static void
test_Trim(TestBatchRunner *runner) {
    uint32_t spaces[] = {
        ' ',    '\t',   '\r',   '\n',   0x000B, 0x000C, 0x000D, 0x0085,
        0x00A0, 0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004,
        0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028, 0x2029,
        0x202F, 0x205F, 0x3000
    };
    uint32_t num_spaces = sizeof(spaces) / sizeof(uint32_t);
    uint32_t i;
    String *got;

    // Surround a smiley with lots of whitespace.
    CharBuf *buf = CB_new(0);
    for (i = 0; i < num_spaces; i++) { CB_Cat_Char(buf, spaces[i]); }
    CB_Cat_Char(buf, 0x263A);
    for (i = 0; i < num_spaces; i++) { CB_Cat_Char(buf, spaces[i]); }

    got = CB_To_String(buf);
    TEST_TRUE(runner, Str_Trim_Top(got), "Trim_Top returns true on success");
    TEST_FALSE(runner, Str_Trim_Top(got),
               "Trim_Top returns false on failure");
    TEST_TRUE(runner, Str_Trim_Tail(got), "Trim_Tail returns true on success");
    TEST_FALSE(runner, Str_Trim_Tail(got),
               "Trim_Tail returns false on failure");
    TEST_TRUE(runner, Str_Equals_Str(got, smiley, smiley_len),
              "Trim_Top and Trim_Tail worked");
    DECREF(got);

    got = CB_To_String(buf);
    TEST_TRUE(runner, Str_Trim(got), "Trim returns true on success");
    TEST_FALSE(runner, Str_Trim(got), "Trim returns false on failure");
    TEST_TRUE(runner, Str_Equals_Str(got, smiley, smiley_len),
              "Trim worked");
    DECREF(got);

    DECREF(buf);
}

static void
test_To_F64(TestBatchRunner *runner) {
    String *string;

    string = S_get_str("1.5");
    double difference = 1.5 - Str_To_F64(string);
    if (difference < 0) { difference = 0 - difference; }
    TEST_TRUE(runner, difference < 0.001, "To_F64");
    DECREF(string);

    string = S_get_str("-1.5");
    difference = 1.5 + Str_To_F64(string);
    if (difference < 0) { difference = 0 - difference; }
    TEST_TRUE(runner, difference < 0.001, "To_F64 negative");
    DECREF(string);

    string = S_get_str("1.59");
    double value_full = Str_To_F64(string);
    Str_Set_Size(string, 3);
    double value_short = Str_To_F64(string);
    TEST_TRUE(runner, value_short < value_full,
              "TO_F64 doesn't run past end of string");
    DECREF(string);
}

static void
test_To_I64(TestBatchRunner *runner) {
    String *string;

    string = S_get_str("10");
    TEST_TRUE(runner, Str_To_I64(string) == 10, "To_I64");
    DECREF(string);

    string = S_get_str("-10");
    TEST_TRUE(runner, Str_To_I64(string) == -10, "To_I64 negative");
    DECREF(string);
}


void
TestStr_Run_IMP(TestString *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 40);
    test_Cat(runner);
    test_Mimic_and_Clone(runner);
    test_Code_Point_At_and_From(runner);
    test_Find(runner);
    test_SubString(runner);
    test_Nip_and_Chop(runner);
    test_Truncate(runner);
    test_Trim(runner);
    test_To_F64(runner);
    test_To_I64(runner);
}


