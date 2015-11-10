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

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#define C_LUCY_SIMPLE
#include "Lucy/Simple.h"

#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/HashIterator.h"
#include "Clownfish/String.h"
#include "Clownfish/Vector.h"
#include "Lucy/Analysis/EasyAnalyzer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"

Simple*
Simple_new(Obj *index, String *language) {
    Simple *self = (Simple*)Class_Make_Obj(SIMPLE);
    return Simple_init(self, index, language);
}

Simple*
Simple_init(Simple *self, Obj *index, String *language) {
    SimpleIVARS *const ivars = Simple_IVARS(self);
    ivars->index    = INCREF(index);
    ivars->language = Str_Clone(language);
    return self;
}

void
Simple_Destroy_IMP(Simple *self) {
    SimpleIVARS *const ivars = Simple_IVARS(self);

    Simple_Finish_Indexing(self);

    DECREF(ivars->index);
    DECREF(ivars->language);
    DECREF(ivars->schema);
    DECREF(ivars->type);
    DECREF(ivars->indexer);
    DECREF(ivars->searcher);
    DECREF(ivars->hits);

    SUPER_DESTROY(self, SIMPLE);
}

static void
S_create_indexer(Simple *self) {
    SimpleIVARS *const ivars = Simple_IVARS(self);

    // Trigger searcher refresh.
    DECREF(ivars->searcher);
    DECREF(ivars->hits);
    ivars->searcher = NULL;
    ivars->hits     = NULL;

    // Get type and schema
    Schema     *schema      = NULL;
    FieldType  *type        = NULL;
    PolyReader *reader      = PolyReader_open(ivars->index, NULL, NULL);
    Vector     *seg_readers = PolyReader_Get_Seg_Readers(reader);

    if (Vec_Get_Size(seg_readers) == 0) {
        // Index is empty, create new schema and type.
        schema = Schema_new();
        EasyAnalyzer *analyzer = EasyAnalyzer_new(ivars->language);
        type = (FieldType*)FullTextType_new((Analyzer*)analyzer);
        DECREF(analyzer);
    }
    else {
        // Get schema from reader.
        schema = (Schema*)INCREF(PolyReader_Get_Schema(reader));
        Vector *fields = Schema_All_Fields(schema);
        String *field  = (String*)CERTIFY(Vec_Fetch(fields, 0), STRING);
        type = (FieldType*)INCREF(Schema_Fetch_Type(schema, field));
        DECREF(fields);
    }

    ivars->indexer = Indexer_new(schema, ivars->index, NULL, 0);
    ivars->schema  = schema;
    ivars->type    = type;

    DECREF(reader);
}

void
Simple_Add_Doc_IMP(Simple *self, Doc *doc) {
    SimpleIVARS *const ivars = Simple_IVARS(self);

    if (!ivars->indexer) {
        S_create_indexer(self);
    }

    Vector *field_names = Doc_Field_Names(doc);

    for (size_t i = 0, max = Vec_Get_Size(field_names); i < max; i++) {
        String *field = (String*)Vec_Fetch(field_names, i);
        Schema_Spec_Field(ivars->schema, field, ivars->type);
    }

    Indexer_Add_Doc(ivars->indexer, doc, 1.0);

    DECREF(field_names);
}

uint32_t
Simple_Search_IMP(Simple *self, String *query, uint32_t offset,
                  uint32_t num_wanted) {
    SimpleIVARS *const ivars = Simple_IVARS(self);

    // Flush recent adds; lazily create searcher.
    Simple_Finish_Indexing(self);
    if (!ivars->searcher) {
        ivars->searcher = IxSearcher_new(ivars->index);
    }

    DECREF(ivars->hits);
    ivars->hits = IxSearcher_Hits(ivars->searcher, (Obj*)query, offset,
                                  num_wanted, NULL);

    return Hits_Total_Hits(ivars->hits);
}

HitDoc*
Simple_Next_IMP(Simple *self) {
    SimpleIVARS *const ivars = Simple_IVARS(self);

    if (!ivars->hits) { return NULL; }

    // Get the hit, bail if hits are exhausted.
    HitDoc *doc = Hits_Next(ivars->hits);
    if (!doc) {
        DECREF(ivars->hits);
        ivars->hits = NULL;
    }

    return doc;
}

Indexer*
Simple_Get_Indexer_IMP(Simple *self) {
    SimpleIVARS *const ivars = Simple_IVARS(self);

    if (!ivars->indexer) {
        S_create_indexer(self);
    }

    return ivars->indexer;
}

Hits*
Simple_Get_Hits_IMP(Simple *self) {
    return Simple_IVARS(self)->hits;
}

void
Simple_Finish_Indexing_IMP(Simple *self) {
    SimpleIVARS *const ivars = Simple_IVARS(self);

    // Don't bother to throw an error if index not modified.
    if (ivars->indexer) {
        Indexer_Commit(ivars->indexer);

        // Trigger searcher and indexer refresh.
        DECREF(ivars->schema);
        DECREF(ivars->type);
        DECREF(ivars->indexer);
        ivars->schema   = NULL;
        ivars->type     = NULL;
        ivars->indexer  = NULL;
    }
}

