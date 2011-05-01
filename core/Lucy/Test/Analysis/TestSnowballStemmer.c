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

#define C_LUCY_TESTSNOWBALLSTEMMER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Analysis/TestSnowballStemmer.h"
#include "Lucy/Analysis/SnowballStemmer.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Util/Json.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    CharBuf *EN = (CharBuf*)ZCB_WRAP_STR("en", 2);
    CharBuf *ES = (CharBuf*)ZCB_WRAP_STR("es", 2);
    SnowballStemmer *stemmer = SnowStemmer_new(EN);
    SnowballStemmer *other   = SnowStemmer_new(ES);
    Obj *dump       = (Obj*)SnowStemmer_Dump(stemmer);
    Obj *other_dump = (Obj*)SnowStemmer_Dump(other);
    SnowballStemmer *clone       = (SnowballStemmer*)SnowStemmer_Load(other, dump);
    SnowballStemmer *other_clone = (SnowballStemmer*)SnowStemmer_Load(other, other_dump);

    TEST_FALSE(batch,
               SnowStemmer_Equals(stemmer, (Obj*)other),
               "Equals() false with different language");
    TEST_TRUE(batch,
              SnowStemmer_Equals(stemmer, (Obj*)clone),
              "Dump => Load round trip");
    TEST_TRUE(batch,
              SnowStemmer_Equals(other, (Obj*)other_clone),
              "Dump => Load round trip");

    DECREF(stemmer);
    DECREF(dump);
    DECREF(clone);
    DECREF(other);
    DECREF(other_dump);
    DECREF(other_clone);
}

static void
test_stemming(TestBatch *batch) {
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
    CB_setf(path, "analysis/snowstem/source/test/tests.json");
    Hash *tests = (Hash*)Json_slurp_json((Folder*)modules_folder, path);
    if (!tests) { RETHROW(Err_get_error()); }

    CharBuf *iso;
    Hash *lang_data;
    Hash_Iterate(tests);
    while (Hash_Next(tests, (Obj**)&iso, (Obj**)&lang_data)) {
        VArray *words = (VArray*)Hash_Fetch_Str(lang_data, "words", 5);
        VArray *stems = (VArray*)Hash_Fetch_Str(lang_data, "stems", 5);
        SnowballStemmer *stemmer = SnowStemmer_new(iso);
        for (uint32_t i = 0, max = VA_Get_Size(words); i < max; i++) {
            CharBuf *word  = (CharBuf*)VA_Fetch(words, i);
            VArray  *got   = SnowStemmer_Split(stemmer, word);
            CharBuf *stem  = (CharBuf*)VA_Fetch(got, 0);
            TEST_TRUE(batch,
                      stem
                      && CB_Is_A(stem, CHARBUF)
                      && CB_Equals(stem, VA_Fetch(stems, i)),
                      "Stem %s: %s", CB_Get_Ptr8(iso), CB_Get_Ptr8(word)
                     );
            DECREF(got);
        }
        DECREF(stemmer);
    }

    DECREF(tests);
    DECREF(modules_folder);
    DECREF(path);
}

void
TestSnowStemmer_run_tests() {
    TestBatch *batch = TestBatch_new(153);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);
    test_stemming(batch);

    DECREF(batch);
}



