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
#include "Clownfish/Util/Memory.h"
#include "Clownfish/VTable.h"

#define SMILEY "\xE2\x98\xBA"
static char smiley[] = { (char)0xE2, (char)0x98, (char)0xBA, 0 };
static uint32_t smiley_len = 3;
static uint32_t smiley_cp  = 0x263A;

TestString*
TestStr_new() {
    return (TestString*)VTable_Make_Obj(TESTSTRING);
}

static String*
S_get_str(const char *string) {
    return Str_new_from_utf8(string, strlen(string));
}

// Surround a smiley with lots of whitespace.
static String*
S_smiley_with_whitespace(int *num_spaces_ptr) {
    uint32_t spaces[] = {
        ' ',    '\t',   '\r',   '\n',   0x000B, 0x000C, 0x000D, 0x0085,
        0x00A0, 0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004,
        0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028, 0x2029,
        0x202F, 0x205F, 0x3000
    };
    int num_spaces = sizeof(spaces) / sizeof(uint32_t);
    String *got;

    CharBuf *buf = CB_new(0);
    for (int i = 0; i < num_spaces; i++) { CB_Cat_Char(buf, spaces[i]); }
    CB_Cat_Char(buf, smiley_cp);
    for (int i = 0; i < num_spaces; i++) { CB_Cat_Char(buf, spaces[i]); }

    String *retval = CB_To_String(buf);
    if (num_spaces_ptr) { *num_spaces_ptr = num_spaces; }

    DECREF(buf);
    return retval;
}

static void
test_Cat(TestBatchRunner *runner) {
    String *wanted = Str_newf("a%s", smiley);
    String *source;
    String *got;

    source = S_get_str("");
    got = Str_Cat(source, wanted);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Cat");
    DECREF(got);
    DECREF(source);

    source = S_get_str("a");
    got = Str_Cat_Utf8(source, smiley, smiley_len);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Cat_Utf8");
    DECREF(got);
    DECREF(source);

    source = S_get_str("a");
    got = Str_Cat_Trusted_Utf8(source, smiley, smiley_len);
    TEST_TRUE(runner, Str_Equals(wanted, (Obj*)got), "Cat_Trusted_Utf8");
    DECREF(got);
    DECREF(source);

    DECREF(wanted);
}

static void
test_Clone(TestBatchRunner *runner) {
    String *wanted = S_get_str("foo");
    String *got    = S_get_str("bar");

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
    // TODO: Enable this test when we have real substrings.
    /*Str_Set_Size(string, 3);
    TEST_TRUE(runner, Str_Find(string, substring) == -1, "Don't overrun");*/
    DECREF(string);

    string = S_get_str("afood");
    TEST_TRUE(runner, Str_Find(string, substring) == 1, "Find in middle");
    DECREF(string);

    DECREF(substring);
}

static void
test_Code_Point_At_and_From(TestBatchRunner *runner) {
    uint32_t code_points[] = {
        'a', smiley_cp, smiley_cp, 'b', smiley_cp, 'c'
    };
    uint32_t num_code_points = sizeof(code_points) / sizeof(uint32_t);
    String *string = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    uint32_t i;

    for (i = 0; i < num_code_points; i++) {
        uint32_t from = num_code_points - i;
        TEST_INT_EQ(runner, Str_Code_Point_At(string, i), code_points[i],
                    "Code_Point_At %ld", (long)i);
        TEST_INT_EQ(runner, Str_Code_Point_From(string, from),
                    code_points[i], "Code_Point_From %ld", (long)from);
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
test_Trim(TestBatchRunner *runner) {
    String *ws_smiley = S_smiley_with_whitespace(NULL);
    String *ws_foo    = S_get_str("  foo  ");
    String *ws_only   = S_get_str("  \t  \r\n");
    String *trimmed   = S_get_str("a     b");
    String *got;

    got = Str_Trim(ws_smiley);
    TEST_TRUE(runner, Str_Equals_Utf8(got, smiley, smiley_len), "Trim");
    DECREF(got);

    got = Str_Trim_Top(ws_foo);
    TEST_TRUE(runner, Str_Equals_Utf8(got, "foo  ", 5), "Trim_Top");
    DECREF(got);

    got = Str_Trim_Tail(ws_foo);
    TEST_TRUE(runner, Str_Equals_Utf8(got, "  foo", 5), "Trim_Tail");
    DECREF(got);

    got = Str_Trim(ws_only);
    TEST_TRUE(runner, Str_Equals_Utf8(got, "", 0), "Trim with only whitespace");
    DECREF(got);

    got = Str_Trim_Top(ws_only);
    TEST_TRUE(runner, Str_Equals_Utf8(got, "", 0),
              "Trim_Top with only whitespace");
    DECREF(got);

    got = Str_Trim_Tail(ws_only);
    TEST_TRUE(runner, Str_Equals_Utf8(got, "", 0),
              "Trim_Tail with only whitespace");
    DECREF(got);

    got = Str_Trim(trimmed);
    TEST_TRUE(runner, Str_Equals(got, (Obj*)trimmed),
              "Trim doesn't change trimmed string");
    DECREF(got);

    got = Str_Trim_Top(trimmed);
    TEST_TRUE(runner, Str_Equals(got, (Obj*)trimmed),
              "Trim_Top doesn't change trimmed string");
    DECREF(got);

    got = Str_Trim_Tail(trimmed);
    TEST_TRUE(runner, Str_Equals(got, (Obj*)trimmed),
              "Trim_Tail doesn't change trimmed string");
    DECREF(got);

    DECREF(trimmed);
    DECREF(ws_only);
    DECREF(ws_foo);
    DECREF(ws_smiley);
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

    // TODO: Enable this test when we have real substrings.
    /*string = S_get_str("1.59");
    double value_full = Str_To_F64(string);
    Str_Set_Size(string, 3);
    double value_short = Str_To_F64(string);
    TEST_TRUE(runner, value_short < value_full,
              "TO_F64 doesn't run past end of string");
    DECREF(string);*/
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

static void
test_To_Utf8(TestBatchRunner *runner) {
    String *string = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    char *buf = Str_To_Utf8(string);
    TEST_TRUE(runner, strcmp(buf, "a" SMILEY SMILEY "b" SMILEY "c") == 0,
              "To_Utf8");
    FREEMEM(buf);
    DECREF(string);
}

static void
test_Length(TestBatchRunner *runner) {
    String *string = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    TEST_INT_EQ(runner, Str_Length(string), 6, "Length");
    DECREF(string);
}

static void
test_Compare_To(TestBatchRunner *runner) {
    String *abc = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    String *ab  = Str_newf("a%s%sb", smiley, smiley);
    String *ac  = Str_newf("a%s%sc", smiley, smiley);

    TEST_TRUE(runner, Str_Compare_To(abc, (Obj*)abc) == 0,
              "Compare_To abc abc");
    TEST_TRUE(runner, Str_Compare_To(ab, (Obj*)abc) < 0,
              "Compare_To ab abc");
    TEST_TRUE(runner, Str_Compare_To(abc, (Obj*)ab) > 0,
              "Compare_To abc ab");
    TEST_TRUE(runner, Str_Compare_To(ab, (Obj*)ac) < 0,
              "Compare_To ab ac");
    TEST_TRUE(runner, Str_Compare_To(ac, (Obj*)ab) > 0,
              "Compare_To ac ab");

    DECREF(ac);
    DECREF(ab);
    DECREF(abc);
}

static void
test_Swap_Chars(TestBatchRunner *runner) {
    String *source = S_get_str("aXXbXc");
    String *got    = Str_Swap_Chars(source, 'X', smiley_cp);
    String *wanted = Str_newf("a%s%sb%sc", smiley, smiley, smiley);
    TEST_TRUE(runner, Str_Equals(got, (Obj*)wanted), "Swap_Chars");
    DECREF(wanted);
    DECREF(got);
    DECREF(source);
}

static void
test_iterator(TestBatchRunner *runner) {
    static const uint32_t code_points[] = {
        0x41,
        0x7F,
        0x80,
        0x7FF,
        0x800,
        0xFFFF,
        0x10000,
        0x10FFFF
    };
    static size_t num_code_points
        = sizeof(code_points) / sizeof(code_points[0]);

    CharBuf *buf = CB_new(0);
    for (int i = 0; i < num_code_points; ++i) {
        CB_Cat_Char(buf, code_points[i]);
    }
    String *string = CB_To_String(buf);

    {
        StringIterator *top  = Str_Top(string);
        StringIterator *tail = Str_Tail(string);

        TEST_INT_EQ(runner, StrIter_Compare_To(top, (Obj*)tail), -1,
                    "Compare_To top < tail");
        TEST_INT_EQ(runner, StrIter_Compare_To(tail, (Obj*)top), 1,
                    "Compare_To tail > top");
        TEST_INT_EQ(runner, StrIter_Compare_To(top, (Obj*)top), 0,
                    "Compare_To top == top");

        StringIterator *clone = StrIter_Clone(top);
        TEST_TRUE(runner, StrIter_Equals(clone, (Obj*)top), "Clone");

        StrIter_Assign(clone, tail);
        TEST_TRUE(runner, StrIter_Equals(clone, (Obj*)tail), "Assign");

        DECREF(clone);
        DECREF(top);
        DECREF(tail);
    }

    {
        StringIterator *iter = Str_Top(string);

        for (int i = 0; i < num_code_points; ++i) {
            TEST_TRUE(runner, StrIter_Has_Next(iter), "Has_Next %d", i);
            int32_t code_point = StrIter_Next(iter);
            TEST_INT_EQ(runner, code_point, code_points[i], "Next %d", i);
        }

        TEST_TRUE(runner, !StrIter_Has_Next(iter),
                  "Has_Next at end of string");
        TEST_INT_EQ(runner, StrIter_Next(iter), STRITER_DONE,
                    "Next at end of string");

        StringIterator *tail = Str_Tail(string);
        TEST_TRUE(runner, StrIter_Equals(iter, (Obj*)tail), "Equals tail");

        DECREF(tail);
        DECREF(iter);
    }

    {
        StringIterator *iter = Str_Tail(string);

        for (int i = num_code_points - 1; i >= 0; --i) {
            TEST_TRUE(runner, StrIter_Has_Prev(iter), "Has_Prev %d", i);
            int32_t code_point = StrIter_Prev(iter);
            TEST_INT_EQ(runner, code_point, code_points[i], "Prev %d", i);
        }

        TEST_TRUE(runner, !StrIter_Has_Prev(iter),
                  "Has_Prev at end of string");
        TEST_INT_EQ(runner, StrIter_Prev(iter), STRITER_DONE,
                    "Prev at start of string");

        StringIterator *top = Str_Top(string);
        TEST_TRUE(runner, StrIter_Equals(iter, (Obj*)top), "Equals top");

        DECREF(top);
        DECREF(iter);
    }

    {
        StringIterator *iter = Str_Top(string);

        StrIter_Next(iter);
        TEST_INT_EQ(runner, StrIter_Advance(iter, 2), 2,
                    "Advance returns number of code points");
        TEST_INT_EQ(runner, StrIter_Next(iter), code_points[3],
                    "Advance works");
        TEST_INT_EQ(runner,
                    StrIter_Advance(iter, 1000000), num_code_points - 4,
                    "Advance past end of string");

        StrIter_Prev(iter);
        TEST_INT_EQ(runner, StrIter_Recede(iter, 2), 2,
                    "Recede returns number of code points");
        TEST_INT_EQ(runner, StrIter_Prev(iter), code_points[num_code_points-4],
                    "Recede works");
        TEST_INT_EQ(runner, StrIter_Recede(iter, 1000000), num_code_points - 4,
                    "Recede past start of string");

        DECREF(iter);
    }

    DECREF(string);
    DECREF(buf);
}

static void
test_iterator_whitespace(TestBatchRunner *runner) {
    int num_spaces;
    String *ws_smiley = S_smiley_with_whitespace(&num_spaces);

    {
        StringIterator *iter = Str_Top(ws_smiley);
        TEST_INT_EQ(runner, StrIter_Skip_Next_Whitespace(iter), num_spaces,
                    "Skip_Next_Whitespace");
        TEST_INT_EQ(runner, StrIter_Skip_Next_Whitespace(iter), 0,
                    "Skip_Next_Whitespace without whitespace");
        DECREF(iter);
    }

    {
        StringIterator *iter = Str_Tail(ws_smiley);
        TEST_INT_EQ(runner, StrIter_Skip_Prev_Whitespace(iter), num_spaces,
                    "Skip_Prev_Whitespace");
        TEST_INT_EQ(runner, StrIter_Skip_Prev_Whitespace(iter), 0,
                    "Skip_Prev_Whitespace without whitespace");
        DECREF(iter);
    }

    DECREF(ws_smiley);
}

static void
test_iterator_substring(TestBatchRunner *runner) {
    String *string = Str_newf("a%sb%sc%sd", smiley, smiley, smiley);

    StringIterator *start = Str_Top(string);
    StringIterator *end = Str_Tail(string);

    {
        String *substring = StrIter_substring(start, end);
        TEST_TRUE(runner, Str_Equals(substring, (Obj*)string),
                  "StrIter_substring whole string");
        DECREF(substring);
    }

    StrIter_Advance(start, 2);
    StrIter_Recede(end, 2);

    {
        String *substring = StrIter_substring(start, end);
        String *wanted = Str_newf("b%sc", smiley);
        TEST_TRUE(runner, Str_Equals(substring, (Obj*)wanted),
                  "StrIter_substring");

        TEST_TRUE(runner, StrIter_Starts_With(start, wanted), "Starts_With");
        TEST_TRUE(runner, StrIter_Ends_With(end, wanted), "Ends_With");

        DECREF(wanted);
        DECREF(substring);
    }

    {
        String *substring = StrIter_substring(end, NULL);
        String *wanted = Str_newf("%sd", smiley);
        TEST_TRUE(runner, Str_Equals(substring, (Obj*)wanted),
                  "StrIter_substring with NULL tail");
        DECREF(wanted);
        DECREF(substring);
    }

    {
        String *substring = StrIter_substring(NULL, start);
        String *wanted = Str_newf("a%s", smiley);
        TEST_TRUE(runner, Str_Equals(substring, (Obj*)wanted),
                  "StrIter_substring with NULL top");
        DECREF(wanted);
        DECREF(substring);
    }

    DECREF(start);
    DECREF(end);
    DECREF(string);
}

void
TestStr_Run_IMP(TestString *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 101);
    test_Cat(runner);
    test_Clone(runner);
    test_Code_Point_At_and_From(runner);
    test_Find(runner);
    test_SubString(runner);
    test_Trim(runner);
    test_To_F64(runner);
    test_To_I64(runner);
    test_To_Utf8(runner);
    test_Length(runner);
    test_Compare_To(runner);
    test_Swap_Chars(runner);
    test_iterator(runner);
    test_iterator_whitespace(runner);
    test_iterator_substring(runner);
}


