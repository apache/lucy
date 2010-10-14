#define C_KINO_DATAWRITER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/DataWriter.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Store/Folder.h"
#include "KinoSearch/Index/PolyReader.h"
#include "KinoSearch/Index/Segment.h"
#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Index/Snapshot.h"
#include "KinoSearch/Store/Folder.h"

DataWriter*
DataWriter_init(DataWriter *self, Schema *schema, Snapshot *snapshot,
                Segment *segment, PolyReader *polyreader)
{
    self->snapshot   = (Snapshot*)INCREF(snapshot);
    self->segment    = (Segment*)INCREF(segment);
    self->polyreader = (PolyReader*)INCREF(polyreader);
    self->schema     = (Schema*)INCREF(schema);
    self->folder     = (Folder*)INCREF(PolyReader_Get_Folder(polyreader));
    ABSTRACT_CLASS_CHECK(self, DATAWRITER);
    return self;
}

void
DataWriter_destroy(DataWriter *self) 
{
    DECREF(self->snapshot);
    DECREF(self->segment);
    DECREF(self->polyreader);
    DECREF(self->schema);
    DECREF(self->folder);
    SUPER_DESTROY(self, DATAWRITER);
}

Snapshot*
DataWriter_get_snapshot(DataWriter *self) { return self->snapshot; }
Segment*
DataWriter_get_segment(DataWriter *self)  { return self->segment; }
PolyReader*
DataWriter_get_polyreader(DataWriter *self) { return self->polyreader; }
Schema*
DataWriter_get_schema(DataWriter *self) { return self->schema; }
Folder*
DataWriter_get_folder(DataWriter *self) { return self->folder; }

void
DataWriter_delete_segment(DataWriter *self, SegReader *reader)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
DataWriter_merge_segment(DataWriter *self, SegReader *reader, 
                         I32Array *doc_map)
{
    DataWriter_Add_Segment(self, reader, doc_map);
    DataWriter_Delete_Segment(self, reader);
}

Hash*
DataWriter_metadata(DataWriter *self)
{
    Hash *metadata = Hash_new(0);
    Hash_Store_Str(metadata, "format", 6, 
        (Obj*)CB_newf("%i32", DataWriter_Format(self)));
    return metadata;
}

/* Copyright 2007-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

