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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_LUCY_TESTBATCH
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Clownfish/Test/TestFormatter.h"
#include "Clownfish/Test/TestRunner.h"
#include "Lucy/Test/Analysis/TestAnalyzer.h"
#include "Lucy/Test/Analysis/TestCaseFolder.h"
#include "Lucy/Test/Analysis/TestNormalizer.h"
#include "Lucy/Test/Analysis/TestPolyAnalyzer.h"
#include "Lucy/Test/Analysis/TestRegexTokenizer.h"
#include "Lucy/Test/Analysis/TestSnowballStemmer.h"
#include "Lucy/Test/Analysis/TestSnowballStopFilter.h"
#include "Lucy/Test/Analysis/TestStandardTokenizer.h"
#include "Lucy/Test/Highlight/TestHeatMap.h"
#include "Lucy/Test/Highlight/TestHighlighter.h"
#include "Lucy/Test/Index/TestDocWriter.h"
#include "Lucy/Test/Index/TestHighlightWriter.h"
#include "Lucy/Test/Index/TestIndexManager.h"
#include "Lucy/Test/Index/TestPolyReader.h"
#include "Lucy/Test/Index/TestPostingListWriter.h"
#include "Lucy/Test/Index/TestSegWriter.h"
#include "Lucy/Test/Index/TestSegment.h"
#include "Lucy/Test/Index/TestSnapshot.h"
#include "Lucy/Test/Index/TestTermInfo.h"
#include "Lucy/Test/Object/TestBitVector.h"
#include "Lucy/Test/Object/TestByteBuf.h"
#include "Lucy/Test/Object/TestCharBuf.h"
#include "Lucy/Test/Object/TestErr.h"
#include "Lucy/Test/Object/TestHash.h"
#include "Lucy/Test/Object/TestI32Array.h"
#include "Lucy/Test/Object/TestLockFreeRegistry.h"
#include "Lucy/Test/Object/TestNum.h"
#include "Lucy/Test/Object/TestObj.h"
#include "Lucy/Test/Object/TestVArray.h"
#include "Lucy/Test/Plan/TestBlobType.h"
#include "Lucy/Test/Plan/TestFieldMisc.h"
#include "Lucy/Test/Plan/TestFieldType.h"
#include "Lucy/Test/Plan/TestFullTextType.h"
#include "Lucy/Test/Plan/TestNumericType.h"
#include "Lucy/Test/Search/TestLeafQuery.h"
#include "Lucy/Test/Search/TestMatchAllQuery.h"
#include "Lucy/Test/Search/TestNOTQuery.h"
#include "Lucy/Test/Search/TestNoMatchQuery.h"
#include "Lucy/Test/Search/TestPhraseQuery.h"
#include "Lucy/Test/Search/TestPolyQuery.h"
#include "Lucy/Test/Search/TestQueryParserLogic.h"
#include "Lucy/Test/Search/TestRangeQuery.h"
#include "Lucy/Test/Search/TestReqOptQuery.h"
#include "Lucy/Test/Search/TestSeriesMatcher.h"
#include "Lucy/Test/Search/TestSortSpec.h"
#include "Lucy/Test/Search/TestSpan.h"
#include "Lucy/Test/Search/TestTermQuery.h"
#include "Lucy/Test/Store/TestCompoundFileReader.h"
#include "Lucy/Test/Store/TestCompoundFileWriter.h"
#include "Lucy/Test/Store/TestFSDirHandle.h"
#include "Lucy/Test/Store/TestFSFileHandle.h"
#include "Lucy/Test/Store/TestFSFolder.h"
#include "Lucy/Test/Store/TestFileHandle.h"
#include "Lucy/Test/Store/TestFolder.h"
#include "Lucy/Test/Store/TestIOChunks.h"
#include "Lucy/Test/Store/TestIOPrimitives.h"
#include "Lucy/Test/Store/TestInStream.h"
#include "Lucy/Test/Store/TestRAMDirHandle.h"
#include "Lucy/Test/Store/TestRAMFileHandle.h"
#include "Lucy/Test/Store/TestRAMFolder.h"
#include "Lucy/Test/TestSchema.h"
#include "Lucy/Test/Util/TestAtomic.h"
#include "Lucy/Test/Util/TestIndexFileNames.h"
#include "Lucy/Test/Util/TestJson.h"
#include "Lucy/Test/Util/TestMemory.h"
#include "Lucy/Test/Util/TestMemoryPool.h"
#include "Lucy/Test/Util/TestNumberUtils.h"
#include "Lucy/Test/Util/TestPriorityQueue.h"
#include "Lucy/Test/Util/TestStringHelper.h"

static bool
S_vtest_true(TestBatch *self, bool condition, const char *pattern,
             va_list args);

static VArray*
S_all_test_batches() {
    VArray *batches = VA_new(0);

    VA_Push(batches, (Obj*)TestPriQ_new());
    VA_Push(batches, (Obj*)TestBitVector_new());
    VA_Push(batches, (Obj*)TestVArray_new());
    VA_Push(batches, (Obj*)TestHash_new());
    VA_Push(batches, (Obj*)TestObj_new());
    VA_Push(batches, (Obj*)TestErr_new());
    VA_Push(batches, (Obj*)TestBB_new());
    VA_Push(batches, (Obj*)TestMemPool_new());
    VA_Push(batches, (Obj*)TestCB_new());
    VA_Push(batches, (Obj*)TestNumUtil_new());
    VA_Push(batches, (Obj*)TestNum_new());
    VA_Push(batches, (Obj*)TestStrHelp_new());
    VA_Push(batches, (Obj*)TestIxFileNames_new());
    VA_Push(batches, (Obj*)TestJson_new());
    VA_Push(batches, (Obj*)TestI32Arr_new());
    VA_Push(batches, (Obj*)TestAtomic_new());
    VA_Push(batches, (Obj*)TestLFReg_new());
    VA_Push(batches, (Obj*)TestMemory_new());
    VA_Push(batches, (Obj*)TestRAMFH_new());
    VA_Push(batches, (Obj*)TestFSFH_new());
    VA_Push(batches, (Obj*)TestInStream_new());
    VA_Push(batches, (Obj*)TestFH_new());
    VA_Push(batches, (Obj*)TestIOPrimitives_new());
    VA_Push(batches, (Obj*)TestIOChunks_new());
    VA_Push(batches, (Obj*)TestRAMDH_new());
    VA_Push(batches, (Obj*)TestFSDH_new());
    VA_Push(batches, (Obj*)TestFSFolder_new());
    VA_Push(batches, (Obj*)TestRAMFolder_new());
    VA_Push(batches, (Obj*)TestFolder_new());
    VA_Push(batches, (Obj*)TestIxManager_new());
    VA_Push(batches, (Obj*)TestCFWriter_new());
    VA_Push(batches, (Obj*)TestCFReader_new());
    VA_Push(batches, (Obj*)TestAnalyzer_new());
    VA_Push(batches, (Obj*)TestPolyAnalyzer_new());
    VA_Push(batches, (Obj*)TestCaseFolder_new());
    VA_Push(batches, (Obj*)TestRegexTokenizer_new());
    VA_Push(batches, (Obj*)TestSnowStop_new());
    VA_Push(batches, (Obj*)TestSnowStemmer_new());
    VA_Push(batches, (Obj*)TestNormalizer_new());
    VA_Push(batches, (Obj*)TestStandardTokenizer_new());
    VA_Push(batches, (Obj*)TestSnapshot_new());
    VA_Push(batches, (Obj*)TestTermInfo_new());
    VA_Push(batches, (Obj*)TestFieldMisc_new());
    VA_Push(batches, (Obj*)TestBatchSchema_new());
    VA_Push(batches, (Obj*)TestDocWriter_new());
    VA_Push(batches, (Obj*)TestHLWriter_new());
    VA_Push(batches, (Obj*)TestPListWriter_new());
    VA_Push(batches, (Obj*)TestSegWriter_new());
    VA_Push(batches, (Obj*)TestPolyReader_new());
    VA_Push(batches, (Obj*)TestFullTextType_new());
    VA_Push(batches, (Obj*)TestBlobType_new());
    VA_Push(batches, (Obj*)TestNumericType_new());
    VA_Push(batches, (Obj*)TestFType_new());
    VA_Push(batches, (Obj*)TestSeg_new());
    VA_Push(batches, (Obj*)TestHighlighter_new());
    VA_Push(batches, (Obj*)TestSpan_new());
    VA_Push(batches, (Obj*)TestHeatMap_new());
    VA_Push(batches, (Obj*)TestTermQuery_new());
    VA_Push(batches, (Obj*)TestPhraseQuery_new());
    VA_Push(batches, (Obj*)TestSortSpec_new());
    VA_Push(batches, (Obj*)TestRangeQuery_new());
    VA_Push(batches, (Obj*)TestANDQuery_new());
    VA_Push(batches, (Obj*)TestMatchAllQuery_new());
    VA_Push(batches, (Obj*)TestNOTQuery_new());
    VA_Push(batches, (Obj*)TestReqOptQuery_new());
    VA_Push(batches, (Obj*)TestLeafQuery_new());
    VA_Push(batches, (Obj*)TestNoMatchQuery_new());
    VA_Push(batches, (Obj*)TestSeriesMatcher_new());
    VA_Push(batches, (Obj*)TestORQuery_new());
    VA_Push(batches, (Obj*)TestQPLogic_new());

    return batches;
}

bool
Test_run_batch(CharBuf *class_name, TestFormatter *formatter) {
    VArray   *batches = S_all_test_batches();
    uint32_t  size    = VA_Get_Size(batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(batches, i);

        if (CB_Equals(TestBatch_Get_Class_Name(batch), (Obj*)class_name)) {
            bool result = TestBatch_Run(batch);
            DECREF(batches);
            return result;
        }
    }

    DECREF(batches);
    THROW(ERR, "Couldn't find test class '%o'", class_name);
    UNREACHABLE_RETURN(bool);
}

bool
Test_run_all_batches(TestFormatter *formatter) {
    TestRunner *runner  = TestRunner_new(formatter);
    VArray     *batches = S_all_test_batches();
    uint32_t    size    = VA_Get_Size(batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(batches, i);
        TestRunner_Run_Batch(runner, batch);
    }

    bool result = TestRunner_Finish(runner);

    DECREF(runner);
    DECREF(batches);
    return result;
}

TestBatch*
TestBatch_new(int64_t num_tests) {
    TestBatch *self = (TestBatch*)VTable_Make_Obj(TESTBATCH);
    return TestBatch_init(self, num_tests);
}

TestBatch*
TestBatch_init(TestBatch *self, int64_t num_tests) {
    // Assign.
    self->num_tests       = num_tests;

    // Initialize.
    self->formatter       = (TestFormatter*)TestFormatterTAP_new();
    self->test_num        = 0;
    self->num_passed      = 0;
    self->num_failed      = 0;
    self->num_skipped     = 0;

    // Unbuffer stdout. TODO: move this elsewhere.
    int check_val = setvbuf(stdout, NULL, _IONBF, 0);
    if (check_val != 0) {
        fprintf(stderr, "Failed when trying to unbuffer stdout\n");
    }

    return self;
}

void
TestBatch_destroy(TestBatch *self) {
    DECREF(self->formatter);
    SUPER_DESTROY(self, TESTBATCH);
}

void
TestBatch_plan(TestBatch *self) {
    TestFormatter_Batch_Prologue(self->formatter, self);
}

bool
TestBatch_run(TestBatch *self) {
    TestBatch_Plan(self);
    TestBatch_Run_Tests(self);

    bool failed = false;
    if (self->num_failed > 0) {
        failed = true;
        TestFormatter_batch_comment(self->formatter, "%d/%d tests failed.\n",
                                    self->num_failed, self->test_num);
    }
    if (self->test_num != self->num_tests) {
        failed = true;
        TestFormatter_batch_comment(self->formatter,
                                    "Bad plan: You planned %d tests but ran"
                                    " %d.\n",
                                    self->num_tests, self->test_num);
    }

    return !failed;
}

void
TestBatch_run_tests(TestBatch *self) {
}

int64_t
TestBatch_get_num_planned(TestBatch *self) {
    return self->num_tests;
}

int64_t
TestBatch_get_num_tests(TestBatch *self) {
    return self->test_num;
}

int64_t
TestBatch_get_num_failed(TestBatch *self) {
    return self->num_failed;
}

bool
TestBatch_test_true(void *vself, bool condition, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_True((TestBatch*)vself, condition,
                                         pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_false(void *vself, bool condition, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_False((TestBatch*)vself, condition,
                                          pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_int_equals(void *vself, long got, long expected,
                          const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_Int_Equals((TestBatch*)vself, got,
                                               expected, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_float_equals(void *vself, double got, double expected,
                            const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_Float_Equals((TestBatch*)vself, got,
                                                 expected, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_string_equals(void *vself, const char *got,
                             const char *expected, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_String_Equals((TestBatch*)vself, got,
                                                  expected, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_pass(void *vself, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VPass((TestBatch*)vself, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_fail(void *vself, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VFail((TestBatch*)vself, pattern, args);
    va_end(args);
    return result;
}

void
TestBatch_skip(void *vself, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    TestBatch_VSkip((TestBatch*)vself, pattern, args);
    va_end(args);
}

bool
TestBatch_vtest_true(TestBatch *self, bool condition, const char *pattern,
                     va_list args) {
    return S_vtest_true(self, condition, pattern, args);
}

bool
TestBatch_vtest_false(TestBatch *self, bool condition,
                      const char *pattern, va_list args) {
    return S_vtest_true(self, !condition, pattern, args);
}

bool
TestBatch_vtest_int_equals(TestBatch *self, long got, long expected,
                           const char *pattern, va_list args) {
    bool pass = (got == expected);
    S_vtest_true(self, pass, pattern, args);
    if (!pass) {
        TestFormatter_test_comment(self->formatter,
                                   "Expected '%ld', got '%ld'.\n",
                                   expected, got);
    }
    return pass;
}

bool
TestBatch_vtest_float_equals(TestBatch *self, double got, double expected,
                             const char *pattern, va_list args) {
    double relative_error = got / expected - 1.0;
    bool   pass           = (fabs(relative_error) < 1e-6);
    S_vtest_true(self, pass, pattern, args);
    if (!pass) {
        TestFormatter_test_comment(self->formatter,
                                   "Expected '%e', got '%e'.\n",
                                   expected, got);
    }
    return pass;
}

bool
TestBatch_vtest_string_equals(TestBatch *self, const char *got,
                              const char *expected, const char *pattern,
                              va_list args) {
    bool pass = (strcmp(got, expected) == 0);
    S_vtest_true(self, pass, pattern, args);
    if (!pass) {
        TestFormatter_test_comment(self->formatter,
                                   "Expected '%s', got '%s'.\n",
                                   expected, got);
    }
    return pass;
}

bool
TestBatch_vpass(TestBatch *self, const char *pattern, va_list args) {
    return S_vtest_true(self, true, pattern, args);
}

bool
TestBatch_vfail(TestBatch *self, const char *pattern, va_list args) {
    return S_vtest_true(self, false, pattern, args);
}

void
TestBatch_vskip(TestBatch *self, const char *pattern, va_list args) {
    self->test_num++;
    // TODO: Add a VTest_Skip method to TestFormatter
    TestFormatter_VTest_Result(self->formatter, true, self->num_tests,
                               pattern, args);
    self->num_skipped++;
}

static bool
S_vtest_true(TestBatch* self, bool condition, const char *pattern,
             va_list args) {
    // Increment test number.
    self->test_num++;

    if (condition) {
        self->num_passed++;
    }
    else {
        self->num_failed++;
    }

    TestFormatter_VTest_Result(self->formatter, condition, self->test_num,
                               pattern, args);

    return condition;
}


