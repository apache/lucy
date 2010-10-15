#define C_KINO_COLLECTOR
#define C_KINO_BITCOLLECTOR
#define C_KINO_OFFSETCOLLECTOR
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/Collector.h"
#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Search/Matcher.h"

Collector*
Coll_init(Collector *self)
{
    ABSTRACT_CLASS_CHECK(self, COLLECTOR);
    self->reader  = NULL;
    self->matcher = NULL;
    self->base    = 0;
    return self;
}

void
Coll_destroy(Collector *self)
{
    DECREF(self->reader);
    DECREF(self->matcher);
    SUPER_DESTROY(self, COLLECTOR);
}

void
Coll_set_reader(Collector *self, SegReader *reader)
{
    DECREF(self->reader);
    self->reader = (SegReader*)INCREF(reader);
}

void
Coll_set_matcher(Collector *self, Matcher *matcher)
{
    DECREF(self->matcher);
    self->matcher = (Matcher*)INCREF(matcher);
}

void
Coll_set_base(Collector *self, int32_t base)
{
    self->base = base;
}

BitCollector*
BitColl_new(BitVector *bit_vec) 
{
    BitCollector *self = (BitCollector*)VTable_Make_Obj(BITCOLLECTOR);
    return BitColl_init(self, bit_vec);
}

BitCollector*
BitColl_init(BitCollector *self, BitVector *bit_vec) 
{
    Coll_init((Collector*)self);
    self->bit_vec = (BitVector*)INCREF(bit_vec);
    return self;
}

void
BitColl_destroy(BitCollector *self)
{
    DECREF(self->bit_vec);
    SUPER_DESTROY(self, BITCOLLECTOR);
}

void
BitColl_collect(BitCollector *self, int32_t doc_id) 
{
    // Add the doc_id to the BitVector. 
    BitVec_Set(self->bit_vec, (self->base + doc_id));
}

bool_t
BitColl_need_score(BitCollector *self) 
{
    UNUSED_VAR(self);
    return false;
}

OffsetCollector*
OffsetColl_new(Collector *inner_coll, int32_t offset) 
{
    OffsetCollector *self 
        = (OffsetCollector*)VTable_Make_Obj(OFFSETCOLLECTOR);
    return OffsetColl_init(self, inner_coll, offset);
}

OffsetCollector*
OffsetColl_init(OffsetCollector *self, Collector *inner_coll, int32_t offset)
{
    Coll_init((Collector*)self);
    self->offset     = offset;
    self->inner_coll = (Collector*)INCREF(inner_coll);
    return self;
}

void
OffsetColl_destroy(OffsetCollector *self)
{
    DECREF(self->inner_coll);
    SUPER_DESTROY(self, OFFSETCOLLECTOR);
}

void
OffsetColl_set_reader(OffsetCollector *self, SegReader *reader)
{
    Coll_Set_Reader(self->inner_coll, reader);
}

void
OffsetColl_set_base(OffsetCollector *self, int32_t base)
{
    Coll_Set_Base(self->inner_coll, base);
}

void
OffsetColl_set_matcher(OffsetCollector *self, Matcher *matcher)
{
    Coll_Set_Matcher(self->inner_coll, matcher);
}

void
OffsetColl_collect(OffsetCollector *self, int32_t doc_id) 
{
    Coll_Collect(self->inner_coll, (doc_id + self->offset));
}

bool_t
OffsetColl_need_score(OffsetCollector *self) 
{
    return Coll_Need_Score(self->inner_coll);
}


