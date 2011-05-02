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

#define C_LUCY_ARCHITECTURE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/Architecture.h"
#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/DeletionsWriter.h"
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/DocWriter.h"
#include "Lucy/Index/HighlightReader.h"
#include "Lucy/Index/HighlightWriter.h"
#include "Lucy/Index/LexiconReader.h"
#include "Lucy/Index/LexiconWriter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/PostingListReader.h"
#include "Lucy/Index/PostingListWriter.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/SegWriter.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SortReader.h"
#include "Lucy/Index/SortWriter.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"

Architecture*
Arch_new() {
    Architecture *self = (Architecture*)VTable_Make_Obj(ARCHITECTURE);
    return Arch_init(self);
}

Architecture*
Arch_init(Architecture *self) {
    return self;
}

bool_t
Arch_equals(Architecture *self, Obj *other) {
    Architecture *twin = (Architecture*)other;
    if (twin == self)                   { return true; }
    if (!Obj_Is_A(other, ARCHITECTURE)) { return false; }
    return true;
}

void
Arch_init_seg_writer(Architecture *self, SegWriter *writer) {
    Arch_Register_Lexicon_Writer(self, writer);
    Arch_Register_Posting_List_Writer(self, writer);
    Arch_Register_Sort_Writer(self, writer);
    Arch_Register_Doc_Writer(self, writer);
    Arch_Register_Highlight_Writer(self, writer);
    Arch_Register_Deletions_Writer(self, writer);
}

void
Arch_register_lexicon_writer(Architecture *self, SegWriter *writer) {
    Schema        *schema     = SegWriter_Get_Schema(writer);
    Snapshot      *snapshot   = SegWriter_Get_Snapshot(writer);
    Segment       *segment    = SegWriter_Get_Segment(writer);
    PolyReader    *polyreader = SegWriter_Get_PolyReader(writer);
    LexiconWriter *lex_writer
        = LexWriter_new(schema, snapshot, segment, polyreader);
    UNUSED_VAR(self);
    SegWriter_Register(writer, VTable_Get_Name(LEXICONWRITER),
                       (DataWriter*)lex_writer);
}

void
Arch_register_posting_list_writer(Architecture *self, SegWriter *writer) {
    Schema        *schema     = SegWriter_Get_Schema(writer);
    Snapshot      *snapshot   = SegWriter_Get_Snapshot(writer);
    Segment       *segment    = SegWriter_Get_Segment(writer);
    PolyReader    *polyreader = SegWriter_Get_PolyReader(writer);
    LexiconWriter *lex_writer = (LexiconWriter*)SegWriter_Fetch(
                                    writer, VTable_Get_Name(LEXICONWRITER));
    UNUSED_VAR(self);
    if (!lex_writer) {
        THROW(ERR, "Can't fetch a LexiconWriter");
    }
    else {
        PostingListWriter *plist_writer
            = PListWriter_new(schema, snapshot, segment, polyreader,
                              lex_writer);
        SegWriter_Register(writer, VTable_Get_Name(POSTINGLISTWRITER),
                           (DataWriter*)plist_writer);
        SegWriter_Add_Writer(writer, (DataWriter*)INCREF(plist_writer));
    }
}

void
Arch_register_doc_writer(Architecture *self, SegWriter *writer) {
    Schema     *schema     = SegWriter_Get_Schema(writer);
    Snapshot   *snapshot   = SegWriter_Get_Snapshot(writer);
    Segment    *segment    = SegWriter_Get_Segment(writer);
    PolyReader *polyreader = SegWriter_Get_PolyReader(writer);
    DocWriter  *doc_writer
        = DocWriter_new(schema, snapshot, segment, polyreader);
    UNUSED_VAR(self);
    SegWriter_Register(writer, VTable_Get_Name(DOCWRITER),
                       (DataWriter*)doc_writer);
    SegWriter_Add_Writer(writer, (DataWriter*)INCREF(doc_writer));
}

void
Arch_register_sort_writer(Architecture *self, SegWriter *writer) {
    Schema     *schema     = SegWriter_Get_Schema(writer);
    Snapshot   *snapshot   = SegWriter_Get_Snapshot(writer);
    Segment    *segment    = SegWriter_Get_Segment(writer);
    PolyReader *polyreader = SegWriter_Get_PolyReader(writer);
    SortWriter *sort_writer
        = SortWriter_new(schema, snapshot, segment, polyreader);
    UNUSED_VAR(self);
    SegWriter_Register(writer, VTable_Get_Name(SORTWRITER),
                       (DataWriter*)sort_writer);
    SegWriter_Add_Writer(writer, (DataWriter*)INCREF(sort_writer));
}

void
Arch_register_highlight_writer(Architecture *self, SegWriter *writer) {
    Schema     *schema     = SegWriter_Get_Schema(writer);
    Snapshot   *snapshot   = SegWriter_Get_Snapshot(writer);
    Segment    *segment    = SegWriter_Get_Segment(writer);
    PolyReader *polyreader = SegWriter_Get_PolyReader(writer);
    HighlightWriter *hl_writer
        = HLWriter_new(schema, snapshot, segment, polyreader);
    UNUSED_VAR(self);
    SegWriter_Register(writer, VTable_Get_Name(HIGHLIGHTWRITER),
                       (DataWriter*)hl_writer);
    SegWriter_Add_Writer(writer, (DataWriter*)INCREF(hl_writer));
}

void
Arch_register_deletions_writer(Architecture *self, SegWriter *writer) {
    Schema     *schema     = SegWriter_Get_Schema(writer);
    Snapshot   *snapshot   = SegWriter_Get_Snapshot(writer);
    Segment    *segment    = SegWriter_Get_Segment(writer);
    PolyReader *polyreader = SegWriter_Get_PolyReader(writer);
    DefaultDeletionsWriter *del_writer
        = DefDelWriter_new(schema, snapshot, segment, polyreader);
    UNUSED_VAR(self);
    SegWriter_Register(writer, VTable_Get_Name(DELETIONSWRITER),
                       (DataWriter*)del_writer);
    SegWriter_Set_Del_Writer(writer, (DeletionsWriter*)del_writer);
}

void
Arch_init_seg_reader(Architecture *self, SegReader *reader) {
    Arch_Register_Doc_Reader(self, reader);
    Arch_Register_Lexicon_Reader(self, reader);
    Arch_Register_Posting_List_Reader(self, reader);
    Arch_Register_Sort_Reader(self, reader);
    Arch_Register_Highlight_Reader(self, reader);
    Arch_Register_Deletions_Reader(self, reader);
}

void
Arch_register_doc_reader(Architecture *self, SegReader *reader) {
    Schema     *schema   = SegReader_Get_Schema(reader);
    Folder     *folder   = SegReader_Get_Folder(reader);
    VArray     *segments = SegReader_Get_Segments(reader);
    Snapshot   *snapshot = SegReader_Get_Snapshot(reader);
    int32_t     seg_tick = SegReader_Get_Seg_Tick(reader);
    DefaultDocReader *doc_reader
        = DefDocReader_new(schema, folder, snapshot, segments, seg_tick);
    UNUSED_VAR(self);
    SegReader_Register(reader, VTable_Get_Name(DOCREADER),
                       (DataReader*)doc_reader);
}

void
Arch_register_posting_list_reader(Architecture *self, SegReader *reader) {
    Schema    *schema   = SegReader_Get_Schema(reader);
    Folder    *folder   = SegReader_Get_Folder(reader);
    VArray    *segments = SegReader_Get_Segments(reader);
    Snapshot  *snapshot = SegReader_Get_Snapshot(reader);
    int32_t    seg_tick = SegReader_Get_Seg_Tick(reader);
    LexiconReader *lex_reader = (LexiconReader*)SegReader_Obtain(
                                    reader, VTable_Get_Name(LEXICONREADER));
    DefaultPostingListReader *plist_reader
        = DefPListReader_new(schema, folder, snapshot, segments, seg_tick,
                             lex_reader);
    UNUSED_VAR(self);
    SegReader_Register(reader, VTable_Get_Name(POSTINGLISTREADER),
                       (DataReader*)plist_reader);
}

void
Arch_register_lexicon_reader(Architecture *self, SegReader *reader) {
    Schema    *schema   = SegReader_Get_Schema(reader);
    Folder    *folder   = SegReader_Get_Folder(reader);
    VArray    *segments = SegReader_Get_Segments(reader);
    Snapshot  *snapshot = SegReader_Get_Snapshot(reader);
    int32_t    seg_tick = SegReader_Get_Seg_Tick(reader);
    DefaultLexiconReader *lex_reader
        = DefLexReader_new(schema, folder, snapshot, segments, seg_tick);
    UNUSED_VAR(self);
    SegReader_Register(reader, VTable_Get_Name(LEXICONREADER),
                       (DataReader*)lex_reader);
}

void
Arch_register_sort_reader(Architecture *self, SegReader *reader) {
    Schema     *schema   = SegReader_Get_Schema(reader);
    Folder     *folder   = SegReader_Get_Folder(reader);
    VArray     *segments = SegReader_Get_Segments(reader);
    Snapshot   *snapshot = SegReader_Get_Snapshot(reader);
    int32_t     seg_tick = SegReader_Get_Seg_Tick(reader);
    DefaultSortReader *sort_reader
        = DefSortReader_new(schema, folder, snapshot, segments, seg_tick);
    UNUSED_VAR(self);
    SegReader_Register(reader, VTable_Get_Name(SORTREADER),
                       (DataReader*)sort_reader);
}

void
Arch_register_highlight_reader(Architecture *self, SegReader *reader) {
    Schema     *schema   = SegReader_Get_Schema(reader);
    Folder     *folder   = SegReader_Get_Folder(reader);
    VArray     *segments = SegReader_Get_Segments(reader);
    Snapshot   *snapshot = SegReader_Get_Snapshot(reader);
    int32_t     seg_tick = SegReader_Get_Seg_Tick(reader);
    DefaultHighlightReader* hl_reader
        = DefHLReader_new(schema, folder, snapshot, segments, seg_tick);
    UNUSED_VAR(self);
    SegReader_Register(reader, VTable_Get_Name(HIGHLIGHTREADER),
                       (DataReader*)hl_reader);
}

void
Arch_register_deletions_reader(Architecture *self, SegReader *reader) {
    Schema     *schema   = SegReader_Get_Schema(reader);
    Folder     *folder   = SegReader_Get_Folder(reader);
    VArray     *segments = SegReader_Get_Segments(reader);
    Snapshot   *snapshot = SegReader_Get_Snapshot(reader);
    int32_t     seg_tick = SegReader_Get_Seg_Tick(reader);
    DefaultDeletionsReader* del_reader
        = DefDelReader_new(schema, folder, snapshot, segments, seg_tick);
    UNUSED_VAR(self);
    SegReader_Register(reader, VTable_Get_Name(DELETIONSREADER),
                       (DataReader*)del_reader);
}

Similarity*
Arch_make_similarity(Architecture *self) {
    UNUSED_VAR(self);
    return Sim_new();
}

int32_t
Arch_index_interval(Architecture *self) {
    UNUSED_VAR(self);
    return 128;
}

int32_t
Arch_skip_interval(Architecture *self) {
    UNUSED_VAR(self);
    return 16;
}


