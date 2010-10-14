#define C_KINO_POSTINGLISTREADER
#define C_KINO_POLYPOSTINGLISTREADER
#define C_KINO_DEFAULTPOSTINGLISTREADER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/PostingListReader.h"
#include "KinoSearch/Index/LexiconReader.h"
#include "KinoSearch/Index/PostingListWriter.h"
#include "KinoSearch/Index/Segment.h"
#include "KinoSearch/Index/SegPostingList.h"
#include "KinoSearch/Index/Snapshot.h"
#include "KinoSearch/Plan/Architecture.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Store/Folder.h"

PostingListReader*
PListReader_init(PostingListReader *self, Schema *schema, Folder *folder, 
                 Snapshot *snapshot, VArray *segments, int32_t seg_tick)
{
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
        seg_tick);
    ABSTRACT_CLASS_CHECK(self, POSTINGLISTREADER);
    return self;
}

PostingListReader*
PListReader_aggregator(PostingListReader *self, VArray *readers, 
                       I32Array *offsets)
{
    UNUSED_VAR(self);
    UNUSED_VAR(readers);
    UNUSED_VAR(offsets);
    return NULL;
}

DefaultPostingListReader*
DefPListReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
                   VArray *segments, int32_t seg_tick, 
                   LexiconReader *lex_reader)
{
    DefaultPostingListReader *self = (DefaultPostingListReader*)
        VTable_Make_Obj(DEFAULTPOSTINGLISTREADER);
    return DefPListReader_init(self, schema, folder, snapshot, segments, 
        seg_tick, lex_reader);
}

DefaultPostingListReader*
DefPListReader_init(DefaultPostingListReader *self, Schema *schema, 
                    Folder *folder, Snapshot *snapshot, VArray *segments,
                   int32_t seg_tick, LexiconReader *lex_reader)
{
    PListReader_init((PostingListReader*)self, schema, folder, snapshot, 
        segments, seg_tick);
    Segment *segment = DefPListReader_Get_Segment(self);

    // Derive. 
    self->lex_reader = (LexiconReader*)INCREF(lex_reader);

    // Check format. 
    {
        Hash *my_meta = (Hash*)Seg_Fetch_Metadata_Str(segment, "postings", 8);
        if (!my_meta) { 
            my_meta = (Hash*)Seg_Fetch_Metadata_Str(segment, 
                "posting_list", 12);
        }

        if (my_meta) {
            Obj *format = Hash_Fetch_Str(my_meta, "format", 6);
            if (!format) { THROW(ERR, "Missing 'format' var"); }
            else {
                if (Obj_To_I64(format) != PListWriter_current_file_format) {
                    THROW(ERR, "Unsupported postings format: %i64", 
                        Obj_To_I64(format));
                }
            }
        }
    }
    
    return self;
}

void
DefPListReader_close(DefaultPostingListReader *self)
{
    if (self->lex_reader) {
        LexReader_Close(self->lex_reader);
        DECREF(self->lex_reader);
        self->lex_reader = NULL;
    }
}

void
DefPListReader_destroy(DefaultPostingListReader *self)
{
    DECREF(self->lex_reader);
    SUPER_DESTROY(self, DEFAULTPOSTINGLISTREADER);
}

SegPostingList*
DefPListReader_posting_list(DefaultPostingListReader *self, 
                           const CharBuf *field, Obj *target)
{
    FieldType *type  = Schema_Fetch_Type(self->schema, field);

    // Only return an object if we've got an indexed field. 
    if (type != NULL && FType_Indexed(type)) {
        SegPostingList *plist = SegPList_new((PostingListReader*)self, field);
        if (target) SegPList_Seek(plist, target);
        return plist;
    }
    else {
        return NULL;
    }
}

LexiconReader*
DefPListReader_get_lex_reader(DefaultPostingListReader *self) 
    { return self->lex_reader; }

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

