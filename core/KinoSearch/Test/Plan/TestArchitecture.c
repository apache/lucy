#define C_KINO_TESTARCHITECTURE
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Plan/TestArchitecture.h"
#include "KinoSearch/Plan/Architecture.h"

TestArchitecture*
TestArch_new()
{
    TestArchitecture *self 
        = (TestArchitecture*)VTable_Make_Obj(TESTARCHITECTURE);
    return TestArch_init(self);
}

TestArchitecture*
TestArch_init(TestArchitecture *self)
{
    Arch_init((Architecture*)self);
    return self;
}

int32_t
TestArch_index_interval(TestArchitecture *self)
{
    UNUSED_VAR(self);
    return 5;
}

int32_t
TestArch_skip_interval(TestArchitecture *self)
{
    UNUSED_VAR(self);
    return 3;
}


