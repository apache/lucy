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

#define C_TESTLUCY_TESTSTANDARDTOKENIZER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Analysis/TestStandardTokenizer.h"
#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Util/Json.h"

TestStandardTokenizer*
TestStandardTokenizer_new() {
    return (TestStandardTokenizer*)Class_Make_Obj(TESTSTANDARDTOKENIZER);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    Obj *dump  = StandardTokenizer_Dump(tokenizer);
    StandardTokenizer *clone
        = (StandardTokenizer*)StandardTokenizer_Load(tokenizer, dump);

    TEST_TRUE(runner,
              StandardTokenizer_Equals(tokenizer, (Obj*)clone),
              "Dump => Load round trip");

    DECREF(tokenizer);
    DECREF(dump);
    DECREF(clone);
}

static void
test_tokenizer(TestBatchRunner *runner) {
    StandardTokenizer *tokenizer = StandardTokenizer_new();

    StackString *word = SSTR_WRAP_UTF8(
                              " ."
                              "tha\xCC\x82t's"
                              ":"
                              "1,02\xC2\xADZ4.38"
                              "\xE0\xB8\x81\xC2\xAD\xC2\xAD"
                              "\xF0\xA0\x80\x80"
                              "a"
                              "/",
                              35);
    VArray *got = StandardTokenizer_Split(tokenizer, (String*)word);
    String *token = (String*)VA_Fetch(got, 0);
    TEST_TRUE(runner,
              token
              && Str_Is_A(token, STRING)
              && Str_Equals_Utf8(token, "tha\xcc\x82t's", 8),
              "Token: %s", Str_Get_Ptr8(token));
    token = (String*)VA_Fetch(got, 1);
    TEST_TRUE(runner,
              token
              && Str_Is_A(token, STRING)
              && Str_Equals_Utf8(token, "1,02\xC2\xADZ4.38", 11),
              "Token: %s", Str_Get_Ptr8(token));
    token = (String*)VA_Fetch(got, 2);
    TEST_TRUE(runner,
              token
              && Str_Is_A(token, STRING)
              && Str_Equals_Utf8(token, "\xE0\xB8\x81\xC2\xAD\xC2\xAD", 7),
              "Token: %s", Str_Get_Ptr8(token));
    token = (String*)VA_Fetch(got, 3);
    TEST_TRUE(runner,
              token
              && Str_Is_A(token, STRING)
              && Str_Equals_Utf8(token, "\xF0\xA0\x80\x80", 4),
              "Token: %s", Str_Get_Ptr8(token));
    token = (String*)VA_Fetch(got, 4);
    TEST_TRUE(runner,
              token
              && Str_Is_A(token, STRING)
              && Str_Equals_Utf8(token, "a", 1),
              "Token: %s", Str_Get_Ptr8(token));
    DECREF(got);

    FSFolder *modules_folder = TestUtils_modules_folder();
    String *path = Str_newf("unicode/ucd/WordBreakTest.json");
    VArray *tests = (VArray*)Json_slurp_json((Folder*)modules_folder, path);
    if (!tests) { RETHROW(Err_get_error()); }

    for (uint32_t i = 0, max = VA_Get_Size(tests); i < max; i++) {
        Hash *test = (Hash*)VA_Fetch(tests, i);
        String *text = (String*)Hash_Fetch_Utf8(test, "text", 4);
        VArray *wanted = (VArray*)Hash_Fetch_Utf8(test, "words", 5);
        VArray *got = StandardTokenizer_Split(tokenizer, text);
        TEST_TRUE(runner, VA_Equals(wanted, (Obj*)got), "UCD test #%d", i + 1);
        DECREF(got);
    }

    DECREF(tests);
    DECREF(modules_folder);
    DECREF(path);

    DECREF(tokenizer);
}

void
TestStandardTokenizer_Run_IMP(TestStandardTokenizer *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 1378);
    test_Dump_Load_and_Equals(runner);
    test_tokenizer(runner);
}


