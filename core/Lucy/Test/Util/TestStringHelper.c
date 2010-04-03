#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestStringHelper.h"
#include "Lucy/Util/StringHelper.h"

static void
test_SKIP_and_TRAILING(TestBatch *batch)
{
    uint8_t i, max;
    
    // Some of the upper max boundaries are skipped (e.g. 127)
    // because they may not appear as initial bytes in legal UTF-8.
    for (i = 0, max = 127; i < max; i++) {
        ASSERT_INT_EQ(batch, StrHelp_UTF8_SKIP[i], 1, 
            "UTF8_SKIP ascii %d", (int)i);
        ASSERT_INT_EQ(batch, StrHelp_UTF8_TRAILING[i], 0, 
            "UTF8_TRAILING ascii %d", (int)i);
    }
    for (i = 128, max = 193; i < max; i++) {
        ASSERT_INT_EQ(batch, StrHelp_UTF8_SKIP[i], 1, 
            "UTF8_SKIP bogus %d", (int)i);
        ASSERT_INT_EQ(batch, StrHelp_UTF8_TRAILING[i], 7, 
            "UTF8_TRAILING bogus %d", (int)i);
    }
    for (i = 194, max = 223; i < max; i++) {
        ASSERT_INT_EQ(batch, StrHelp_UTF8_SKIP[i], 2, 
            "UTF8_SKIP two-byte %d", (int)i);
        ASSERT_INT_EQ(batch, StrHelp_UTF8_TRAILING[i], 1, 
            "UTF8_TRAILING two-byte %d", (int)i);
    }
    for (i = 224, max = 239; i < max; i++) {
        ASSERT_INT_EQ(batch, StrHelp_UTF8_SKIP[i], 3, 
            "UTF8_SKIP three-byte %d", (int)i);
        ASSERT_INT_EQ(batch, StrHelp_UTF8_TRAILING[i], 2, 
            "UTF8_TRAILING three-byte %d", (int)i);
    }
    for (i = 240, max = 244; i < max; i++) {
        ASSERT_INT_EQ(batch, StrHelp_UTF8_SKIP[i], 4, 
            "UTF8_SKIP four-byte %d", (int)i);
        ASSERT_INT_EQ(batch, StrHelp_UTF8_TRAILING[i], 3, 
            "UTF8_TRAILING four-byte %d", (int)i);
    }
    for (i = 245, max = 255; i < max; i++) {
        ASSERT_TRUE(batch, StrHelp_UTF8_SKIP[i] > 0, 
            "UTF8_SKIP bogus but no memory problems %d", (int)i);
        ASSERT_TRUE(batch, StrHelp_UTF8_TRAILING[i] == 7, 
            "UTF8_TRAILING bogus but no memory problems %d", (int)i);
    }
}

static void
test_overlap(TestBatch *batch)
{
    int32_t result;
    result = StrHelp_overlap("", "", 0, 0);
    ASSERT_INT_EQ(batch, result, 0, "two empty strings");
    result = StrHelp_overlap("", "foo", 0, 3);
    ASSERT_INT_EQ(batch, result, 0, "first string is empty");
    result = StrHelp_overlap("foo", "", 3, 0);
    ASSERT_INT_EQ(batch, result, 0, "second string is empty");
    result = StrHelp_overlap("foo", "foo", 3, 3);
    ASSERT_INT_EQ(batch, result, 3, "equal strings");
    result = StrHelp_overlap("foo bar", "foo", 7, 3);
    ASSERT_INT_EQ(batch, result, 3, "first string is longer");
    result = StrHelp_overlap("foo", "foo bar", 3, 7);
    ASSERT_INT_EQ(batch, result, 3, "second string is longer");
}


static void
test_to_base36(TestBatch *batch)
{
    char buffer[StrHelp_MAX_BASE36_BYTES];
    StrHelp_to_base36(U64_MAX, buffer);
    ASSERT_STR_EQ(batch, "3w5e11264sgsf", buffer, "base36 U64_MAX");
    StrHelp_to_base36(1, buffer);
    ASSERT_STR_EQ(batch, "1", buffer, "base36 1");
    ASSERT_INT_EQ(batch, buffer[1], 0, "base36 NULL termination");
}

static void
S_round_trip_utf8_code_point(TestBatch *batch, uint32_t code_point)
{
    char buffer[4];
    uint32_t len   = StrHelp_encode_utf8_char(code_point, buffer);
    char *start = buffer;
    char *end   = start + len;
    ASSERT_TRUE(batch, StrHelp_utf8_valid(buffer, len), "Valid UTF-8 for %lu", 
        (unsigned long)code_point);
    ASSERT_INT_EQ(batch, len, StrHelp_UTF8_SKIP[(unsigned char)buffer[0]], 
        "length returned for %lu", (unsigned long)code_point);
    ASSERT_TRUE(batch, StrHelp_back_utf8_char(end, start) == start, 
        "back_utf8_char for %lu", (unsigned long)code_point);
    ASSERT_INT_EQ(batch, StrHelp_decode_utf8_char(buffer), code_point,
        "round trip encode and decode for %lu", (unsigned long)code_point);
}

static void
test_utf8_round_trip(TestBatch *batch)
{
    uint32_t code_points[] = { 
        0, 
        0xA,      // newline 
        'a', 
        128,      // two-byte 
        0x263A,   // smiley (three-byte)  
        0x10FFFF, // Max legal code point (four-byte). 
    };
    uint32_t num_code_points = sizeof(code_points) / sizeof(uint32_t);
    uint32_t i;
    for (i = 0; i < num_code_points; i++) {
        S_round_trip_utf8_code_point(batch, code_points[i]);
    }
}

static void
test_is_whitespace(TestBatch *batch)
{
    ASSERT_TRUE(batch, StrHelp_is_whitespace(' '), "space is whitespace");
    ASSERT_TRUE(batch, StrHelp_is_whitespace('\n'), "newline is whitespace");
    ASSERT_TRUE(batch, StrHelp_is_whitespace('\t'), "tab is whitespace");
    ASSERT_TRUE(batch, StrHelp_is_whitespace('\v'), 
        "vertical tab is whitespace");
    ASSERT_TRUE(batch, StrHelp_is_whitespace(0x180E), 
        "Mongolian vowel separator is whitespace");
    ASSERT_FALSE(batch, StrHelp_is_whitespace('a'), "'a' isn't whitespace");
    ASSERT_FALSE(batch, StrHelp_is_whitespace(0), "NULL isn't whitespace");
    ASSERT_FALSE(batch, StrHelp_is_whitespace(0x263A), 
        "Smiley isn't whitespace");
}

static void
test_back_utf8_char(TestBatch *batch)
{
    char buffer[4];
    char *buf = buffer + 1;
    uint32_t len = StrHelp_encode_utf8_char(0x263A, buffer);
    char *end = buffer + len;
    ASSERT_TRUE(batch, StrHelp_back_utf8_char(end, buffer) == buffer,
        "back_utf8_char");
    ASSERT_TRUE(batch, StrHelp_back_utf8_char(end, buf) == NULL,
        "back_utf8_char returns NULL rather than back up beyond start");
}

static void
S_set_buf(char *buffer, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    uint8_t *buf = (uint8_t*)buffer;
    buf[0] = a;
    buf[1] = b;
    buf[2] = c;
    buf[3] = d;
}

#include <stdio.h>
static void
test_invalid_utf8(TestBatch *batch)
{
    char buffer[8];

    S_set_buf(buffer, 0xC0, 0x80, 0, 0);
    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 2), 
        "Modified UTF-8 NULL fails");

    S_set_buf(buffer, 0x81, 0, 0, 0);
    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 1), 
        "Isolated continuation byte fails");

    StrHelp_encode_utf8_char(0x263A, buffer);
    buffer[2] = 'a';
    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 3), 
        "Interrupted 3-byte sequence fails");

    StrHelp_encode_utf8_char(0x10FFFF, buffer);
    buffer[3] = 'a';
    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 4), 
        "Interrupted 4-byte sequence fails");

    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 3), 
        "Truncated 4-byte sequence fails");

    StrHelp_encode_utf8_char(0x10FFFF, buffer);
    buffer[3] = 'a';
    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 4), 
        "Interrupted 4-byte sequence fails");

    S_set_buf(buffer, 0xF5, 0x80, 0x80, 0x80);
    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 4), 
        "Illegal 4-byte sequence for code point above 0x10FFFF fails");

    buffer[0] = (char)0xF8;
    buffer[1] = (char)0x80;
    buffer[2] = (char)0x80;
    buffer[3] = (char)0x80;
    buffer[4] = (char)0x80;
    ASSERT_FALSE(batch, StrHelp_utf8_valid(buffer, 5), 
        "Illegal 5-byte sequence fails");

    {
        uint32_t i;
        for (i = 0; i <= 0x10FFFF; i++) {
            uint32_t len = StrHelp_encode_utf8_char(i, buffer);
            if (!StrHelp_utf8_valid(buffer, len)) {
                FAIL(batch, "Encoding or validity failure for %lu", 
                    (unsigned long)i);
                break;
            }
            else if (i == 0x10FFFF) {
                PASS(batch, "All valid Unicode code points pass");
                break;
            }
        }
    }
}

void
TestStrHelp_run_tests()
{
    TestBatch *batch = TestBatch_new(552);

    TestBatch_Plan(batch);

    test_SKIP_and_TRAILING(batch);
    test_overlap(batch);
    test_to_base36(batch);
    test_utf8_round_trip(batch);
    test_is_whitespace(batch);
    test_back_utf8_char(batch);
    test_invalid_utf8(batch);

    DECREF(batch);
}


/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

