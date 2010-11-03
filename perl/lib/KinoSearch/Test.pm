# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package KinoSearch::Test;
use KinoSearch;

# Set the default memory threshold for PostingListWriter to a low number so
# that we simulate large indexes by performing a lot of PostingPool flushes.
KinoSearch::Index::PostingListWriter::set_default_mem_thresh(0x1000);

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Test::TestUtils

SV*
doc_set()
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(kino_TestUtils_doc_set());
OUTPUT: RETVAL

MODULE = KinoSearch   PACKAGE = KinoSearch::Test

void
run_tests(package)
    char *package;
PPCODE:
{
    // Lucy::Analysis 
    if (strEQ(package, "TestCaseFolder")) {
        kino_TestCaseFolder_run_tests();
    }
    else if (strEQ(package, "TestPolyAnalyzer")) {
        kino_TestPolyAnalyzer_run_tests();
    }
    else if (strEQ(package, "TestStopalizer")) {
        kino_TestStopalizer_run_tests();
    }
    else if (strEQ(package, "TestStemmer")) {
        kino_TestStemmer_run_tests();
    }
    else if (strEQ(package, "TestTokenizer")) {
        kino_TestTokenizer_run_tests();
    }
    // Lucy::Object 
    else if (strEQ(package, "TestObj")) {
        kino_TestObj_run_tests();
    }
    else if (strEQ(package, "TestI32Array")) {
        kino_TestI32Arr_run_tests();
    }
    else if (strEQ(package, "TestByteBuf")) {
        kino_TestBB_run_tests();
    }
    else if (strEQ(package, "TestLockFreeRegistry")) {
        kino_TestLFReg_run_tests();
    }
    // Lucy::Plan 
    else if (strEQ(package, "TestBlobType")) {
        kino_TestBlobType_run_tests();
    }
    else if (strEQ(package, "TestFullTextType")) {
        kino_TestFullTextType_run_tests();
    }
    else if (strEQ(package, "TestNumericType")) {
        kino_TestNumericType_run_tests();
    }
    else if (strEQ(package, "TestSchema")) {
        kino_TestSchema_run_tests();
    }
    // Lucy::Index 
    else if (strEQ(package, "TestDocWriter")) {
        kino_TestDocWriter_run_tests();
    }
    else if (strEQ(package, "TestHighlightWriter")) {
        kino_TestHLWriter_run_tests();
    }
    else if (strEQ(package, "TestIndexManager")) {
        kino_TestIxManager_run_tests();
    }
    else if (strEQ(package, "TestPostingListWriter")) {
        kino_TestPListWriter_run_tests();
    }
    else if (strEQ(package, "TestSegment")) {
        kino_TestSeg_run_tests();
    }
    else if (strEQ(package, "TestSegWriter")) {
        kino_TestSegWriter_run_tests();
    }
    else if (strEQ(package, "TestSnapshot")) {
        kino_TestSnapshot_run_tests();
    }
    // Lucy::Search 
    else if (strEQ(package, "TestANDQuery")) {
        kino_TestANDQuery_run_tests();
    }
    else if (strEQ(package, "TestLeafQuery")) {
        kino_TestLeafQuery_run_tests();
    }
    else if (strEQ(package, "TestMatchAllQuery")) {
        kino_TestMatchAllQuery_run_tests();
    }
    else if (strEQ(package, "TestNoMatchQuery")) {
        kino_TestNoMatchQuery_run_tests();
    }
    else if (strEQ(package, "TestNOTQuery")) {
        kino_TestNOTQuery_run_tests();
    }
    else if (strEQ(package, "TestORQuery")) {
        kino_TestORQuery_run_tests();
    }
    else if (strEQ(package, "TestPhraseQuery")) {
        kino_TestPhraseQuery_run_tests();
    }
    else if (strEQ(package, "TestQueryParserLogic")) {
        kino_TestQPLogic_run_tests();
    }
    else if (strEQ(package, "TestSeriesMatcher")) {
        kino_TestSeriesMatcher_run_tests();
    }
    else if (strEQ(package, "TestRangeQuery")) {
        kino_TestRangeQuery_run_tests();
    }
    else if (strEQ(package, "TestReqOptQuery")) {
        kino_TestReqOptQuery_run_tests();
    }
    else if (strEQ(package, "TestTermQuery")) {
        kino_TestTermQuery_run_tests();
    }
    // Lucy::Store 
    else if (strEQ(package, "TestCompoundFileReader")) {
        kino_TestCFReader_run_tests();
    }
    else if (strEQ(package, "TestCompoundFileWriter")) {
        kino_TestCFWriter_run_tests();
    }
    else if (strEQ(package, "TestFileHandle")) {
        kino_TestFH_run_tests();
    }
    else if (strEQ(package, "TestFolder")) {
        kino_TestFolder_run_tests();
    }
    else if (strEQ(package, "TestFSDirHandle")) {
        kino_TestFSDH_run_tests();
    }
    else if (strEQ(package, "TestFSFolder")) {
        kino_TestFSFolder_run_tests();
    }
    else if (strEQ(package, "TestFSFileHandle")) {
        kino_TestFSFH_run_tests();
    }
    else if (strEQ(package, "TestInStream")) {
        kino_TestInStream_run_tests();
    }
    else if (strEQ(package, "TestIOChunks")) {
        kino_TestIOChunks_run_tests();
    }
    else if (strEQ(package, "TestIOPrimitives")) {
        kino_TestIOPrimitives_run_tests();
    }
    else if (strEQ(package, "TestRAMDirHandle")) {
        kino_TestRAMDH_run_tests();
    }
    else if (strEQ(package, "TestRAMFileHandle")) {
        kino_TestRAMFH_run_tests();
    }
    else if (strEQ(package, "TestRAMFolder")) {
        kino_TestRAMFolder_run_tests();
    }
    // Lucy::Util 
    else if (strEQ(package, "TestAtomic")) {
        kino_TestAtomic_run_tests();
    }
    else if (strEQ(package, "TestBitVector")) {
        kino_TestBitVector_run_tests();
    }
    else if (strEQ(package, "TestCharBuf")) {
        kino_TestCB_run_tests();
    }
    else if (strEQ(package, "TestHash")) {
        kino_TestHash_run_tests();
    }
    else if (strEQ(package, "TestJson")) {
        kino_TestJson_run_tests();
    }
    else if (strEQ(package, "TestMemory")) {
        kino_TestMemory_run_tests();
    }
    else if (strEQ(package, "TestIndexFileNames")) {
        kino_TestIxFileNames_run_tests();
    }
    else if (strEQ(package, "TestNumberUtils")) {
        kino_TestNumUtil_run_tests();
    }
    else if (strEQ(package, "TestNum")) {
        kino_TestNum_run_tests();
    }
    else if (strEQ(package, "TestPriorityQueue")) {
        kino_TestPriQ_run_tests();
    }
    else if (strEQ(package, "TestStringHelper")) {
        kino_TestStrHelp_run_tests();
    }
    else if (strEQ(package, "TestMemoryPool")) {
        kino_TestMemPool_run_tests();
    }
    else if (strEQ(package, "TestVArray")) {
        kino_TestVArray_run_tests();
    }
    else {
        THROW(KINO_ERR, "Unknown test id: %s", package);
    }
}

MODULE = KinoSearch   PACKAGE = KinoSearch::Test::TestQueryParserSyntax

void
run_tests(index);
    kino_Folder *index;
PPCODE:
    kino_TestQPSyntax_run_tests(index);
END_XS_CODE

my $charm_xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Test::TestCharmonizer

void
run_tests(which)
    char *which;
PPCODE:
{
    chaz_TestBatch *batch = NULL;
    chaz_Test_init();

    if (strcmp(which, "dirmanip") == 0) {
        batch = chaz_TestDirManip_prepare();
    }
    else if (strcmp(which, "integers") == 0) {
        batch = chaz_TestIntegers_prepare();
    }
    else if (strcmp(which, "func_macro") == 0) {
        batch = chaz_TestFuncMacro_prepare();
    }
    else if (strcmp(which, "headers") == 0) {
        batch = chaz_TestHeaders_prepare();
    }
    else if (strcmp(which, "large_files") == 0) {
        batch = chaz_TestLargeFiles_prepare();
    }
    else if (strcmp(which, "unused_vars") == 0) {
        batch = chaz_TestUnusedVars_prepare();
    }
    else if (strcmp(which, "variadic_macros") == 0) {
        batch = chaz_TestVariadicMacros_prepare();
    }
    else {
        THROW(KINO_ERR, "Unknown test identifier: '%s'", which);
    }

    batch->run_test(batch);
    batch->destroy(batch);
}
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Test::TestSchema",
    bind_constructors => ["new"],
);

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Test",
    xs_code           => $xs_code,
);

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Test::TestCharmonizer",
    xs_code           => $charm_xs_code,
);


