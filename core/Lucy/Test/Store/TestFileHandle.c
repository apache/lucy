#define C_LUCY_TESTINSTREAM
#define C_LUCY_INSTREAM
#define C_LUCY_FILEWINDOW
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFileHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/FileWindow.h"

static void
S_no_op_method(const void *vself)
{
    UNUSED_VAR(vself);
}

static FileHandle*
S_new_filehandle()
{
    static ZombieCharBuf klass = ZCB_LITERAL("TestFileHandle");
    FileHandle *fh;
    VTable *vtable = VTable_fetch_vtable((CharBuf*)&klass);
    if (!vtable) {
        vtable = VTable_singleton((CharBuf*)&klass, FILEHANDLE);
    }   
    VTable_Override(vtable, S_no_op_method, Lucy_FH_Close_OFFSET);
    fh = (FileHandle*)VTable_Make_Obj(vtable);
    return FH_do_open(fh, NULL, 0);
}

void
TestFH_run_tests()
{
    TestBatch     *batch  = Test_new_batch("TestFileHandle", 2, NULL);
    FileHandle    *fh     = S_new_filehandle();
    ZombieCharBuf  foo    = ZCB_LITERAL("foo");

    PLAN(batch);

    ASSERT_TRUE(batch, CB_Equals_Str(FH_Get_Path(fh), "", 0), "Get_Path");
    FH_Set_Path(fh, (CharBuf*)&foo);
    ASSERT_TRUE(batch, CB_Equals(FH_Get_Path(fh), (Obj*)&foo), "Set_Path");

    DECREF(fh);
    batch->destroy(batch);
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

