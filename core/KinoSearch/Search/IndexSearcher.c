#define C_KINO_INDEXSEARCHER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/IndexSearcher.h"

#include "KinoSearch/Index/DeletionsReader.h"
#include "KinoSearch/Index/DocReader.h"
#include "KinoSearch/Index/DocVector.h"
#include "KinoSearch/Index/IndexReader.h"
#include "KinoSearch/Index/LexiconReader.h"
#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Index/SortCache.h"
#include "KinoSearch/Index/HighlightReader.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/Collector.h"
#include "KinoSearch/Search/Collector/SortCollector.h"
#include "KinoSearch/Search/HitQueue.h"
#include "KinoSearch/Search/MatchDoc.h"
#include "KinoSearch/Search/Matcher.h"
#include "KinoSearch/Search/Query.h"
#include "KinoSearch/Search/SortRule.h"
#include "KinoSearch/Search/SortSpec.h"
#include "KinoSearch/Search/TopDocs.h"
#include "KinoSearch/Search/Compiler.h"
#include "KinoSearch/Store/Folder.h"
#include "KinoSearch/Store/FSFolder.h"

IndexSearcher*
IxSearcher_new(Obj *index)
{
    IndexSearcher *self = (IndexSearcher*)VTable_Make_Obj(INDEXSEARCHER);
    return IxSearcher_init(self, index);
}

IndexSearcher*
IxSearcher_init(IndexSearcher *self, Obj *index)
{
    if (Obj_Is_A(index, INDEXREADER)) {
        self->reader = (IndexReader*)INCREF(index);
    }
    else {
        self->reader = IxReader_open(index, NULL, NULL);
    }
    Searcher_init((Searcher*)self, IxReader_Get_Schema(self->reader));
    self->seg_readers = IxReader_Seg_Readers(self->reader);
    self->seg_starts  = IxReader_Offsets(self->reader);
    self->doc_reader = (DocReader*)IxReader_Fetch(
        self->reader, VTable_Get_Name(DOCREADER));
    self->hl_reader = (HighlightReader*)IxReader_Fetch(
        self->reader, VTable_Get_Name(HIGHLIGHTREADER));
    if (self->doc_reader) { INCREF(self->doc_reader); }
    if (self->hl_reader)  { INCREF(self->hl_reader); }

    return self;
}

void
IxSearcher_destroy(IndexSearcher *self)
{
    DECREF(self->reader);
    DECREF(self->doc_reader);
    DECREF(self->hl_reader);
    DECREF(self->seg_readers);
    DECREF(self->seg_starts);
    SUPER_DESTROY(self, INDEXSEARCHER);
}

Obj*
IxSearcher_fetch_doc(IndexSearcher *self, int32_t doc_id, float score, int32_t offset)
{
    if (!self->doc_reader) { THROW(ERR, "No DocReader"); }
    return DocReader_Fetch(self->doc_reader, doc_id, score, offset);
}

DocVector*
IxSearcher_fetch_doc_vec(IndexSearcher *self, int32_t doc_id)
{
    if (!self->hl_reader) { THROW(ERR, "No HighlightReader"); }
    return HLReader_Fetch(self->hl_reader, doc_id);
}

int32_t 
IxSearcher_doc_max(IndexSearcher *self)
{
    return IxReader_Doc_Max(self->reader);
}

uint32_t
IxSearcher_doc_freq(IndexSearcher *self, const CharBuf *field, Obj *term)
{
    LexiconReader *lex_reader = (LexiconReader*)IxReader_Fetch(self->reader, 
        VTable_Get_Name(LEXICONREADER));
    return lex_reader ? LexReader_Doc_Freq(lex_reader, field, term) : 0;
}

TopDocs*
IxSearcher_top_docs(IndexSearcher *self, Query *query, uint32_t num_wanted, 
                    SortSpec *sort_spec)
{
    Schema        *schema    = IxSearcher_Get_Schema(self);
    SortCollector *collector = SortColl_new(schema, sort_spec, num_wanted);
    IxSearcher_Collect(self, query, (Collector*)collector);
    {
        VArray  *match_docs = SortColl_Pop_Match_Docs(collector);
        int32_t  total_hits = SortColl_Get_Total_Hits(collector);
        TopDocs *retval     = TopDocs_new(match_docs, total_hits);
        DECREF(collector);
        DECREF(match_docs);
        return retval;
    }
}

void
IxSearcher_collect(IndexSearcher *self, Query *query, Collector *collector)
{
    uint32_t i, max;
    VArray   *const seg_readers  = self->seg_readers;
    I32Array *const seg_starts   = self->seg_starts;
    bool_t    need_score         = Coll_Need_Score(collector);
    Compiler *compiler = Query_Is_A(query, COMPILER)
                       ? (Compiler*)INCREF(query)
                       : Query_Make_Compiler(query, (Searcher*)self, 
                                             Query_Get_Boost(query));

    // Accumulate hits into the Collector. 
    for (i = 0, max = VA_Get_Size(seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(seg_readers, i);
        DeletionsReader *del_reader = (DeletionsReader*)SegReader_Fetch(
            seg_reader, VTable_Get_Name(DELETIONSREADER));
        Matcher *matcher 
            = Compiler_Make_Matcher(compiler, seg_reader, need_score);
        if (matcher) {
            int32_t seg_start = I32Arr_Get(seg_starts, i);
            Matcher *deletions = DelReader_Iterator(del_reader);
            Coll_Set_Reader(collector, seg_reader);
            Coll_Set_Base(collector, seg_start);
            Coll_Set_Matcher(collector, matcher);
            Matcher_Collect(matcher, collector, deletions);
            DECREF(deletions);
            DECREF(matcher);
        }
    }

    DECREF(compiler);
}

IndexReader*
IxSearcher_get_reader(IndexSearcher *self) { return self->reader; }

void
IxSearcher_close(IndexSearcher *self)
{
    UNUSED_VAR(self);
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

