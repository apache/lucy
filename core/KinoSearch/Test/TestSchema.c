#define C_KINO_TESTSCHEMA
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Plan/TestArchitecture.h"
#include "KinoSearch/Test/TestSchema.h"
#include "KinoSearch/Analysis/CaseFolder.h"
#include "KinoSearch/Analysis/Tokenizer.h"
#include "KinoSearch/Plan/FullTextType.h"
#include "KinoSearch/Plan/Architecture.h"

TestSchema*
TestSchema_new()
{
    TestSchema *self = (TestSchema*)VTable_Make_Obj(TESTSCHEMA);
    return TestSchema_init(self);
}

TestSchema*
TestSchema_init(TestSchema *self)
{
    Tokenizer *tokenizer = Tokenizer_new(NULL);
    FullTextType *type = FullTextType_new((Analyzer*)tokenizer);

    Schema_init((Schema*)self);
    FullTextType_Set_Highlightable(type, true);
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    TestSchema_Spec_Field(self, content, (FieldType*)type);
    DECREF(type);
    DECREF(tokenizer);

    return self;
}

Architecture*
TestSchema_architecture(TestSchema *self)
{
    UNUSED_VAR(self);
    return (Architecture*)TestArch_new();
}

static void
test_Equals(TestBatch *batch)
{
    TestSchema *schema = TestSchema_new();
    TestSchema *arch_differs = TestSchema_new();
    TestSchema *spec_differs = TestSchema_new();
    CharBuf    *content      = (CharBuf*)ZCB_WRAP_STR("content", 7);
    FullTextType *type = (FullTextType*)TestSchema_Fetch_Type(spec_differs, 
        content);
    CaseFolder *case_folder = CaseFolder_new();

    TEST_TRUE(batch, TestSchema_Equals(schema, (Obj*)schema), "Equals");

    FullTextType_Set_Boost(type, 2.0f);
    TEST_FALSE(batch, TestSchema_Equals(schema, (Obj*)spec_differs), 
        "Equals spoiled by differing FieldType");

    DECREF(arch_differs->arch);
    arch_differs->arch = Arch_new();
    TEST_FALSE(batch, TestSchema_Equals(schema, (Obj*)arch_differs), 
        "Equals spoiled by differing Architecture");

    DECREF(schema);
    DECREF(arch_differs);
    DECREF(spec_differs);
    DECREF(case_folder);
}

static void
test_Dump_and_Load(TestBatch *batch)
{
    TestSchema *schema = TestSchema_new();
    Obj *dump = (Obj*)TestSchema_Dump(schema);
    TestSchema *loaded = (TestSchema*)Obj_Load(dump, dump);

    TEST_FALSE(batch, TestSchema_Equals(schema, (Obj*)loaded), 
        "Dump => Load round trip");

    DECREF(schema);
    DECREF(dump);
    DECREF(loaded);
}

void
TestSchema_run_tests()
{
    TestBatch *batch = TestBatch_new(4);
    TestBatch_Plan(batch);
    test_Equals(batch);
    test_Dump_and_Load(batch);
    DECREF(batch);
}


