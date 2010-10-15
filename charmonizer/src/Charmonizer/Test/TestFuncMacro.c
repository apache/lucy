#define CHAZ_USE_SHORT_NAMES

#include "charmony.h"
#include <string.h>
#include "Charmonizer/Test.h"
#include "Charmonizer/Test/AllTests.h"

TestBatch*
TestFuncMacro_prepare()
{
    return Test_new_batch("FuncMacro", 4, TestFuncMacro_run);
}

#ifdef INLINE
static INLINE char* S_inline_function()
{
    return "inline works";
}
#endif

void
TestFuncMacro_run(TestBatch *batch)
{

#ifdef HAS_FUNC_MACRO
    TEST_STR_EQ(batch, FUNC_MACRO, "chaz_TestFuncMacro_run", 
        "FUNC_MACRO");
#else
    SKIP(batch, "no FUNC_MACRO");
#endif

#ifdef HAS_ISO_FUNC_MACRO
    TEST_STR_EQ(batch, __func__, "chaz_TestFuncMacro_run",
        "HAS_ISO_FUNC_MACRO");
#else
    SKIP(batch, "no ISO_FUNC_MACRO");
#endif

#ifdef HAS_GNUC_FUNC_MACRO
    TEST_STR_EQ(batch, __FUNCTION__, "chaz_TestFuncMacro_run", 
        "HAS_GNUC_FUNC_MACRO");
#else
    SKIP(batch, "no GNUC_FUNC_MACRO");
#endif

#ifdef INLINE
    PASS(batch, S_inline_function());
#else
    SKIP(batch, "no INLINE functions");
#endif
}


