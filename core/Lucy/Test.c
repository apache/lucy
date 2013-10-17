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

#define CFISH_USE_SHORT_NAMES
#define TESTLUCY_USE_SHORT_NAMES

#include "Lucy/Test.h"

#include "Clownfish/TestHarness/TestBatch.h"
#include "Clownfish/TestHarness/TestSuite.h"

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
#include "Lucy/Test/Index/TestSortWriter.h"
#include "Lucy/Test/Index/TestTermInfo.h"
#include "Lucy/Test/Object/TestBitVector.h"
#include "Lucy/Test/Object/TestI32Array.h"
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
#include "Lucy/Test/Search/TestQueryParserSyntax.h"
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
#include "Lucy/Test/Util/TestIndexFileNames.h"
#include "Lucy/Test/Util/TestJson.h"
#include "Lucy/Test/Util/TestMemoryPool.h"
#include "Lucy/Test/Util/TestPriorityQueue.h"
#include "Lucy/Test/Util/TestSortExternal.h"

TestSuite*
Test_create_test_suite() {
    TestSuite *suite = TestSuite_new();

    TestSuite_Add_Batch(suite, (TestBatch*)TestPriQ_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestBitVector_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSortExternal_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestMemPool_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestIxFileNames_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestJson_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestI32Arr_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestRAMFH_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFSFH_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestInStream_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFH_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestIOPrimitives_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestIOChunks_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestRAMDH_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFSDH_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFSFolder_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestRAMFolder_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFolder_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestIxManager_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestCFWriter_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestCFReader_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestAnalyzer_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestPolyAnalyzer_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestCaseFolder_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestRegexTokenizer_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSnowStop_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSnowStemmer_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestNormalizer_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestStandardTokenizer_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSnapshot_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestTermInfo_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFieldMisc_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestBatchSchema_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestDocWriter_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestHLWriter_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestPListWriter_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSegWriter_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSortWriter_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestPolyReader_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFullTextType_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestBlobType_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestNumericType_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestFType_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSeg_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestHighlighter_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSpan_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestHeatMap_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestTermQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestPhraseQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSortSpec_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestRangeQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestANDQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestMatchAllQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestNOTQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestReqOptQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestLeafQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestNoMatchQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestSeriesMatcher_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestORQuery_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestQPLogic_new());
    TestSuite_Add_Batch(suite, (TestBatch*)TestQPSyntax_new());

    return suite;
}


