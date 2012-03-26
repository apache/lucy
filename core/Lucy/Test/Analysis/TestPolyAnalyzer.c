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

#define C_LUCY_TESTPOLYANALYZER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Analysis/TestPolyAnalyzer.h"
#include "Lucy/Analysis/PolyAnalyzer.h"
#include "Lucy/Analysis/CaseFolder.h"
#include "Lucy/Analysis/SnowballStopFilter.h"
#include "Lucy/Analysis/SnowballStemmer.h"
#include "Lucy/Analysis/RegexTokenizer.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    CharBuf      *EN          = (CharBuf*)ZCB_WRAP_STR("en", 2);
    CharBuf      *ES          = (CharBuf*)ZCB_WRAP_STR("es", 2);
    PolyAnalyzer *analyzer    = PolyAnalyzer_new(EN, NULL);
    PolyAnalyzer *other       = PolyAnalyzer_new(ES, NULL);
    Obj          *dump        = (Obj*)PolyAnalyzer_Dump(analyzer);
    Obj          *other_dump  = (Obj*)PolyAnalyzer_Dump(other);
    PolyAnalyzer *clone       = (PolyAnalyzer*)PolyAnalyzer_Load(other, dump);
    PolyAnalyzer *other_clone
        = (PolyAnalyzer*)PolyAnalyzer_Load(other, other_dump);

    TEST_FALSE(batch, PolyAnalyzer_Equals(analyzer, (Obj*)other),
               "Equals() false with different language");
    TEST_TRUE(batch, PolyAnalyzer_Equals(analyzer, (Obj*)clone),
              "Dump => Load round trip");
    TEST_TRUE(batch, PolyAnalyzer_Equals(other, (Obj*)other_clone),
              "Dump => Load round trip");

    DECREF(analyzer);
    DECREF(dump);
    DECREF(clone);
    DECREF(other);
    DECREF(other_dump);
    DECREF(other_clone);
}

static void
test_analysis(TestBatch *batch) {
    CharBuf            *EN          = (CharBuf*)ZCB_WRAP_STR("en", 2);
    CharBuf            *source_text = CB_newf("Eats, shoots and leaves.");
    CaseFolder         *case_folder = CaseFolder_new();
    RegexTokenizer     *tokenizer   = RegexTokenizer_new(NULL);
    SnowballStopFilter *stopfilter  = SnowStop_new(EN, NULL);
    SnowballStemmer    *stemmer     = SnowStemmer_new(EN);

    {
        VArray       *analyzers    = VA_new(0);
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, INCREF(source_text));
        TestUtils_test_analyzer(batch, (Analyzer*)polyanalyzer, source_text,
                                expected, "No sub analyzers");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(case_folder));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)CB_newf("eats, shoots and leaves."));
        TestUtils_test_analyzer(batch, (Analyzer*)polyanalyzer, source_text,
                                expected, "With CaseFolder");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(case_folder));
        VA_Push(analyzers, INCREF(tokenizer));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)CB_newf("eats"));
        VA_Push(expected, (Obj*)CB_newf("shoots"));
        VA_Push(expected, (Obj*)CB_newf("and"));
        VA_Push(expected, (Obj*)CB_newf("leaves"));
        TestUtils_test_analyzer(batch, (Analyzer*)polyanalyzer, source_text,
                                expected, "With RegexTokenizer");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(case_folder));
        VA_Push(analyzers, INCREF(tokenizer));
        VA_Push(analyzers, INCREF(stopfilter));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)CB_newf("eats"));
        VA_Push(expected, (Obj*)CB_newf("shoots"));
        VA_Push(expected, (Obj*)CB_newf("leaves"));
        TestUtils_test_analyzer(batch, (Analyzer*)polyanalyzer, source_text,
                                expected, "With SnowballStopFilter");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(case_folder));
        VA_Push(analyzers, INCREF(tokenizer));
        VA_Push(analyzers, INCREF(stopfilter));
        VA_Push(analyzers, INCREF(stemmer));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)CB_newf("eat"));
        VA_Push(expected, (Obj*)CB_newf("shoot"));
        VA_Push(expected, (Obj*)CB_newf("leav"));
        TestUtils_test_analyzer(batch, (Analyzer*)polyanalyzer, source_text,
                                expected, "With SnowballStemmer");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    DECREF(stemmer);
    DECREF(stopfilter);
    DECREF(tokenizer);
    DECREF(case_folder);
    DECREF(source_text);
}

static void
test_Get_Analyzers(TestBatch *batch) {
    VArray *analyzers = VA_new(0);
    PolyAnalyzer *analyzer = PolyAnalyzer_new(NULL, analyzers);
    TEST_TRUE(batch, PolyAnalyzer_Get_Analyzers(analyzer) == analyzers,
              "Get_Analyzers()");
    DECREF(analyzer);
    DECREF(analyzers);
}

void
TestPolyAnalyzer_run_tests() {
    TestBatch *batch = TestBatch_new(19);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);
    test_analysis(batch);
    test_Get_Analyzers(batch);

    DECREF(batch);
}

