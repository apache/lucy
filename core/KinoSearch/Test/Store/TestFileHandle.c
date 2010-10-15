#define C_KINO_TESTINSTREAM
#define C_KINO_INSTREAM
#define C_KINO_FILEWINDOW
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Store/TestFileHandle.h"
#include "KinoSearch/Store/FileHandle.h"
#include "KinoSearch/Store/FileWindow.h"

static void
S_no_op_method(const void *vself)
{
    UNUSED_VAR(vself);
}

static FileHandle*
S_new_filehandle()
{
    ZombieCharBuf *klass = ZCB_WRAP_STR("TestFileHandle", 14);
    FileHandle *fh;
    VTable *vtable = VTable_fetch_vtable((CharBuf*)klass);
    if (!vtable) {
        vtable = VTable_singleton((CharBuf*)klass, FILEHANDLE);
    }   
    VTable_Override(vtable, S_no_op_method, Kino_FH_Close_OFFSET);
    fh = (FileHandle*)VTable_Make_Obj(vtable);
    return FH_do_open(fh, NULL, 0);
}

void
TestFH_run_tests()
{
    TestBatch     *batch  = TestBatch_new(2);
    FileHandle    *fh     = S_new_filehandle();
    ZombieCharBuf *foo    = ZCB_WRAP_STR("foo", 3);

    TestBatch_Plan(batch);

    TEST_TRUE(batch, CB_Equals_Str(FH_Get_Path(fh), "", 0), "Get_Path");
    FH_Set_Path(fh, (CharBuf*)foo);
    TEST_TRUE(batch, CB_Equals(FH_Get_Path(fh), (Obj*)foo), "Set_Path");

    DECREF(fh);
    DECREF(batch);
}


