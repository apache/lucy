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

#define C_TESTLUCY_TESTNORMALIZER
#define C_LUCY_NORMALIZER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Analysis/TestNormalizer.h"
#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Util/Json.h"
#include "utf8proc.h"

TestNormalizer*
TestNormalizer_new() {
    return (TestNormalizer*)Class_Make_Obj(TESTNORMALIZER);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    Normalizer *normalizer[4];

    String *NFC  = (String*)SSTR_WRAP_UTF8("NFC",  3);
    String *NFKC = (String*)SSTR_WRAP_UTF8("NFKC", 4);

    normalizer[0] = Normalizer_new(NFKC, true,  false);
    normalizer[1] = Normalizer_new(NFC,  true,  false);
    normalizer[2] = Normalizer_new(NFKC, false, false);
    normalizer[3] = Normalizer_new(NFKC, true,  true);

    TEST_FALSE(runner,
               Normalizer_Equals(normalizer[0], (Obj*)normalizer[1]),
               "Equals() false with different normalization form");
    TEST_FALSE(runner,
               Normalizer_Equals(normalizer[0], (Obj*)normalizer[2]),
               "Equals() false with different case_fold flag");
    TEST_FALSE(runner,
               Normalizer_Equals(normalizer[0], (Obj*)normalizer[3]),
               "Equals() false with different strip_accents flag");

    for (int i = 0; i < 4; ++i) {
        Obj *dump = (Obj*)Normalizer_Dump(normalizer[i]);
        Normalizer *clone = (Normalizer*)Normalizer_Load(normalizer[i], dump);

        TEST_TRUE(runner,
                  Normalizer_Equals(normalizer[i], (Obj*)clone),
                  "Dump => Load round trip");

        DECREF(normalizer[i]);
        DECREF(dump);
        DECREF(clone);
    }
}

static void
test_normalization(TestBatchRunner *runner) {
    FSFolder *modules_folder = TestUtils_modules_folder();
    String *path = Str_newf("unicode/utf8proc/tests.json");
    VArray *tests = (VArray*)Json_slurp_json((Folder*)modules_folder, path);
    if (!tests) { RETHROW(Err_get_error()); }

    for (uint32_t i = 0, max = VA_Get_Size(tests); i < max; i++) {
        Hash *test = (Hash*)VA_Fetch(tests, i);
        String *form = (String*)Hash_Fetch_Utf8(
                            test, "normalization_form", 18);
        bool case_fold = Bool_Get_Value((BoolNum*)Hash_Fetch_Utf8(
                                              test, "case_fold", 9));
        bool strip_accents = Bool_Get_Value((BoolNum*)Hash_Fetch_Utf8(
                                                  test, "strip_accents", 13));
        Normalizer *normalizer = Normalizer_new(form, case_fold, strip_accents);
        VArray *words = (VArray*)Hash_Fetch_Utf8(test, "words", 5);
        VArray *norms = (VArray*)Hash_Fetch_Utf8(test, "norms", 5);
        for (uint32_t j = 0, max = VA_Get_Size(words); j < max; j++) {
            String *word = (String*)VA_Fetch(words, j);
            VArray *got  = Normalizer_Split(normalizer, word);
            String *norm = (String*)VA_Fetch(got, 0);
            TEST_TRUE(runner,
                      norm
                      && Str_Is_A(norm, STRING)
                      && Str_Equals(norm, VA_Fetch(norms, j)),
                      "Normalize %s %d %d: %s", Str_Get_Ptr8(form),
                      case_fold, strip_accents, Str_Get_Ptr8(word)
                     );
            DECREF(got);
        }
        DECREF(normalizer);
    }

    DECREF(tests);
    DECREF(modules_folder);
    DECREF(path);
}

static void
test_utf8proc_normalization(TestBatchRunner *runner) {
    SKIP(runner, "utf8proc can't handle control chars or Unicode non-chars");
    return;

    for (int32_t i = 0; i < 100; i++) {
        String *source = TestUtils_random_string(rand() % 40);

        // Normalize once.
        uint8_t *normalized;
        int32_t check = utf8proc_map((const uint8_t*)Str_Get_Ptr8(source),
                                     Str_Get_Size(source),
                                     &normalized,
                                     UTF8PROC_STABLE  |
                                     UTF8PROC_COMPOSE |
                                     UTF8PROC_COMPAT  |
                                     UTF8PROC_CASEFOLD);
        if (check < 0) {
            lucy_Json_set_tolerant(1);
            String *json = lucy_Json_to_json((Obj*)source);
            if (!json) {
                json = Str_newf("[failed to encode]");
            }
            FAIL(runner, "Failed to normalize: %s", Str_Get_Ptr8(json));
            DECREF(json);
            DECREF(source);
            return;
        }

        // Normalize again.
        size_t normalized_len = strlen((char*)normalized);
        uint8_t *dupe;
        int32_t dupe_check = utf8proc_map(normalized, normalized_len, &dupe,
                                          UTF8PROC_STABLE  |
                                          UTF8PROC_COMPOSE |
                                          UTF8PROC_COMPAT  |
                                          UTF8PROC_CASEFOLD);
        if (dupe_check < 0) {
            THROW(ERR, "Unexpected normalization error: %i32", dupe_check);
        }
        int comparison = strcmp((char*)normalized, (char*)dupe);
        free(dupe);
        free(normalized);
        DECREF(source);
        if (comparison != 0) {
            FAIL(runner, "Not fully normalized");
            return;
        }
    }
    PASS(runner, "Normalization successful.");
}

void
TestNormalizer_Run_IMP(TestNormalizer *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 21);
    test_Dump_Load_and_Equals(runner);
    test_normalization(runner);
    test_utf8proc_normalization(runner);
}



