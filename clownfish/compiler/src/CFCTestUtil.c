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
#include <string.h>
#include <time.h>

#define CFC_USE_TEST_MACROS
#include "CFCUtil.h"
#include "CFCTest.h"

static void
S_run_tests(CFCTest *test);

static void
S_run_string_tests(CFCTest *test);

static void
S_run_file_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_UTIL = {
    "Clownfish::CFC::Util",
    15,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    S_run_string_tests(test);
    S_run_file_tests(test);
}

static void
S_run_string_tests(CFCTest *test) {
    const char *src = "Source string";
    char *str;

    str = CFCUtil_strdup(src);
    STR_EQ(test, str, src, "strdup");
    FREEMEM(str);
    str = CFCUtil_strndup(src, 6);
    STR_EQ(test, str, "Source", "strndup");
    FREEMEM(str);
    str = CFCUtil_sprintf("%s: %d", src, 123456789);
    STR_EQ(test, str, "Source string: 123456789", "sprintf");
    str = CFCUtil_cat(str, " ", "abc", NULL);
    STR_EQ(test, str, "Source string: 123456789 abc", "cat");
    FREEMEM(str);
    str = CFCUtil_strdup(" \r\n\tabc \r\n\tdef \r\n\t");
    CFCUtil_trim_whitespace(str);
    STR_EQ(test, str, "abc \r\n\tdef", "trim_whitespace");
    FREEMEM(str);
}

static void
S_run_file_tests(CFCTest *test) {
    const char *foo_txt = "foo.txt";
    remove(foo_txt);
    CFCUtil_write_file(foo_txt, "foo", 3);

    {
        FILE *file = fopen(foo_txt, "rb");
        OK(test, file != NULL, "can open file");
        char buf[10];
        size_t chars_read = fread(buf, 1, 10, file);
        INT_EQ(test, chars_read, 3, "read correct number of chars");
        OK(test, memcmp(buf, "foo", 3) == 0, "read correct string");

        long file_length = CFCUtil_flength(file);
        INT_EQ(test, file_length, 3, "flength");

        fclose(file);
    }

    {
        size_t content_len;
        char *content = CFCUtil_slurp_text(foo_txt, &content_len);
        INT_EQ(test, content_len, 3, "slurp_text len");
        OK(test, memcmp(content, "foo", 3) == 0, "slurp_text content");
        FREEMEM(content);
    }

    {
        OK(test, CFCUtil_current(foo_txt, foo_txt), "current");
        OK(test, !CFCUtil_current(foo_txt, "nonexistent_file"),
             "not current when dest file missing");
        // TODO: Test two different files.
    }

    {
        time_t past_time = time(NULL) - 10;
        CFCTest_set_file_times(foo_txt, past_time);
        past_time = CFCTest_get_file_mtime(foo_txt);
        time_t mtime;
        CFCUtil_write_if_changed(foo_txt, "foo", 3);
        mtime = CFCTest_get_file_mtime(foo_txt);
        OK(test, mtime == past_time,
           "write_if_changed does nothing if contents not changed");
        CFCUtil_write_if_changed(foo_txt, "foofoo", 6);
        mtime = CFCTest_get_file_mtime(foo_txt);
        OK(test, mtime != past_time,
           "write_if_changed writes if contents changed");
    }

    remove(foo_txt);
}

