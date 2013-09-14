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
#include "Clownfish/String.h"
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

static String*
S_get_str(const char *string) {
    return Str_new_from_utf8(string, strlen(string));
}

static bool
S_cb_equals(CharBuf *cb, String *other) {
    String *string = CB_To_String(cb);
    bool retval = Str_Equals(string, (Obj*)other);
    DECREF(string);
    return retval;
}

static void
test_Cat(TestBatchRunner *runner) {
    String  *wanted = Str_newf("a%s", smiley);
    CharBuf *got    = S_get_cb("");

    CB_Cat(got, wanted);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Cat");
    DECREF(got);

    got = S_get_cb("a");
    CB_Cat_Char(got, 0x263A);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Cat_Char");
    DECREF(got);

    got = S_get_cb("a");
    CB_Cat_Utf8(got, smiley, smiley_len);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Cat_Utf8");
    DECREF(got);

    got = S_get_cb("a");
    CB_Cat_Trusted_Utf8(got, smiley, smiley_len);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Cat_Trusted_Utf8");
    DECREF(got);

    DECREF(wanted);
}

static void
test_Mimic_and_Clone(TestBatchRunner *runner) {
    String  *wanted    = S_get_str("foo");
    CharBuf *wanted_cb = S_get_cb("foo");
    CharBuf *got       = S_get_cb("bar");

    CB_Mimic(got, (Obj*)wanted);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Mimic String");
    DECREF(got);

    got = S_get_cb("bar");
    CB_Mimic(got, (Obj*)wanted_cb);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Mimic CharBuf");
    DECREF(got);

    got = S_get_cb("bar");
    CB_Mimic_Utf8(got, "foo", 3);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Mimic_Utf8");
    DECREF(got);

    got = CB_Clone(wanted_cb);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Clone");
    DECREF(got);

    DECREF(wanted);
}

/*
static void
test_Truncate(TestBatchRunner *runner) {
    String  *wanted = Str_newf("a%s", smiley);
    CharBuf *got    = CB_newf("a%s%sb%sc", smiley, smiley, smiley);
    CB_Truncate(got, 2);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "Truncate");
    DECREF(wanted);
    DECREF(got);
}
*/

static void
test_vcatf_s(TestBatchRunner *runner) {
    String  *wanted = S_get_str("foo bar bizzle baz");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %s baz", "bizzle");
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%s");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_null_string(TestBatchRunner *runner) {
    String  *wanted = S_get_str("foo bar [NULL] baz");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %s baz", NULL);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%s NULL");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_str(TestBatchRunner *runner) {
    String  *wanted = S_get_str("foo bar ZEKE baz");
    String  *catworthy = S_get_str("ZEKE");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %o baz", catworthy);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%o CharBuf");
    DECREF(catworthy);
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_obj(TestBatchRunner *runner) {
    String    *wanted = S_get_str("ooga 20 booga");
    Integer32 *i32 = Int32_new(20);
    CharBuf   *got = S_get_cb("ooga");
    CB_catf(got, " %o booga", i32);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%o Obj");
    DECREF(i32);
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_null_obj(TestBatchRunner *runner) {
    String  *wanted = S_get_str("foo bar [NULL] baz");
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %o baz", NULL);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%o NULL");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_i8(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo bar -3 baz");
    int8_t num = -3;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %i8 baz", num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%i8");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_i32(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo bar -100000 baz");
    int32_t num = -100000;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %i32 baz", num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%i32");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_i64(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo bar -5000000000 baz");
    int64_t num = INT64_C(-5000000000);
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %i64 baz", num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%i64");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_u8(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo bar 3 baz");
    uint8_t num = 3;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %u8 baz", num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%u8");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_u32(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo bar 100000 baz");
    uint32_t num = 100000;
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %u32 baz", num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%u32");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_u64(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo bar 5000000000 baz");
    uint64_t num = UINT64_C(5000000000);
    CharBuf *got = S_get_cb("foo ");
    CB_catf(got, "bar %u64 baz", num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%u64");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_f64(TestBatchRunner *runner) {
    String *wanted;
    char buf[64];
    float num = 1.3f;
    CharBuf *got = S_get_cb("foo ");
    sprintf(buf, "foo bar %g baz", num);
    wanted = Str_new_from_trusted_utf8(buf, strlen(buf));
    CB_catf(got, "bar %f64 baz", num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%f64");
    DECREF(wanted);
    DECREF(got);
}

static void
test_vcatf_x32(TestBatchRunner *runner) {
    String *wanted;
    char buf[64];
    unsigned long num = INT32_MAX;
    CharBuf *got = S_get_cb("foo ");
#if (SIZEOF_LONG == 4)
    sprintf(buf, "foo bar %.8lx baz", num);
#elif (SIZEOF_INT == 4)
    sprintf(buf, "foo bar %.8x baz", (unsigned)num);
#endif
    wanted = Str_new_from_trusted_utf8(buf, strlen(buf));
    CB_catf(got, "bar %x32 baz", (uint32_t)num);
    TEST_TRUE(runner, S_cb_equals(got, wanted), "%%x32");
    DECREF(wanted);
    DECREF(got);
}

void
TestCB_Run_IMP(TestCharBuf *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 21);
    test_vcatf_s(runner);
    test_vcatf_null_string(runner);
    test_vcatf_str(runner);
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
}


