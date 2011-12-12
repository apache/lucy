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

#define C_LUCY_TESTNORMALIZER
#define C_LUCY_NORMALIZER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Analysis/TestNormalizer.h"
#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Util/Json.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    Normalizer *normalizer[4];

    CharBuf *NFC  = (CharBuf*)ZCB_WRAP_STR("NFC",  3);
    CharBuf *NFKC = (CharBuf*)ZCB_WRAP_STR("NFKC", 4);

    normalizer[0] = Normalizer_new(NFKC, true,  false);
    normalizer[1] = Normalizer_new(NFC,  true,  false);
    normalizer[2] = Normalizer_new(NFKC, false, false);
    normalizer[3] = Normalizer_new(NFKC, true,  true);

    TEST_FALSE(batch,
               Normalizer_Equals(normalizer[0], (Obj*)normalizer[1]),
               "Equals() false with different normalization form");
    TEST_FALSE(batch,
               Normalizer_Equals(normalizer[0], (Obj*)normalizer[2]),
               "Equals() false with different case_fold flag");
    TEST_FALSE(batch,
               Normalizer_Equals(normalizer[0], (Obj*)normalizer[3]),
               "Equals() false with different strip_accents flag");

    for (int i = 0; i < 4; ++i) {
        Obj *dump = (Obj*)Normalizer_Dump(normalizer[i]);
        Normalizer *clone = (Normalizer*)Normalizer_Load(normalizer[i], dump);

        TEST_TRUE(batch,
                  Normalizer_Equals(normalizer[i], (Obj*)clone),
                  "Dump => Load round trip");

        DECREF(normalizer[i]);
        DECREF(dump);
        DECREF(clone);
    }
}

static void
test_normalization(TestBatch *batch) {
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
    CB_setf(path, "unicode/utf8proc/tests.json");
    VArray *tests = (VArray*)Json_slurp_json((Folder*)modules_folder, path);
    if (!tests) { RETHROW(Err_get_error()); }

    for (uint32_t i = 0, max = VA_Get_Size(tests); i < max; i++) {
        Hash *test = (Hash*)VA_Fetch(tests, i);
        CharBuf *form = (CharBuf*)Hash_Fetch_Str(
                            test, "normalization_form", 18);
        bool_t case_fold = Bool_Get_Value((BoolNum*)Hash_Fetch_Str(
                                              test, "case_fold", 9));
        bool_t strip_accents = Bool_Get_Value((BoolNum*)Hash_Fetch_Str(
                                                  test, "strip_accents", 13));
        Normalizer *normalizer = Normalizer_new(form, case_fold, strip_accents);
        VArray *words = (VArray*)Hash_Fetch_Str(test, "words", 5);
        VArray *norms = (VArray*)Hash_Fetch_Str(test, "norms", 5);
        for (uint32_t j = 0, max = VA_Get_Size(words); j < max; j++) {
            CharBuf *word = (CharBuf*)VA_Fetch(words, j);
            VArray  *got  = Normalizer_Split(normalizer, word);
            CharBuf *norm = (CharBuf*)VA_Fetch(got, 0);
            TEST_TRUE(batch,
                      norm
                      && CB_Is_A(norm, CHARBUF)
                      && CB_Equals(norm, VA_Fetch(norms, j)),
                      "Normalize %s %d %d: %s", CB_Get_Ptr8(form),
                      case_fold, strip_accents, CB_Get_Ptr8(word)
                     );
            DECREF(got);
        }
        DECREF(normalizer);
    }

    DECREF(tests);
    DECREF(modules_folder);
    DECREF(path);
}

void
TestNormalizer_run_tests() {
    TestBatch *batch = TestBatch_new(20);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);
    test_normalization(batch);

    DECREF(batch);
}



