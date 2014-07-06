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

#define C_TESTLUCY_TESTPOLYANALYZER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Analysis/TestPolyAnalyzer.h"
#include "Lucy/Analysis/PolyAnalyzer.h"
#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Analysis/RegexTokenizer.h"
#include "Lucy/Analysis/SnowballStopFilter.h"
#include "Lucy/Analysis/SnowballStemmer.h"
#include "Lucy/Analysis/StandardTokenizer.h"

TestPolyAnalyzer*
TestPolyAnalyzer_new() {
    return (TestPolyAnalyzer*)Class_Make_Obj(TESTPOLYANALYZER);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    if (!RegexTokenizer_is_available()) {
        SKIP(runner, "RegexTokenizer not available");
        SKIP(runner, "RegexTokenizer not available");
        SKIP(runner, "RegexTokenizer not available");
        return;
    }

    String       *EN          = (String*)SSTR_WRAP_UTF8("en", 2);
    String       *ES          = (String*)SSTR_WRAP_UTF8("es", 2);
    PolyAnalyzer *analyzer    = PolyAnalyzer_new(EN, NULL);
    PolyAnalyzer *other       = PolyAnalyzer_new(ES, NULL);
    Obj          *dump        = (Obj*)PolyAnalyzer_Dump(analyzer);
    Obj          *other_dump  = (Obj*)PolyAnalyzer_Dump(other);
    PolyAnalyzer *clone       = (PolyAnalyzer*)PolyAnalyzer_Load(other, dump);
    PolyAnalyzer *other_clone
        = (PolyAnalyzer*)PolyAnalyzer_Load(other, other_dump);

    TEST_FALSE(runner, PolyAnalyzer_Equals(analyzer, (Obj*)other),
               "Equals() false with different language");
    TEST_TRUE(runner, PolyAnalyzer_Equals(analyzer, (Obj*)clone),
              "Dump => Load round trip");
    TEST_TRUE(runner, PolyAnalyzer_Equals(other, (Obj*)other_clone),
              "Dump => Load round trip");

    DECREF(analyzer);
    DECREF(dump);
    DECREF(clone);
    DECREF(other);
    DECREF(other_dump);
    DECREF(other_clone);
}

static void
test_analysis(TestBatchRunner *runner) {
    String             *EN          = (String*)SSTR_WRAP_UTF8("en", 2);
    String             *source_text = Str_newf("Eats, shoots and leaves.");
    Normalizer         *normalizer  = Normalizer_new(NULL, true, false);
    StandardTokenizer  *tokenizer   = StandardTokenizer_new();
    SnowballStopFilter *stopfilter  = SnowStop_new(EN, NULL);
    SnowballStemmer    *stemmer     = SnowStemmer_new(EN);

    {
        VArray       *analyzers    = VA_new(0);
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, INCREF(source_text));
        TestUtils_test_analyzer(runner, (Analyzer*)polyanalyzer, source_text,
                                expected, "No sub analyzers");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(normalizer));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)Str_newf("eats, shoots and leaves."));
        TestUtils_test_analyzer(runner, (Analyzer*)polyanalyzer, source_text,
                                expected, "With Normalizer");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(normalizer));
        VA_Push(analyzers, INCREF(tokenizer));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)Str_newf("eats"));
        VA_Push(expected, (Obj*)Str_newf("shoots"));
        VA_Push(expected, (Obj*)Str_newf("and"));
        VA_Push(expected, (Obj*)Str_newf("leaves"));
        TestUtils_test_analyzer(runner, (Analyzer*)polyanalyzer, source_text,
                                expected, "With StandardTokenizer");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(normalizer));
        VA_Push(analyzers, INCREF(tokenizer));
        VA_Push(analyzers, INCREF(stopfilter));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)Str_newf("eats"));
        VA_Push(expected, (Obj*)Str_newf("shoots"));
        VA_Push(expected, (Obj*)Str_newf("leaves"));
        TestUtils_test_analyzer(runner, (Analyzer*)polyanalyzer, source_text,
                                expected, "With SnowballStopFilter");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    {
        VArray       *analyzers    = VA_new(0);
        VA_Push(analyzers, INCREF(normalizer));
        VA_Push(analyzers, INCREF(tokenizer));
        VA_Push(analyzers, INCREF(stopfilter));
        VA_Push(analyzers, INCREF(stemmer));
        PolyAnalyzer *polyanalyzer = PolyAnalyzer_new(NULL, analyzers);
        VArray       *expected     = VA_new(1);
        VA_Push(expected, (Obj*)Str_newf("eat"));
        VA_Push(expected, (Obj*)Str_newf("shoot"));
        VA_Push(expected, (Obj*)Str_newf("leav"));
        TestUtils_test_analyzer(runner, (Analyzer*)polyanalyzer, source_text,
                                expected, "With SnowballStemmer");
        DECREF(expected);
        DECREF(polyanalyzer);
        DECREF(analyzers);
    }

    DECREF(stemmer);
    DECREF(stopfilter);
    DECREF(tokenizer);
    DECREF(normalizer);
    DECREF(source_text);
}

static void
test_Get_Analyzers(TestBatchRunner *runner) {
    VArray *analyzers = VA_new(0);
    PolyAnalyzer *analyzer = PolyAnalyzer_new(NULL, analyzers);
    TEST_TRUE(runner, PolyAnalyzer_Get_Analyzers(analyzer) == analyzers,
              "Get_Analyzers()");
    DECREF(analyzer);
    DECREF(analyzers);
}

void
TestPolyAnalyzer_Run_IMP(TestPolyAnalyzer *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 19);
    test_Dump_Load_and_Equals(runner);
    test_analysis(runner);
    test_Get_Analyzers(runner);
}

