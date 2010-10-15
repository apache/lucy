#define C_KINO_SORTSPEC
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/SortSpec.h"
#include "KinoSearch/Index/IndexReader.h"
#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/SortRule.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"
#include "KinoSearch/Util/SortUtils.h"

SortSpec*
SortSpec_new(VArray *rules)
{
    SortSpec *self = (SortSpec*)VTable_Make_Obj(SORTSPEC);
    return SortSpec_init(self, rules);
}

SortSpec*
SortSpec_init(SortSpec *self, VArray *rules)
{
    int32_t i, max;
    self->rules = VA_Shallow_Copy(rules);
    for (i = 0, max = VA_Get_Size(rules); i < max; i++) {
        SortRule *rule = (SortRule*)VA_Fetch(rules, i);
        CERTIFY(rule, SORTRULE);
    }
    return self;
}

void
SortSpec_destroy(SortSpec *self)
{
    DECREF(self->rules);
    SUPER_DESTROY(self, SORTSPEC);
}

SortSpec*
SortSpec_deserialize(SortSpec *self, InStream *instream)
{
    uint32_t num_rules = InStream_Read_C32(instream);
    VArray *rules = VA_new(num_rules);
    uint32_t i;

    // Create base object. 
    self = self ? self : (SortSpec*)VTable_Make_Obj(SORTSPEC);

    // Add rules. 
    for (i = 0; i < num_rules; i++) {
        VA_Push(rules, (Obj*)SortRule_deserialize(NULL, instream));
    }
    SortSpec_init(self, rules);
    DECREF(rules);

    return self;
}

VArray*
SortSpec_get_rules(SortSpec *self) { return self->rules; }

void
SortSpec_serialize(SortSpec *self, OutStream *target)
{
    uint32_t num_rules = VA_Get_Size(self->rules);
    uint32_t i;
    OutStream_Write_C32(target, num_rules);
    for (i = 0; i < num_rules; i++) {
        SortRule *rule = (SortRule*)VA_Fetch(self->rules, i);
        SortRule_Serialize(rule, target);
    }
}


