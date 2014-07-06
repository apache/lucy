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

#define C_LUCY_INVERTER
#define C_LUCY_INVERTERENTRY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Inverter.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/BlobType.h"
#include "Lucy/Plan/NumericType.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/TextType.h"
#include "Lucy/Plan/Schema.h"

Inverter*
Inverter_new(Schema *schema, Segment *segment) {
    Inverter *self = (Inverter*)Class_Make_Obj(INVERTER);
    return Inverter_init(self, schema, segment);
}

Inverter*
Inverter_init(Inverter *self, Schema *schema, Segment *segment) {
    InverterIVARS *const ivars = Inverter_IVARS(self);

    // Init.
    ivars->tick       = -1;
    ivars->doc        = NULL;
    ivars->sorted     = false;
    ivars->blank      = InvEntry_new(NULL, NULL, 0);
    ivars->current    = ivars->blank;

    // Derive.
    ivars->entry_pool = VA_new(Schema_Num_Fields(schema));
    ivars->entries    = VA_new(Schema_Num_Fields(schema));

    // Assign.
    ivars->schema  = (Schema*)INCREF(schema);
    ivars->segment = (Segment*)INCREF(segment);

    return self;
}

void
Inverter_Destroy_IMP(Inverter *self) {
    InverterIVARS *const ivars = Inverter_IVARS(self);
    Inverter_Clear(self);
    DECREF(ivars->blank);
    DECREF(ivars->entries);
    DECREF(ivars->entry_pool);
    DECREF(ivars->schema);
    DECREF(ivars->segment);
    SUPER_DESTROY(self, INVERTER);
}

uint32_t
Inverter_Iterate_IMP(Inverter *self) {
    InverterIVARS *const ivars = Inverter_IVARS(self);
    ivars->tick = -1;
    if (!ivars->sorted) {
        VA_Sort(ivars->entries, NULL, NULL);
        ivars->sorted = true;
    }
    return VA_Get_Size(ivars->entries);
}

int32_t
Inverter_Next_IMP(Inverter *self) {
    InverterIVARS *const ivars = Inverter_IVARS(self);
    ivars->current = (InverterEntry*)VA_Fetch(ivars->entries, ++ivars->tick);
    if (!ivars->current) { ivars->current = ivars->blank; } // Exhausted.
    return InvEntry_IVARS(ivars->current)->field_num;
}

void
Inverter_Set_Doc_IMP(Inverter *self, Doc *doc) {
    InverterIVARS *const ivars = Inverter_IVARS(self);
    Inverter_Clear(self); // Zap all cached field values and Inversions.
    ivars->doc = (Doc*)INCREF(doc);
}

void
Inverter_Set_Boost_IMP(Inverter *self, float boost) {
    Inverter_IVARS(self)->boost = boost;
}

float
Inverter_Get_Boost_IMP(Inverter *self) {
    return Inverter_IVARS(self)->boost;
}

Doc*
Inverter_Get_Doc_IMP(Inverter *self) {
    return Inverter_IVARS(self)->doc;
}

String*
Inverter_Get_Field_Name_IMP(Inverter *self) {
    InverterEntry *current = Inverter_IVARS(self)->current;
    return InvEntry_IVARS(current)->field;
}

Obj*
Inverter_Get_Value_IMP(Inverter *self) {
    InverterEntry *current = Inverter_IVARS(self)->current;
    return InvEntry_IVARS(current)->value;
}

FieldType*
Inverter_Get_Type_IMP(Inverter *self) {
    InverterEntry *current = Inverter_IVARS(self)->current;
    return InvEntry_IVARS(current)->type;
}

Analyzer*
Inverter_Get_Analyzer_IMP(Inverter *self) {
    InverterEntry *current = Inverter_IVARS(self)->current;
    return InvEntry_IVARS(current)->analyzer;
}

Similarity*
Inverter_Get_Similarity_IMP(Inverter *self) {
    InverterEntry *current = Inverter_IVARS(self)->current;
    return InvEntry_IVARS(current)->sim;
}

Inversion*
Inverter_Get_Inversion_IMP(Inverter *self) {
    InverterEntry *current = Inverter_IVARS(self)->current;
    return InvEntry_IVARS(current)->inversion;
}


void
Inverter_Add_Field_IMP(Inverter *self, InverterEntry *entry) {
    InverterIVARS *const ivars = Inverter_IVARS(self);
    InverterEntryIVARS *const entry_ivars = InvEntry_IVARS(entry);

    // Get an Inversion, going through analyzer if appropriate.
    if (entry_ivars->analyzer) {
        DECREF(entry_ivars->inversion);
        entry_ivars->inversion
            = Analyzer_Transform_Text(entry_ivars->analyzer,
                                      (String*)entry_ivars->value);
        Inversion_Invert(entry_ivars->inversion);
    }
    else if (entry_ivars->indexed || entry_ivars->highlightable) {
        String *value = (String*)entry_ivars->value;
        size_t token_len = Str_Get_Size(value);
        Token *seed = Token_new(Str_Get_Ptr8(value),
                                token_len, 0, token_len, 1.0f, 1);
        DECREF(entry_ivars->inversion);
        entry_ivars->inversion = Inversion_new(seed);
        DECREF(seed);
        Inversion_Invert(entry_ivars->inversion); // Nearly a no-op.
    }

    // Prime the iterator.
    VA_Push(ivars->entries, INCREF(entry));
    ivars->sorted = false;
}

void
Inverter_Clear_IMP(Inverter *self) {
    InverterIVARS *const ivars = Inverter_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(ivars->entries); i < max; i++) {
        InvEntry_Clear(VA_Fetch(ivars->entries, i));
    }
    VA_Clear(ivars->entries);
    ivars->tick = -1;
    DECREF(ivars->doc);
    ivars->doc = NULL;
}

InverterEntry*
InvEntry_new(Schema *schema, String *field, int32_t field_num) {
    InverterEntry *self = (InverterEntry*)Class_Make_Obj(INVERTERENTRY);
    return InvEntry_init(self, schema, field, field_num);
}

InverterEntry*
InvEntry_init(InverterEntry *self, Schema *schema, String *field,
              int32_t field_num) {
    InverterEntryIVARS *const ivars = InvEntry_IVARS(self);
    ivars->field_num  = field_num;
    ivars->field      = field ? Str_Clone(field) : NULL;
    ivars->inversion  = NULL;

    if (schema) {
        ivars->analyzer
            = (Analyzer*)INCREF(Schema_Fetch_Analyzer(schema, field));
        ivars->sim  = (Similarity*)INCREF(Schema_Fetch_Sim(schema, field));
        ivars->type = (FieldType*)INCREF(Schema_Fetch_Type(schema, field));
        if (!ivars->type) { THROW(ERR, "Unknown field: '%o'", field); }

        uint8_t prim_id = FType_Primitive_ID(ivars->type);
        switch (prim_id & FType_PRIMITIVE_ID_MASK) {
            case FType_TEXT:
                ivars->value = NULL;
                break;
            case FType_BLOB:
                ivars->value = (Obj*)ViewBB_new(NULL, 0);
                break;
            case FType_INT32:
                ivars->value = (Obj*)Int32_new(0);
                break;
            case FType_INT64:
                ivars->value = (Obj*)Int64_new(0);
                break;
            case FType_FLOAT32:
                ivars->value = (Obj*)Float32_new(0);
                break;
            case FType_FLOAT64:
                ivars->value = (Obj*)Float64_new(0);
                break;
            default:
                THROW(ERR, "Unrecognized primitive id: %i8", prim_id);
        }

        ivars->indexed = FType_Indexed(ivars->type);
        if (ivars->indexed && FType_Is_A(ivars->type, NUMERICTYPE)) {
            THROW(ERR, "Field '%o' spec'd as indexed, but numerical types cannot "
                  "be indexed yet", field);
        }
        if (FType_Is_A(ivars->type, FULLTEXTTYPE)) {
            ivars->highlightable
                = FullTextType_Highlightable((FullTextType*)ivars->type);
        }
    }
    return self;
}

void
InvEntry_Destroy_IMP(InverterEntry *self) {
    InverterEntryIVARS *const ivars = InvEntry_IVARS(self);
    DECREF(ivars->field);
    DECREF(ivars->value);
    DECREF(ivars->analyzer);
    DECREF(ivars->type);
    DECREF(ivars->sim);
    DECREF(ivars->inversion);
    SUPER_DESTROY(self, INVERTERENTRY);
}

void
InvEntry_Clear_IMP(InverterEntry *self) {
    InverterEntryIVARS *const ivars = InvEntry_IVARS(self);
    DECREF(ivars->inversion);
    ivars->inversion = NULL;
}

int32_t
InvEntry_Compare_To_IMP(InverterEntry *self, Obj *other) {
    CERTIFY(other, INVERTERENTRY);
    InverterEntryIVARS *const ivars = InvEntry_IVARS(self);
    InverterEntryIVARS *const ovars = InvEntry_IVARS((InverterEntry*)other);
    return ivars->field_num - ovars->field_num;
}


