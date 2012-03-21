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

#define C_LUCY_TESTTERMINFO
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestTermInfo.h"
#include "Lucy/Index/TermInfo.h"

void 
test_freqfilepos(TestBatch *batch) {
    TermInfo* tinfo = TInfo_new(10);
    TInfo_set_post_filepos(tinfo, 20);
    TInfo_set_skip_filepos(tinfo, 40);
    TInfo_set_lex_filepos(tinfo, 50);

    TermInfo* cloned_tinfo = TInfo_clone(tinfo);

    TEST_FALSE(batch, Lucy_TInfo_Equals(tinfo, (lucy_Obj*)cloned_tinfo),"the clone should be a separate C struct");
    TEST_INT_EQ(batch, TInfo_get_doc_freq(tinfo), 10, "new sets doc_freq correctly" );
    TEST_INT_EQ(batch, TInfo_get_doc_freq(tinfo), 10, "... doc_freq cloned" );
    TEST_INT_EQ(batch, TInfo_get_post_filepos(tinfo), 20, "... post_filepos cloned" );
    TEST_INT_EQ(batch, TInfo_get_skip_filepos(tinfo), 40, "... skip_filepos cloned" );
    TEST_INT_EQ(batch, TInfo_get_lex_filepos(tinfo),  50, "... lex_filepos cloned" );

    TInfo_set_doc_freq(tinfo, 5);
    TEST_INT_EQ(batch, TInfo_get_doc_freq(tinfo), 5,  "set/get doc_freq" );
    TEST_INT_EQ(batch, TInfo_get_doc_freq(cloned_tinfo), 10, "setting orig doesn't affect clone" );

    TInfo_set_post_filepos(tinfo, 15);
    TEST_INT_EQ(batch, TInfo_get_post_filepos(tinfo), 15, "set/get post_filepos" );

    TInfo_set_skip_filepos(tinfo, 35);
    TEST_INT_EQ(batch, TInfo_get_skip_filepos(tinfo), 35, "set/get skip_filepos" );

    TInfo_set_lex_filepos(tinfo, 45);
    TEST_INT_EQ(batch, TInfo_get_lex_filepos(tinfo), 45, "set/get lex_filepos" );

    DECREF(tinfo);
    DECREF(cloned_tinfo);
}

void
TestTermInfo_run_tests() {
    TestBatch *batch = TestBatch_new(11);
    TestBatch_Plan(batch);
    test_freqfilepos(batch);

    DECREF(batch);
}
