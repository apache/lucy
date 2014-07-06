/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define C_LUCY_POSTINGLISTREADER
#define C_LUCY_POLYPOSTINGLISTREADER
#define C_LUCY_DEFAULTPOSTINGLISTREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/PostingListReader.h"
#include "Lucy/Index/LexiconReader.h"
#include "Lucy/Index/PostingListWriter.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegPostingList.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"

PostingListReader*
PListReader_init(PostingListReader *self, Schema *schema, Folder *folder,
                 Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
                    seg_tick);
    ABSTRACT_CLASS_CHECK(self, POSTINGLISTREADER);
    return self;
}

PostingListReader*
PListReader_Aggregator_IMP(PostingListReader *self, VArray *readers,
                           I32Array *offsets) {
    UNUSED_VAR(self);
    UNUSED_VAR(readers);
    UNUSED_VAR(offsets);
    return NULL;
}

DefaultPostingListReader*
DefPListReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
                   VArray *segments, int32_t seg_tick,
                   LexiconReader *lex_reader) {
    DefaultPostingListReader *self
        = (DefaultPostingListReader*)Class_Make_Obj(DEFAULTPOSTINGLISTREADER);
    return DefPListReader_init(self, schema, folder, snapshot, segments,
                               seg_tick, lex_reader);
}

DefaultPostingListReader*
DefPListReader_init(DefaultPostingListReader *self, Schema *schema,
                    Folder *folder, Snapshot *snapshot, VArray *segments,
                    int32_t seg_tick, LexiconReader *lex_reader) {
    PListReader_init((PostingListReader*)self, schema, folder, snapshot,
                     segments, seg_tick);
    DefaultPostingListReaderIVARS *const ivars = DefPListReader_IVARS(self);
    Segment *segment = DefPListReader_Get_Segment(self);

    // Derive.
    ivars->lex_reader = (LexiconReader*)INCREF(lex_reader);

    // Check format.
    Hash *my_meta = (Hash*)Seg_Fetch_Metadata_Utf8(segment, "postings", 8);
    if (!my_meta) {
        my_meta = (Hash*)Seg_Fetch_Metadata_Utf8(segment, "posting_list", 12);
    }

    if (my_meta) {
        Obj *format = Hash_Fetch_Utf8(my_meta, "format", 6);
        if (!format) { THROW(ERR, "Missing 'format' var"); }
        else {
            if (Obj_To_I64(format) != PListWriter_current_file_format) {
                THROW(ERR, "Unsupported postings format: %i64",
                      Obj_To_I64(format));
            }
        }
    }

    return self;
}

void
DefPListReader_Close_IMP(DefaultPostingListReader *self) {
    DefaultPostingListReaderIVARS *const ivars = DefPListReader_IVARS(self);
    if (ivars->lex_reader) {
        LexReader_Close(ivars->lex_reader);
        DECREF(ivars->lex_reader);
        ivars->lex_reader = NULL;
    }
}

void
DefPListReader_Destroy_IMP(DefaultPostingListReader *self) {
    DefaultPostingListReaderIVARS *const ivars = DefPListReader_IVARS(self);
    DECREF(ivars->lex_reader);
    SUPER_DESTROY(self, DEFAULTPOSTINGLISTREADER);
}

SegPostingList*
DefPListReader_Posting_List_IMP(DefaultPostingListReader *self,
                                String *field, Obj *target) {
    DefaultPostingListReaderIVARS *const ivars = DefPListReader_IVARS(self);
    FieldType *type = Schema_Fetch_Type(ivars->schema, field);

    // Only return an object if we've got an indexed field.
    if (type != NULL && FType_Indexed(type)) {
        SegPostingList *plist = SegPList_new((PostingListReader*)self, field);
        if (target) { SegPList_Seek(plist, target); }
        return plist;
    }
    else {
        return NULL;
    }
}

LexiconReader*
DefPListReader_Get_Lex_Reader_IMP(DefaultPostingListReader *self) {
    return DefPListReader_IVARS(self)->lex_reader;
}

