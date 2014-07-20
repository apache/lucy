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

#define C_TESTLUCY_TESTTERMINFO
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestTermInfo.h"
#include "Lucy/Index/TermInfo.h"

TestTermInfo*
TestTermInfo_new() {
    return (TestTermInfo*)Class_Make_Obj(TESTTERMINFO);
}

void 
test_freqfilepos(TestBatchRunner *runner) {
    TermInfo* tinfo = TInfo_new(10);
    TInfo_Set_Post_FilePos(tinfo, 20);
    TInfo_Set_Skip_FilePos(tinfo, 40);
    TInfo_Set_Lex_FilePos(tinfo, 50);

    TermInfo* cloned_tinfo = TInfo_Clone(tinfo);

    TEST_FALSE(runner, LUCY_TInfo_Equals(tinfo, (Obj*)cloned_tinfo),"the clone should be a separate C struct");
    TEST_INT_EQ(runner, TInfo_Get_Doc_Freq(tinfo), 10, "new sets doc_freq correctly" );
    TEST_INT_EQ(runner, TInfo_Get_Doc_Freq(tinfo), 10, "... doc_freq cloned" );
    TEST_INT_EQ(runner, (int)TInfo_Get_Post_FilePos(tinfo), 20, "... post_filepos cloned" );
    TEST_INT_EQ(runner, (int)TInfo_Get_Skip_FilePos(tinfo), 40, "... skip_filepos cloned" );
    TEST_INT_EQ(runner, (int)TInfo_Get_Lex_FilePos(tinfo),  50, "... lex_filepos cloned" );

    TInfo_Set_Doc_Freq(tinfo, 5);
    TEST_INT_EQ(runner, TInfo_Get_Doc_Freq(tinfo), 5,  "set/get doc_freq" );
    TEST_INT_EQ(runner, TInfo_Get_Doc_Freq(cloned_tinfo), 10, "setting orig doesn't affect clone" );

    TInfo_Set_Post_FilePos(tinfo, 15);
    TEST_INT_EQ(runner, (int)TInfo_Get_Post_FilePos(tinfo), 15, "set/get post_filepos" );

    TInfo_Set_Skip_FilePos(tinfo, 35);
    TEST_INT_EQ(runner, (int)TInfo_Get_Skip_FilePos(tinfo), 35, "set/get skip_filepos" );

    TInfo_Set_Lex_FilePos(tinfo, 45);
    TEST_INT_EQ(runner, (int)TInfo_Get_Lex_FilePos(tinfo), 45, "set/get lex_filepos" );

    DECREF(tinfo);
    DECREF(cloned_tinfo);
}

void
TestTermInfo_Run_IMP(TestTermInfo *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 11);
    test_freqfilepos(runner);
}
