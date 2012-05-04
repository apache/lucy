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

package Lucy::Test;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

# Set the default memory threshold for PostingListWriter to a low number so
# that we simulate large indexes by performing a lot of PostingPool flushes.
Lucy::Index::PostingListWriter::set_default_mem_thresh(0x1000);

package Lucy::Test::TestCharmonizer;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;
use Config;
use File::Spec::Functions qw( catfile updir );

sub run_tests {
    my $name = ucfirst shift;
    $name =~ s/_([a-z])/\u$1/g;
    my $path = catfile( 'charmonizer', "Test$name$Config{_exe}" );
    if ( !-e $path ) {
        $path = catfile( updir(), $path );
    }
    exec $path;
}

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Test::TestUtils

SV*
doc_set()
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(lucy_TestUtils_doc_set());
OUTPUT: RETVAL

MODULE = Lucy   PACKAGE = Lucy::Test

void
run_tests(package)
    char *package;
PPCODE:
{
    // Lucy::Analysis
    if (strEQ(package, "TestAnalyzer")) {
        lucy_TestAnalyzer_run_tests();
    }
    else if (strEQ(package, "TestCaseFolder")) {
        lucy_TestCaseFolder_run_tests();
    }
    else if (strEQ(package, "TestPolyAnalyzer")) {
        lucy_TestPolyAnalyzer_run_tests();
    }
    else if (strEQ(package, "TestSnowballStopFilter")) {
        lucy_TestSnowStop_run_tests();
    }
    else if (strEQ(package, "TestSnowStemmer")) {
        lucy_TestSnowStemmer_run_tests();
    }
    else if (strEQ(package, "TestNormalizer")) {
        lucy_TestNormalizer_run_tests();
    }
    else if (strEQ(package, "TestRegexTokenizer")) {
        lucy_TestRegexTokenizer_run_tests();
    }
    else if (strEQ(package, "TestStandardTokenizer")) {
        lucy_TestStandardTokenizer_run_tests();
    }
    // Lucy::Object
    else if (strEQ(package, "TestObj")) {
        lucy_TestObj_run_tests();
    }
    else if (strEQ(package, "TestI32Array")) {
        lucy_TestI32Arr_run_tests();
    }
    else if (strEQ(package, "TestByteBuf")) {
        lucy_TestBB_run_tests();
    }
    else if (strEQ(package, "TestLockFreeRegistry")) {
        lucy_TestLFReg_run_tests();
    }
    // Lucy::Plan
    else if (strEQ(package, "TestBlobType")) {
        lucy_TestBlobType_run_tests();
    }
    else if (strEQ(package, "TestFieldType")) {
        lucy_TestFType_run_tests();
    }
    else if (strEQ(package, "TestFullTextType")) {
        lucy_TestFullTextType_run_tests();
    }
    else if (strEQ(package, "TestNumericType")) {
        lucy_TestNumericType_run_tests();
    }
    else if (strEQ(package, "TestSchema")) {
        lucy_TestSchema_run_tests();
    }
    // Lucy::Index
    else if (strEQ(package, "TestDocWriter")) {
        lucy_TestDocWriter_run_tests();
    }
    else if (strEQ(package, "TestHighlightWriter")) {
        lucy_TestHLWriter_run_tests();
    }
    else if (strEQ(package, "TestIndexManager")) {
        lucy_TestIxManager_run_tests();
    }
    else if (strEQ(package, "TestPolyReader")) {
        lucy_TestPolyReader_run_tests();
    }
    else if (strEQ(package, "TestPostingListWriter")) {
        lucy_TestPListWriter_run_tests();
    }
    else if (strEQ(package, "TestSegment")) {
        lucy_TestSeg_run_tests();
    }
    else if (strEQ(package, "TestSegWriter")) {
        lucy_TestSegWriter_run_tests();
    }
    else if (strEQ(package, "TestSnapshot")) {
        lucy_TestSnapshot_run_tests();
    }
    // Lucy::Search
    else if (strEQ(package, "TestANDQuery")) {
        lucy_TestANDQuery_run_tests();
    }
    else if (strEQ(package, "TestLeafQuery")) {
        lucy_TestLeafQuery_run_tests();
    }
    else if (strEQ(package, "TestMatchAllQuery")) {
        lucy_TestMatchAllQuery_run_tests();
    }
    else if (strEQ(package, "TestNoMatchQuery")) {
        lucy_TestNoMatchQuery_run_tests();
    }
    else if (strEQ(package, "TestNOTQuery")) {
        lucy_TestNOTQuery_run_tests();
    }
    else if (strEQ(package, "TestORQuery")) {
        lucy_TestORQuery_run_tests();
    }
    else if (strEQ(package, "TestPhraseQuery")) {
        lucy_TestPhraseQuery_run_tests();
    }
    else if (strEQ(package, "TestQueryParserLogic")) {
        lucy_TestQPLogic_run_tests();
    }
    else if (strEQ(package, "TestSeriesMatcher")) {
        lucy_TestSeriesMatcher_run_tests();
    }
    else if (strEQ(package, "TestRangeQuery")) {
        lucy_TestRangeQuery_run_tests();
    }
    else if (strEQ(package, "TestReqOptQuery")) {
        lucy_TestReqOptQuery_run_tests();
    }
    else if (strEQ(package, "TestTermQuery")) {
        lucy_TestTermQuery_run_tests();
    }
    // Lucy::Store
    else if (strEQ(package, "TestCompoundFileReader")) {
        lucy_TestCFReader_run_tests();
    }
    else if (strEQ(package, "TestCompoundFileWriter")) {
        lucy_TestCFWriter_run_tests();
    }
    else if (strEQ(package, "TestFileHandle")) {
        lucy_TestFH_run_tests();
    }
    else if (strEQ(package, "TestFolder")) {
        lucy_TestFolder_run_tests();
    }
    else if (strEQ(package, "TestFSDirHandle")) {
        lucy_TestFSDH_run_tests();
    }
    else if (strEQ(package, "TestFSFolder")) {
        lucy_TestFSFolder_run_tests();
    }
    else if (strEQ(package, "TestFSFileHandle")) {
        lucy_TestFSFH_run_tests();
    }
    else if (strEQ(package, "TestInStream")) {
        lucy_TestInStream_run_tests();
    }
    else if (strEQ(package, "TestIOChunks")) {
        lucy_TestIOChunks_run_tests();
    }
    else if (strEQ(package, "TestIOPrimitives")) {
        lucy_TestIOPrimitives_run_tests();
    }
    else if (strEQ(package, "TestRAMDirHandle")) {
        lucy_TestRAMDH_run_tests();
    }
    else if (strEQ(package, "TestRAMFileHandle")) {
        lucy_TestRAMFH_run_tests();
    }
    else if (strEQ(package, "TestRAMFolder")) {
        lucy_TestRAMFolder_run_tests();
    }
    // Lucy::Util
    else if (strEQ(package, "TestAtomic")) {
        lucy_TestAtomic_run_tests();
    }
    else if (strEQ(package, "TestBitVector")) {
        lucy_TestBitVector_run_tests();
    }
    else if (strEQ(package, "TestCharBuf")) {
        lucy_TestCB_run_tests();
    }
    else if (strEQ(package, "TestHash")) {
        lucy_TestHash_run_tests();
    }
    else if (strEQ(package, "TestJson")) {
        lucy_TestJson_run_tests();
    }
    else if (strEQ(package, "TestMemory")) {
        lucy_TestMemory_run_tests();
    }
    else if (strEQ(package, "TestIndexFileNames")) {
        lucy_TestIxFileNames_run_tests();
    }
    else if (strEQ(package, "TestNumberUtils")) {
        lucy_TestNumUtil_run_tests();
    }
    else if (strEQ(package, "TestNum")) {
        lucy_TestNum_run_tests();
    }
    else if (strEQ(package, "TestPriorityQueue")) {
        lucy_TestPriQ_run_tests();
    }
    else if (strEQ(package, "TestStringHelper")) {
        lucy_TestStrHelp_run_tests();
    }
    else if (strEQ(package, "TestMemoryPool")) {
        lucy_TestMemPool_run_tests();
    }
    else if (strEQ(package, "TestVArray")) {
        lucy_TestVArray_run_tests();
    }
    // Lucy::Highlight
    else if (strEQ(package, "TestHighlighter")) {
        lucy_TestHighlighter_run_tests();
    }
    else {
        THROW(LUCY_ERR, "Unknown test id: %s", package);
    }
}

MODULE = Lucy   PACKAGE = Lucy::Test::Search::TestQueryParserSyntax

void
run_tests(index);
    lucy_Folder *index;
PPCODE:
    lucy_TestQPSyntax_run_tests(index);
END_XS_CODE

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Test::TestSchema",
    bind_constructors => ["new"],
);

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Test",
    xs_code           => $xs_code,
);

