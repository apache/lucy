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

#define C_LUCY_FULLTEXTTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Index/Posting/ScorePosting.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Util/Freezer.h"

FullTextType*
FullTextType_new(Analyzer *analyzer) {
    FullTextType *self = (FullTextType*)Class_Make_Obj(FULLTEXTTYPE);
    return FullTextType_init(self, analyzer);
}

FullTextType*
FullTextType_init(FullTextType *self, Analyzer *analyzer) {
    return FullTextType_init2(self, analyzer, 1.0, true, true, false, false);
}

FullTextType*
FullTextType_init2(FullTextType *self, Analyzer *analyzer, float boost,
                   bool indexed, bool stored, bool sortable,
                   bool highlightable) {
    FType_init((FieldType*)self);
    FullTextTypeIVARS *const ivars = FullTextType_IVARS(self);

    /* Assign */
    ivars->boost         = boost;
    ivars->indexed       = indexed;
    ivars->stored        = stored;
    ivars->sortable      = sortable;
    ivars->highlightable = highlightable;
    ivars->analyzer      = (Analyzer*)INCREF(analyzer);

    return self;
}

void
FullTextType_Destroy_IMP(FullTextType *self) {
    FullTextTypeIVARS *const ivars = FullTextType_IVARS(self);
    DECREF(ivars->analyzer);
    SUPER_DESTROY(self, FULLTEXTTYPE);
}

bool
FullTextType_Equals_IMP(FullTextType *self, Obj *other) {
    if ((FullTextType*)other == self)                     { return true; }
    if (!Obj_Is_A(other, FULLTEXTTYPE))                   { return false; }
    FullTextTypeIVARS *const ivars = FullTextType_IVARS(self);
    FullTextTypeIVARS *const ovars = FullTextType_IVARS((FullTextType*)other);
    FullTextType_Equals_t super_equals
        = (FullTextType_Equals_t)SUPER_METHOD_PTR(FULLTEXTTYPE,
                                                  LUCY_FullTextType_Equals);
    if (!super_equals(self, other))                       { return false; }
    if (!!ivars->sortable      != !!ovars->sortable)      { return false; }
    if (!!ivars->highlightable != !!ovars->highlightable) { return false; }
    if (!Analyzer_Equals(ivars->analyzer, (Obj*)ovars->analyzer)) {
        return false;
    }
    return true;
}

Hash*
FullTextType_Dump_For_Schema_IMP(FullTextType *self) {
    FullTextTypeIVARS *const ivars = FullTextType_IVARS(self);
    Hash *dump = Hash_new(0);
    Hash_Store_Utf8(dump, "type", 4, (Obj*)Str_newf("fulltext"));

    // Store attributes that override the defaults.
    if (ivars->boost != 1.0) {
        Hash_Store_Utf8(dump, "boost", 5, (Obj*)Str_newf("%f64", ivars->boost));
    }
    if (!ivars->indexed) {
        Hash_Store_Utf8(dump, "indexed", 7, (Obj*)CFISH_FALSE);
    }
    if (!ivars->stored) {
        Hash_Store_Utf8(dump, "stored", 6, (Obj*)CFISH_FALSE);
    }
    if (ivars->sortable) {
        Hash_Store_Utf8(dump, "sortable", 8, (Obj*)CFISH_TRUE);
    }
    if (ivars->highlightable) {
        Hash_Store_Utf8(dump, "highlightable", 13, (Obj*)CFISH_TRUE);
    }

    return dump;
}

Hash*
FullTextType_Dump_IMP(FullTextType *self) {
    FullTextTypeIVARS *const ivars = FullTextType_IVARS(self);
    Hash *dump = FullTextType_Dump_For_Schema(self);
    Hash_Store_Utf8(dump, "_class", 6,
                    (Obj*)Str_Clone(FullTextType_Get_Class_Name(self)));
    Hash_Store_Utf8(dump, "analyzer", 8,
                    (Obj*)Analyzer_Dump(ivars->analyzer));
    DECREF(Hash_Delete_Utf8(dump, "type", 4));

    return dump;
}

FullTextType*
FullTextType_Load_IMP(FullTextType *self, Obj *dump) {
    UNUSED_VAR(self);
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    String *class_name = (String*)Hash_Fetch_Utf8(source, "_class", 6);
    Class *klass
        = (class_name != NULL && Obj_Is_A((Obj*)class_name, STRING))
          ? Class_singleton(class_name, NULL)
          : FULLTEXTTYPE;
    FullTextType *loaded = (FullTextType*)Class_Make_Obj(klass);

    // Extract boost.
    Obj *boost_dump = Hash_Fetch_Utf8(source, "boost", 5);
    float boost = boost_dump ? (float)Obj_To_F64(boost_dump) : 1.0f;

    // Find boolean properties.
    Obj *indexed_dump = Hash_Fetch_Utf8(source, "indexed", 7);
    Obj *stored_dump  = Hash_Fetch_Utf8(source, "stored", 6);
    Obj *sort_dump    = Hash_Fetch_Utf8(source, "sortable", 8);
    Obj *hl_dump      = Hash_Fetch_Utf8(source, "highlightable", 13);
    bool indexed  = indexed_dump ? Obj_To_Bool(indexed_dump) : true;
    bool stored   = stored_dump  ? Obj_To_Bool(stored_dump)  : true;
    bool sortable = sort_dump    ? Obj_To_Bool(sort_dump)    : false;
    bool hl       = hl_dump      ? Obj_To_Bool(hl_dump)      : false;

    // Extract an Analyzer.
    Obj *analyzer_dump = Hash_Fetch_Utf8(source, "analyzer", 8);
    Analyzer *analyzer = NULL;
    if (analyzer_dump) {
        if (Obj_Is_A(analyzer_dump, ANALYZER)) {
            // Schema munged the dump and installed a shared analyzer.
            analyzer = (Analyzer*)INCREF(analyzer_dump);
        }
        else if (Obj_Is_A((Obj*)analyzer_dump, HASH)) {
            analyzer = (Analyzer*)Freezer_load(analyzer_dump);
        }
    }
    CERTIFY(analyzer, ANALYZER);

    FullTextType_init2(loaded, analyzer, boost, indexed, stored,
                       sortable, hl);
    DECREF(analyzer);
    return loaded;
}

void
FullTextType_Set_Highlightable_IMP(FullTextType *self, bool highlightable) {
    FullTextType_IVARS(self)->highlightable = highlightable;
}

Analyzer*
FullTextType_Get_Analyzer_IMP(FullTextType *self) {
    return FullTextType_IVARS(self)->analyzer;
}

bool
FullTextType_Highlightable_IMP(FullTextType *self) {
    return FullTextType_IVARS(self)->highlightable;
}

Similarity*
FullTextType_Make_Similarity_IMP(FullTextType *self) {
    UNUSED_VAR(self);
    return Sim_new();
}


