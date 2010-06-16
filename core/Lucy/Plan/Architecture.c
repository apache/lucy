#define C_LUCY_ARCHITECTURE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/Architecture.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/SegWriter.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/Similarity/LuceneSimilarity.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"

Architecture*
Arch_new()
{
    Architecture *self = (Architecture*)VTable_Make_Obj(ARCHITECTURE);
    return Arch_init(self);
}

Architecture*
Arch_init(Architecture *self)
{
    return self;
}

bool_t
Arch_equals(Architecture *self, Obj *other)
{
    Architecture *evil_twin = (Architecture*)other;
    if (evil_twin == self) return true;
    if (!other) return false;
    if (!Obj_Is_A(other, ARCHITECTURE)) return false;
    return true;
}

void
Arch_init_seg_writer(Architecture *self, SegWriter *writer)
{
    Arch_Register_Lexicon_Writer(self, writer);
    Arch_Register_Posting_List_Writer(self, writer);
    Arch_Register_Sort_Writer(self, writer);
    Arch_Register_Doc_Writer(self, writer);
    Arch_Register_Highlight_Writer(self, writer);
    Arch_Register_Deletions_Writer(self, writer);
}

void
Arch_register_lexicon_writer(Architecture *self, SegWriter *writer)
{
    UNUSED_VAR(self);
    UNUSED_VAR(writer);
}

void
Arch_register_posting_list_writer(Architecture *self, SegWriter *writer)
{
    UNUSED_VAR(self);
    UNUSED_VAR(writer);
}

void
Arch_register_doc_writer(Architecture *self, SegWriter *writer)
{
    UNUSED_VAR(self);
    UNUSED_VAR(writer);
}

void
Arch_register_sort_writer(Architecture *self, SegWriter *writer)
{
    UNUSED_VAR(self);
    UNUSED_VAR(writer);
}

void
Arch_register_highlight_writer(Architecture *self, SegWriter *writer)
{
    UNUSED_VAR(self);
    UNUSED_VAR(writer);
}

void
Arch_register_deletions_writer(Architecture *self, SegWriter *writer)
{
    UNUSED_VAR(self);
    UNUSED_VAR(writer);
}

void
Arch_init_seg_reader(Architecture *self, SegReader *reader)
{
    Arch_Register_Doc_Reader(self, reader);
    Arch_Register_Lexicon_Reader(self, reader);
    Arch_Register_Posting_List_Reader(self, reader);
    Arch_Register_Sort_Reader(self, reader);
    Arch_Register_Highlight_Reader(self, reader);
    Arch_Register_Deletions_Reader(self, reader);
}

void
Arch_register_doc_reader(Architecture *self, SegReader *reader)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
Arch_register_posting_list_reader(Architecture *self, SegReader *reader)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
Arch_register_lexicon_reader(Architecture *self, SegReader *reader)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
Arch_register_sort_reader(Architecture *self, SegReader *reader)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
Arch_register_highlight_reader(Architecture *self, SegReader *reader)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
Arch_register_deletions_reader(Architecture *self, SegReader *reader)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

Similarity*
Arch_make_similarity(Architecture *self)
{
    UNUSED_VAR(self);
    return (Similarity*)LuceneSim_new();
}

int32_t
Arch_index_interval(Architecture *self) 
{
    UNUSED_VAR(self);
    return 128;
}

int32_t
Arch_skip_interval(Architecture *self) 
{
    UNUSED_VAR(self);
    return 16;
}

/* Copyright 2010 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

