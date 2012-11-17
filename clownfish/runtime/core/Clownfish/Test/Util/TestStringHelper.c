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

#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test.h"
#include "Clownfish/Test/Util/TestStringHelper.h"
#include "Clownfish/Test/TestUtils.h"
#include "Clownfish/Util/StringHelper.h"

/* This alternative implementation of utf8_valid() is (presumably) slower, but
 * it implements the standard in a more linear, easy-to-grok way.
 */
#define TRAIL_OK(n) (n >= 0x80 && n <= 0xBF)
static bool_t
S_utf8_valid_alt(const char *maybe_utf8, size_t size) {
    const uint8_t *string = (const uint8_t*)maybe_utf8;
    const uint8_t *const end = string + size;
    while (string < end) {
        int count = StrHelp_UTF8_COUNT[*string];
        bool_t valid = false;
        if (count == 1) {
            if (string[0] <= 0x7F) {
                valid = true;
            }
        }
        else if (count == 2) {
            if (string[0] >= 0xC2 && string[0] <= 0xDF) {
                if (TRAIL_OK(string[1])) {
                    valid = true;
                }
            }
        }
        else if (count == 3) {
            if (string[0] == 0xE0) {
                if (string[1] >= 0xA0 && string[1] <= 0xBF
                    && TRAIL_OK(string[2])
                   ) {
                    valid = true;
                }
            }
            else if (string[0] >= 0xE1 && string[0] <= 0xEC) {
                if (TRAIL_OK(string[1])
                    && TRAIL_OK(string[2])
                   ) {
                    valid = true;
                }
            }
            else if (string[0] == 0xED) {
                if (string[1] >= 0x80 && string[1] <= 0x9F
                    && TRAIL_OK(string[2])
                   ) {
                    valid = true;
                }
            }
            else if (string[0] >= 0xEE && string[0] <= 0xEF) {
                if (TRAIL_OK(string[1])
                    && TRAIL_OK(string[2])
                   ) {
                    valid = true;
                }
            }
        }
        else if (count == 4) {
            if (string[0] == 0xF0) {
                if (string[1] >= 0x90 && string[1] <= 0xBF
                    && TRAIL_OK(string[2])
                    && TRAIL_OK(string[3])
                   ) {
                    valid = true;
                }
            }
            else if (string[0] >= 0xF1 && string[0] <= 0xF3) {
                if (TRAIL_OK(string[1])
                    && TRAIL_OK(string[2])
                    && TRAIL_OK(string[3])
                   ) {
                    valid = true;
                }
            }
            else if (string[0] == 0xF4) {
                if (string[1] >= 0x80 && string[1] <= 0x8F
                    && TRAIL_OK(string[2])
                    && TRAIL_OK(string[3])
                   ) {
                    valid = true;
                }
            }
        }

        if (!valid) {
            return false;
        }
        string += count;
    }

    if (string != end) {
        return false;
    }

    return true;
}

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
    StrHelp_to_base36(UINT64_MAX, buffer);
    TEST_STR_EQ(batch, "3w5e11264sgsf", buffer, "base36 UINT64_MAX");
    StrHelp_to_base36(1, buffer);
    TEST_STR_EQ(batch, "1", buffer, "base36 1");
    TEST_INT_EQ(batch, buffer[1], 0, "base36 NULL termination");
}

static void
test_utf8_round_trip(TestBatch *batch) {
    uint32_t code_point;
    for (code_point = 0; code_point <= 0x10FFFF; code_point++) {
        char buffer[4];
        uint32_t size = StrHelp_encode_utf8_char(code_point, buffer);
        char *start = buffer;
        char *end   = start + size;

        // Verify length returned by encode_utf8_char().
        if (size != StrHelp_UTF8_COUNT[(unsigned char)buffer[0]]) {
            break;
        }
        // Verify that utf8_valid() agrees with alternate implementation.
        if (!!StrHelp_utf8_valid(start, size)
            != !!S_utf8_valid_alt(start, size)
           ) {
            break;
        }

        // Verify back_utf8_char().
        if (StrHelp_back_utf8_char(end, start) != start) {
            break;
        }

        // Verify round trip of encode/decode.
        if (StrHelp_decode_utf8_char(buffer) != code_point) {
            break;
        }
    }
    if (code_point == 0x110000) {
        PASS(batch, "Successfully round tripped 0 - 0x10FFFF");
    }
    else {
        FAIL(batch, "Failed round trip at 0x%.1X", (unsigned)code_point);
    }
}

static void
S_test_validity(TestBatch *batch, const char *content, size_t size,
                bool_t expected, const char *description) {
    bool_t sane = StrHelp_utf8_valid(content, size);
    bool_t double_check = S_utf8_valid_alt(content, size);
    if (sane != double_check) {
        FAIL(batch, "Disagreement: %s", description);
    }
    else {
        TEST_TRUE(batch, sane == expected, "%s", description);
    }
}

static void
test_utf8_valid(TestBatch *batch) {
    // Musical symbol G clef:
    // Code point: U+1D11E
    // UTF-16:     0xD834 0xDD1E
    // UTF-8       0xF0 0x9D 0x84 0x9E
    S_test_validity(batch, "\xF0\x9D\x84\x9E", 4, true,
                    "Musical symbol G clef");
    S_test_validity(batch, "\xED\xA0\xB4\xED\xB4\x9E", 6, false,
                    "G clef as UTF-8 encoded UTF-16 surrogates");
    S_test_validity(batch, ".\xED\xA0\xB4.", 5, false,
                    "Isolated high surrogate");
    S_test_validity(batch, ".\xED\xB4\x9E.", 5, false,
                    "Isolated low surrogate");

    // Shortest form.
    S_test_validity(batch, ".\xC1\x9C.", 4, false,
                    "Non-shortest form ASCII backslash");
    S_test_validity(batch, ".\xC0\xAF.", 4, false,
                    "Non-shortest form ASCII slash");
    S_test_validity(batch, ".\xC0\x80.", 4, false,
                    "Non-shortest form ASCII NUL character");

    // Range.
    S_test_validity(batch, "\xF8\x88\x80\x80\x80", 5, false, "5-byte UTF-8");

    // Bad continuations.
    S_test_validity(batch, "\xE2\x98\xBA\xE2\x98\xBA", 6, true,
                    "SmileySmiley");
    S_test_validity(batch, "\xE2\xBA\xE2\x98\xBA", 5, false,
                    "missing first continuation byte");
    S_test_validity(batch, "\xE2\x98\xE2\x98\xBA", 5, false,
                    "missing second continuation byte");
    S_test_validity(batch, "\xE2\xE2\x98\xBA", 4, false,
                    "missing both continuation bytes");
    S_test_validity(batch, "\xBA\xE2\x98\xBA\xE2\xBA", 5, false,
                    "missing first continuation byte (end)");
    S_test_validity(batch, "\xE2\x98\xBA\xE2\x98", 5, false,
                    "missing second continuation byte (end)");
    S_test_validity(batch, "\xE2\x98\xBA\xE2", 4, false,
                    "missing both continuation bytes (end)");
    S_test_validity(batch, "\xBA\xE2\x98\xBA", 4, false,
                    "isolated continuation byte 0xBA");
    S_test_validity(batch, "\x98\xE2\x98\xBA", 4, false,
                    "isolated continuation byte 0x98");
    S_test_validity(batch, "\xE2\x98\xBA\xBA", 4, false,
                    "isolated continuation byte 0xBA (end)");
    S_test_validity(batch, "\xE2\x98\xBA\x98", 4, false,
                    "isolated continuation byte 0x98 (end)");
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
    TEST_TRUE(batch, StrHelp_back_utf8_char(buffer, buffer) == NULL,
              "back_utf8_char returns NULL when end == start");
}

void
TestStrHelp_run_tests() {
    TestBatch *batch = TestBatch_new(40);

    TestBatch_Plan(batch);

    test_overlap(batch);
    test_to_base36(batch);
    test_utf8_round_trip(batch);
    test_utf8_valid(batch);
    test_is_whitespace(batch);
    test_back_utf8_char(batch);

    DECREF(batch);
}



