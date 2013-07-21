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

#include "Clownfish/Test/TestCharBuf.h"

#include "Clownfish/CharBuf.h"
#include "Clownfish/Num.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Clownfish/VTable.h"

static char smiley[] = { (char)0xE2, (char)0x98, (char)0xBA, 0 };
static uint32_t smiley_len = 3;

TestCharBuf*
TestCB_new() {
    return (TestCharBuf*)VTable_Make_Obj(TESTCHARBUF);
}

static CharBuf*
S_get_cb(const char *string) {
    return CB_new_from_utf8(string, strlen(string));
}

static void
test_Cat(TestBatchRunner *runner) {
    CharBuf *wanted = CB_newf("a%s", smiley);
    CharBuf *got    = S_get_cb("");

    CB_Cat(got, wanted);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Cat");
    DECREF(got);

    got = S_get_cb("a");
    CB_Cat_Char(got, 0x263A);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Cat_Char");
    DECREF(got);

    got = S_get_cb("a");
    CB_Cat_Str(got, smiley, smiley_len);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Cat_Str");
    DECREF(got);

    got = S_get_cb("a");
    CB_Cat_Trusted_Str(got, smiley, smiley_len);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Cat_Trusted_Str");
    DECREF(got);

    DECREF(wanted);
}

static void
test_Mimic_and_Clone(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo");
    CharBuf *got    = S_get_cb("bar");

    CB_Mimic(got, (Obj*)wanted);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Mimic");
    DECREF(got);

    got = S_get_cb("bar");
    CB_Mimic_Str(got, "foo", 3);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Mimic_Str");
    DECREF(got);

    got = CB_Clone(wanted);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Clone");
    DECREF(got);

    DECREF(wanted);
}

static void
test_Find(TestBatchRunner *runner) {
    CharBuf *string = CB_new(10);
    CharBuf *substring = S_get_cb("foo");

    TEST_TRUE(runner, CB_Find(string, substring) == -1, "Not in empty string");
    CB_setf(string, "foo");
    TEST_TRUE(runner, CB_Find(string, substring) == 0, "Find complete string");
    CB_setf(string, "afoo");
    TEST_TRUE(runner, CB_Find(string, substring) == 1, "Find after first");
    CB_Set_Size(string, 3);
    TEST_TRUE(runner, CB_Find(string, substring) == -1, "Don't overrun");
    CB_setf(string, "afood");
    TEST_TRUE(runner, CB_Find(string, substring) == 1, "Find in middle");

    DECREF(substring);
    DECREF(string);
}

static void
test_Code_Point_At_and_From(TestBatchRunner *runner) {
    uint32_t code_points[] = { 'a', 0x263A, 0x263A, 'b', 0x263A, 'c' };
    uint32_t num_code_points = sizeof(code_points) / sizeof(uint32_t);
    CharBuf *string = CB_newf("a%s%sb%sc", smiley, smiley, smiley);
    uint32_t i;

    for (i = 0; i < num_code_points; i++) {
        uint32_t from = num_code_points - i - 1;
        TEST_INT_EQ(runner, CB_Code_Point_At(string, i), code_points[i],
                    "Code_Point_At %ld", (long)i);
        TEST_INT_EQ(runner, CB_Code_Point_At(string, from),
                    code_points[from], "Code_Point_From %ld", (long)from);
    }

    DECREF(string);
}

static void
test_SubString(TestBatchRunner *runner) {
    CharBuf *string = CB_newf("a%s%sb%sc", smiley, smiley, smiley);
    CharBuf *wanted = CB_newf("%sb%s", smiley, smiley);
    CharBuf *got = CB_SubString(string, 2, 3);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "SubString");
    DECREF(wanted);
    DECREF(got);
    DECREF(string);
}

static void
test_Nip_and_Chop(TestBatchRunner *runner) {
    CharBuf *wanted;
    CharBuf *got;

    wanted = CB_newf("%sb%sc", smiley, smiley);
    got    = CB_newf("a%s%sb%sc", smiley, smiley, smiley);
    CB_Nip(got, 2);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Nip");
    DECREF(wanted);
    DECREF(got);

    wanted = CB_newf("a%s%s", smiley, smiley);
    got    = CB_newf("a%s%sb%sc", smiley, smiley, smiley);
    CB_Chop(got, 3);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Chop");
    DECREF(wanted);
    DECREF(got);
}


static void
test_Truncate(TestBatchRunner *runner) {
    CharBuf *wanted = CB_newf("a%s", smiley, smiley);
    CharBuf *got    = CB_newf("a%s%sb%sc", smiley, smiley, smiley);
    CB_Truncate(got, 2);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "Truncate");
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
    CharBuf *got = CB_new(0);

    // Surround a smiley with lots of whitespace.
    for (i = 0; i < num_spaces; i++) { CB_Cat_Char(got, spaces[i]); }
    CB_Cat_Char(got, 0x263A);
    for (i = 0; i < num_spaces; i++) { CB_Cat_Char(got, spaces[i]); }

    TEST_TRUE(runner, CB_Trim_Top(got), "Trim_Top returns true on success");
    TEST_FALSE(runner, CB_Trim_Top(got),
               "Trim_Top returns false on failure");
    TEST_TRUE(runner, CB_Trim_Tail(got), "Trim_Tail returns true on success");
    TEST_FALSE(runner, CB_Trim_Tail(got),
               "Trim_Tail returns false on failure");
    TEST_TRUE(runner, CB_Equals_Str(got, smiley, smiley_len),
              "Trim_Top and Trim_Tail worked");

    // Build the spacey smiley again.
    CB_Truncate(got, 0);
    for (i = 0; i < num_spaces; i++) { CB_Cat_Char(got, spaces[i]); }
    CB_Cat_Char(got, 0x263A);
    for (i = 0; i < num_spaces; i++) { CB_Cat_Char(got, spaces[i]); }

    TEST_TRUE(runner, CB_Trim(got), "Trim returns true on success");
    TEST_FALSE(runner, CB_Trim(got), "Trim returns false on failure");
    TEST_TRUE(runner, CB_Equals_Str(got, smiley, smiley_len),
              "Trim worked");

    DECREF(got);
}

static void
test_To_F64(TestBatchRunner *runner) {
    CharBuf *charbuf = S_get_cb("1.5");
    double difference = 1.5 - CB_To_F64(charbuf);
    if (difference < 0) { difference = 0 - difference; }
    TEST_TRUE(runner, difference < 0.001, "To_F64");

    CB_setf(charbuf, "-1.5");
    difference = 1.5 + CB_To_F64(charbuf);
    if (difference < 0) { difference = 0 - difference; }
    TEST_TRUE(runner, difference < 0.001, "To_F64 negative");

    CB_setf(charbuf, "1.59");
    double value_full = CB_To_F64(charbuf);
    CB_Set_Size(charbuf, 3);
    double value_short = CB_To_F64(charbuf);
    TEST_TRUE(runner, value_short < value_full,
              "TO_F64 doesn't run past end of string");

    DECREF(charbuf);
}

static void
test_To_I64(TestBatchRunner *runner) {
    CharBuf *charbuf = S_get_cb("10");
    TEST_TRUE(runner, CB_To_I64(charbuf) == 10, "To_I64");
    CB_setf(charbuf, "-10");
    TEST_TRUE(runner, CB_To_I64(charbuf) == -10, "To_I64 negative");
    DECREF(charbuf);
}


static void
test_vcatf_s(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar bizzle baz");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %s baz", "bizzle");
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%s");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_null_string(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar [NULL] baz");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %s baz", NULL);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%s NULL");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_cb(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar ZEKE baz");
    CharBuf *catworthy = S_get_cb("ZEKE");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %o baz", catworthy);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%o CharBuf");
    DECREF(catworthy);
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_obj(TestBatchRunner *runner) {
    CharBuf   *wanted = S_get_cb("ooga 20 booga");
    Integer32 *i32 = Int32_new(20);
    CharBuf   *got = S_get_cb("ooga");
    CB_catf(got, " %o booga", i32);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%o Obj");
    DECREF(i32);
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_null_obj(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar [NULL] baz");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %o baz", NULL);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%o NULL");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_i8(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar -3 baz");
    int8_t num = -3;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %i8 baz", num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%i8");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_i32(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar -100000 baz");
    int32_t num = -100000;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %i32 baz", num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%i32");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_i64(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar -5000000000 baz");
    int64_t num = INT64_C(-5000000000);
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %i64 baz", num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%i64");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_u8(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar 3 baz");
    uint8_t num = 3;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %u8 baz", num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%u8");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_u32(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar 100000 baz");
    uint32_t num = 100000;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %u32 baz", num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%u32");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_u64(TestBatchRunner *runner) {
    CharBuf *wanted = S_get_cb("foo bar 5000000000 baz");
    uint64_t num = UINT64_C(5000000000);
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %u64 baz", num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%u64");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_f64(TestBatchRunner *runner) {
    CharBuf *wanted;
    char buf[64];
    float num = 1.3f;
    CharBuf *got = S_get_cb("foo ");
    sprintf(buf, "foo bar %g baz", num);
    wanted = CB_new_from_trusted_utf8(buf, strlen(buf));
    CB_catf(got, "bar %f64 baz", num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%f64");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_x32(TestBatchRunner *runner) {
    CharBuf *wanted;
    char buf[64];
    unsigned long num = INT32_MAX;
    CharBuf *got = S_get_cb("foo ");
#if (SIZEOF_LONG == 4)
    sprintf(buf, "foo bar %.8lx baz", num);
#elif (SIZEOF_INT == 4)
    sprintf(buf, "foo bar %.8x baz", (unsigned)num);
#endif
    wanted = CB_new_from_trusted_utf8(buf, strlen(buf));
    CB_catf(got, "bar %x32 baz", (uint32_t)num);
    TEST_TRUE(runner, CB_Equals(wanted, (Obj*)got), "%%x32");
    DECREF(wanted);
    DECREF(got);
}

void
TestCB_run(TestCharBuf *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 54);
    test_vcatf_s(runner);
    test_vcatf_null_string(runner);
    test_vcatf_cb(runner);
    test_vcatf_obj(runner);
    test_vcatf_null_obj(runner);
    test_vcatf_i8(runner);
    test_vcatf_i32(runner);
    test_vcatf_i64(runner);
    test_vcatf_u8(runner);
    test_vcatf_u32(runner);
    test_vcatf_u64(runner);
    test_vcatf_f64(runner);
    test_vcatf_x32(runner);
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


