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

#define C_LUCY_TESTSTANDARDTOKENIZER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Analysis/TestStandardTokenizer.h"
#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Util/Json.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    Obj *dump  = StandardTokenizer_Dump(tokenizer);
    StandardTokenizer *clone = (StandardTokenizer*)StandardTokenizer_Load(tokenizer, dump);

    TEST_TRUE(batch,
              StandardTokenizer_Equals(tokenizer, (Obj*)clone),
              "Dump => Load round trip");

    DECREF(tokenizer);
    DECREF(dump);
    DECREF(clone);
}

static void
test_tokenizer(TestBatch *batch) {
    StandardTokenizer *tokenizer = StandardTokenizer_new();

    ZombieCharBuf *word = ZCB_WRAP_STR(
                              " ."
                              "tha\xCC\x82t's"
                              ":"
                              "1,02\xC2\xADZ4.38"
                              "\xE0\xB8\x81\xC2\xAD\xC2\xAD"
                              "\xF0\xA0\x80\x80"
                              "a"
                              "/",
                              35);
    VArray *got = StandardTokenizer_Split(tokenizer, (CharBuf*)word);
    CharBuf *token = (CharBuf*)VA_Fetch(got, 0);
    TEST_TRUE(batch,
              token
              && CB_Is_A(token, CHARBUF)
              && CB_Equals_Str(token, "tha\xcc\x82t's", 8),
              "Token: %s", CB_Get_Ptr8(token));
    token = (CharBuf*)VA_Fetch(got, 1);
    TEST_TRUE(batch,
              token
              && CB_Is_A(token, CHARBUF)
              && CB_Equals_Str(token, "1,02\xC2\xADZ4.38", 11),
              "Token: %s", CB_Get_Ptr8(token));
    token = (CharBuf*)VA_Fetch(got, 2);
    TEST_TRUE(batch,
              token
              && CB_Is_A(token, CHARBUF)
              && CB_Equals_Str(token, "\xE0\xB8\x81\xC2\xAD\xC2\xAD", 7),
              "Token: %s", CB_Get_Ptr8(token));
    token = (CharBuf*)VA_Fetch(got, 3);
    TEST_TRUE(batch,
              token
              && CB_Is_A(token, CHARBUF)
              && CB_Equals_Str(token, "\xF0\xA0\x80\x80", 4),
              "Token: %s", CB_Get_Ptr8(token));
    token = (CharBuf*)VA_Fetch(got, 4);
    TEST_TRUE(batch,
              token
              && CB_Is_A(token, CHARBUF)
              && CB_Equals_Str(token, "a", 1),
              "Token: %s", CB_Get_Ptr8(token));
    DECREF(got);

    CharBuf  *path           = CB_newf("modules");
    FSFolder *modules_folder = FSFolder_new(path);
    if (!FSFolder_Check(modules_folder)) {
        DECREF(modules_folder);
        CB_setf(path, "../modules");
        modules_folder = FSFolder_new(path);
        if (!FSFolder_Check(modules_folder)) {
            THROW(ERR, "Can't open modules folder");
        }
    }
    CB_setf(path, "unicode/ucd/WordBreakTest.json");
    VArray *tests = (VArray*)Json_slurp_json((Folder*)modules_folder, path);
    if (!tests) { RETHROW(Err_get_error()); }

    for (uint32_t i = 0, max = VA_Get_Size(tests); i < max; i++) {
        Hash *test = (Hash*)VA_Fetch(tests, i);
        CharBuf *text = (CharBuf*)Hash_Fetch_Str(test, "text", 4);
        VArray *wanted = (VArray*)Hash_Fetch_Str(test, "words", 5);
        VArray *got = StandardTokenizer_Split(tokenizer, text);
        TEST_TRUE(batch, VA_Equals(wanted, (Obj*)got), "UCD test #%d", i + 1);
        DECREF(got);
    }

    DECREF(tests);
    DECREF(modules_folder);
    DECREF(path);

    DECREF(tokenizer);
}

void
TestStandardTokenizer_run_tests() {
    TestBatch *batch = TestBatch_new(984);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);
    test_tokenizer(batch);

    DECREF(batch);
}


