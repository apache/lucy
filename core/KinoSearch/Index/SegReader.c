#define C_KINO_SEGREADER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Index/DeletionsReader.h"
#include "KinoSearch/Index/DocReader.h"
#include "KinoSearch/Index/DocVector.h"
#include "KinoSearch/Index/Segment.h"
#include "KinoSearch/Index/Snapshot.h"
#include "KinoSearch/Plan/Architecture.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/Matcher.h"
#include "KinoSearch/Store/Folder.h"

SegReader*
SegReader_new(Schema *schema, Folder *folder, Snapshot *snapshot, 
              VArray *segments, int32_t seg_tick)
{
    SegReader *self = (SegReader*)VTable_Make_Obj(SEGREADER);
    return SegReader_init(self, schema, folder, snapshot, segments, seg_tick);
}

SegReader*
SegReader_init(SegReader *self, Schema *schema, Folder *folder,
               Snapshot *snapshot, VArray *segments, int32_t seg_tick)
{
    CharBuf *mess;
    Segment *segment;

    IxReader_init((IndexReader*)self, schema, folder, snapshot, segments,
        seg_tick, NULL);
    segment = SegReader_Get_Segment(self);

    self->doc_max    = (int32_t)Seg_Get_Count(segment);
    self->seg_name   = (CharBuf*)INCREF(Seg_Get_Name(segment));
    self->seg_num    = Seg_Get_Number(segment);
    mess = SegReader_Try_Init_Components(self);
    if (mess) {
        // An error occurred, so clean up self and throw an exception. 
        DECREF(self);
        Err_throw_mess(ERR, mess);
    }
    {
        DeletionsReader *del_reader = (DeletionsReader*)Hash_Fetch(
            self->components, (Obj*)VTable_Get_Name(DELETIONSREADER));
        self->del_count = del_reader ? DelReader_Del_Count(del_reader) : 0;
    }
    return self;
}

void
SegReader_destroy(SegReader *self)
{
    DECREF(self->seg_name);
    SUPER_DESTROY(self, SEGREADER);
}

void
SegReader_register(SegReader *self, const CharBuf *api, DataReader *component)
{
    if (Hash_Fetch(self->components, (Obj*)api)) {
        THROW(ERR, "Interface '%o' already registered");
    }
    CERTIFY(component, DATAREADER);
    Hash_Store(self->components, (Obj*)api, (Obj*)component);
}

CharBuf*
SegReader_get_seg_name(SegReader *self) { return self->seg_name; }
int64_t
SegReader_get_seg_num(SegReader *self)  { return self->seg_num; }

int32_t
SegReader_del_count(SegReader *self) 
{
    return self->del_count;
}

int32_t
SegReader_doc_max(SegReader *self)
{
    return self->doc_max;
}

int32_t
SegReader_doc_count(SegReader *self)
{
    return self->doc_max - self->del_count;
}

I32Array*
SegReader_offsets(SegReader *self)
{
    int32_t *ints = (int32_t*)CALLOCATE(1, sizeof(int32_t));
    UNUSED_VAR(self);
    return I32Arr_new_steal(ints, 1);
}

VArray*
SegReader_seg_readers(SegReader *self)
{
    VArray *seg_readers = VA_new(1);
    VA_Push(seg_readers, INCREF(self));
    return seg_readers;
}


