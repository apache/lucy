#define C_LUCY_TEXTTYPE
#define C_LUCY_TEXTTERMSTEPPER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/TextType.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/Similarity/LuceneSimilarity.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/StringHelper.h"

TextType*
TextType_new(Analyzer *analyzer, Similarity *similarity)
{
    TextType *self = (TextType*)VTable_Make_Obj(TEXTTYPE);
    return TextType_init(self, analyzer, similarity);
}

TextType*
TextType_init(TextType *self, Analyzer *analyzer, Similarity *similarity)
{
    return TextType_init2(self, analyzer, similarity, 1.0, true, true, 
        false, false);
}

TextType*
TextType_init2(TextType *self, Analyzer *analyzer, Similarity *similarity, 
               float boost, bool_t indexed, bool_t stored, bool_t sortable,
               bool_t highlightable)
{
    FType_init2((FieldType*)self, similarity, boost, indexed, stored,
        sortable);
    self->highlightable = highlightable;
    self->analyzer      = (Analyzer*)INCREF(analyzer);
    return self;
}

void
TextType_destroy(TextType *self)
{
    DECREF(self->analyzer);
    SUPER_DESTROY(self, TEXTTYPE);
}

void
TextType_set_highlightable(TextType *self, bool_t highlightable)
{ 
    self->highlightable = highlightable; 
}

Analyzer*
TextType_get_analyzer(TextType *self)  { return self->analyzer; }
bool_t
TextType_highlightable(TextType *self) { return self->highlightable; }

CharBuf*
TextType_make_blank(TextType *self)
{
    UNUSED_VAR(self);
    return CB_new(0);
}

uint8_t
TextType_scalar_type_id(TextType *self)
{
    UNUSED_VAR(self);
    return Obj_TEXT;
}

bool_t
TextType_equals(TextType *self, Obj *other)
{
    TextType *evil_twin = (TextType*)other;
    TextType_equals_t super_equals = (TextType_equals_t)SUPER_METHOD(
        TEXTTYPE, TextType, Equals);
    if (!other) return false; 
    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, TEXTTYPE)) return false;
    if (!super_equals(self, other)) return false;
    if (!!self->highlightable != !!evil_twin->highlightable) return false;
    if (!!self->analyzer != !!evil_twin->analyzer) return false;
    if (self->analyzer) {
        if (!Analyzer_Equals(self->analyzer, (Obj*)evil_twin->analyzer)) {
            return false;
        }
    }
    return true;
}

Hash*
TextType_dump_for_schema(TextType *self) 
{
    Hash *dump = Hash_new(0);
    Hash_Store_Str(dump, "type", 4, (Obj*)CB_newf("text"));

    // Store attributes that override the defaults. 
    if (self->boost != 1.0) {
        Hash_Store_Str(dump, "boost", 5, (Obj*)CB_newf("%f64", self->boost));
    }
    if (!self->indexed) {
        Hash_Store_Str(dump, "indexed", 7, (Obj*)CB_newf("0"));
    }
    if (!self->stored) {
        Hash_Store_Str(dump, "stored", 6, (Obj*)CB_newf("0"));
    }
    if (self->sortable) {
        Hash_Store_Str(dump, "sortable", 8, (Obj*)CB_newf("1"));
    }
    if (self->highlightable) {
        Hash_Store_Str(dump, "highlightable", 13, (Obj*)CB_newf("1"));
    }

    return dump;
}

Hash*
TextType_dump(TextType *self)
{
    Hash *dump = TextType_Dump_For_Schema(self);
    Hash_Store_Str(dump, "_class", 6, 
        (Obj*)CB_Clone(TextType_Get_Class_Name(self)));
    if (self->analyzer) {
        Hash_Store_Str(dump, "analyzer", 8, 
            (Obj*)Analyzer_Dump(self->analyzer));
    }
    Hash_Store_Str(dump, "similarity", 10, (Obj*)Sim_Dump(self->sim));
    DECREF(Hash_Delete_Str(dump, "type", 4));
    return dump;
}

TextType*
TextType_load(TextType *self, Obj *dump)
{
    UNUSED_VAR(self);
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *class_name = (CharBuf*)Hash_Fetch_Str(source, "_class", 6);
    CharBuf *type_str   = (CharBuf*)Hash_Fetch_Str(source, "type", 4);
    VTable *vtable = NULL;
    if (class_name && Obj_Is_A((Obj*)class_name, CHARBUF)) { 
         vtable = VTable_singleton(class_name, NULL);
    }
    else if (type_str && Obj_Is_A((Obj*)type_str, CHARBUF)) { 
        if (   CB_Equals_Str(type_str, "text", 4)
            || CB_Equals_Str(type_str, "fulltext", 8)
            || CB_Equals_Str(type_str, "string", 6)
        ) {
            vtable = TEXTTYPE;
        }
    }
    if (!vtable) {
        THROW(ERR, "Unknown class or type");
    }
    TextType *loaded = (TextType*)VTable_Make_Obj(vtable);

    // Extract boost.
    Obj *boost_dump = Hash_Fetch_Str(source, "boost", 5);
    float boost = boost_dump ? (float)Obj_To_F64(boost_dump) : 1.0f;

    // Find boolean properties.
    Obj *indexed_dump = Hash_Fetch_Str(source, "indexed", 7);
    Obj *stored_dump  = Hash_Fetch_Str(source, "stored", 6);
    Obj *sort_dump    = Hash_Fetch_Str(source, "sortable", 8);
    Obj *hl_dump      = Hash_Fetch_Str(source, "highlightable", 13);
    bool_t indexed  = indexed_dump ? (bool_t)Obj_To_I64(indexed_dump) : true;
    bool_t stored   = stored_dump  ? (bool_t)Obj_To_I64(stored_dump)  : true;
    bool_t sortable = sort_dump    ? (bool_t)Obj_To_I64(sort_dump)    : false;
    bool_t hl       = hl_dump      ? (bool_t)Obj_To_I64(hl_dump)      : false;

    // Extract an Analyzer.  
    Obj *analyzer_dump = Hash_Fetch_Str(source, "analyzer", 8);
    Analyzer *analyzer = NULL;
    if (analyzer_dump) {
        if (Obj_Is_A(analyzer_dump, ANALYZER)) {
            // Schema munged the dump and installed a shared analyzer.
            analyzer = (Analyzer*)INCREF(analyzer_dump);
        }
        else {
            analyzer = (Analyzer*)Obj_Load(analyzer_dump, analyzer_dump);
        }
        CERTIFY(analyzer, ANALYZER);
    }

    // Extract a Similarity.
    Similarity *sim = NULL;
    Obj *sim_dump = Hash_Fetch_Str(source, "similarity", 10);
    if (sim_dump) {
        if (Obj_Is_A(sim_dump, SIMILARITY)) {
            // Schema munged the dump and installed a shared sim.
            sim = (Similarity*)INCREF(sim_dump);
        }
        else {
            sim = (Similarity*)CERTIFY(
                Obj_Load(sim_dump, sim_dump), SIMILARITY);
        }
    }

    TextType_init2(loaded, analyzer, sim, boost, indexed, stored,
        sortable, hl);

    DECREF(sim);
    DECREF(analyzer);
    return loaded;
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

