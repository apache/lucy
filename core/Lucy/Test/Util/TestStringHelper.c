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

#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestStringHelper.h"
#include "Lucy/Util/StringHelper.h"

static void
test_overlap(TestBatch *batch) {
    int32_t result;
    result = StrHelp_overlap("", "", 0, 0);
    TEST_INT_EQ(batch, result, 0, "two empty strings");
    result = StrHelp_overlap("", "foo", 0, 3);
    TEST_INT_EQ(batch, result, 0, "first string is empty");
    result = StrHelp_overlap("foo", "", 3, 0);
    TEST_INT_EQ(batch, result, 0, "second string is empty");
    result = StrHelp_overlap("foo", "foo", 3, 3);
    TEST_INT_EQ(batch, result, 3, "equal strings");
    result = StrHelp_overlap("foo bar", "foo", 7, 3);
    TEST_INT_EQ(batch, result, 3, "first string is longer");
    result = StrHelp_overlap("foo", "foo bar", 3, 7);
    TEST_INT_EQ(batch, result, 3, "second string is longer");
}


static void
test_to_base36(TestBatch *batch) {
    char buffer[StrHelp_MAX_BASE36_BYTES];
    StrHelp_to_base36(U64_MAX, buffer);
    TEST_STR_EQ(batch, "3w5e11264sgsf", buffer, "base36 U64_MAX");
    StrHelp_to_base36(1, buffer);
    TEST_STR_EQ(batch, "1", buffer, "base36 1");
    TEST_INT_EQ(batch, buffer[1], 0, "base36 NULL termination");
}

static void
S_round_trip_utf8_code_point(TestBatch *batch, uint32_t code_point) {
    char buffer[4];
    uint32_t len   = StrHelp_encode_utf8_char(code_point, buffer);
    char *start = buffer;
    char *end   = start + len;
    TEST_TRUE(batch, StrHelp_utf8_valid(buffer, len), "Valid UTF-8 for %lu",
              (unsigned long)code_point);
    TEST_INT_EQ(batch, len, StrHelp_UTF8_COUNT[(unsigned char)buffer[0]],
                "length returned for %lu", (unsigned long)code_point);
    TEST_TRUE(batch, StrHelp_back_utf8_char(end, start) == start,
              "back_utf8_char for %lu", (unsigned long)code_point);
    TEST_INT_EQ(batch, StrHelp_decode_utf8_char(buffer), code_point,
                "round trip encode and decode for %lu", (unsigned long)code_point);
}

static void
test_utf8_round_trip(TestBatch *batch) {
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
test_is_whitespace(TestBatch *batch) {
    TEST_TRUE(batch, StrHelp_is_whitespace(' '), "space is whitespace");
    TEST_TRUE(batch, StrHelp_is_whitespace('\n'), "newline is whitespace");
    TEST_TRUE(batch, StrHelp_is_whitespace('\t'), "tab is whitespace");
    TEST_TRUE(batch, StrHelp_is_whitespace('\v'),
              "vertical tab is whitespace");
    TEST_TRUE(batch, StrHelp_is_whitespace(0x180E),
              "Mongolian vowel separator is whitespace");
    TEST_FALSE(batch, StrHelp_is_whitespace('a'), "'a' isn't whitespace");
    TEST_FALSE(batch, StrHelp_is_whitespace(0), "NULL isn't whitespace");
    TEST_FALSE(batch, StrHelp_is_whitespace(0x263A),
               "Smiley isn't whitespace");
}

static void
test_back_utf8_char(TestBatch *batch) {
    char buffer[4];
    char *buf = buffer + 1;
    uint32_t len = StrHelp_encode_utf8_char(0x263A, buffer);
    char *end = buffer + len;
    TEST_TRUE(batch, StrHelp_back_utf8_char(end, buffer) == buffer,
              "back_utf8_char");
    TEST_TRUE(batch, StrHelp_back_utf8_char(end, buf) == NULL,
              "back_utf8_char returns NULL rather than back up beyond start");
}

void
TestStrHelp_run_tests() {
    TestBatch *batch = TestBatch_new(43);

    TestBatch_Plan(batch);

    test_overlap(batch);
    test_to_base36(batch);
    test_utf8_round_trip(batch);
    test_is_whitespace(batch);
    test_back_utf8_char(batch);

    DECREF(batch);
}



