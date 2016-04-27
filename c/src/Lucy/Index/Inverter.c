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
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "Lucy/Index/Inverter.h"
#include "Clownfish/Blob.h"
#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/HashIterator.h"
#include "Clownfish/Num.h"
#include "Clownfish/Vector.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"

static InverterEntry*
S_fetch_entry(InverterIVARS *ivars, String *field) {
    Schema *const schema = ivars->schema;
    int32_t field_num = Seg_Field_Num(ivars->segment, field);
    if (!field_num) {
        // This field seems not to be in the segment yet.  Try to find it in
        // the Schema.
        if (Schema_Fetch_Type(schema, field)) {
            // The field is in the Schema.  Get a field num from the Segment.
            field_num = Seg_Add_Field(ivars->segment, field);
        }
        else {
            // We've truly failed to find the field.  The user must
            // not have spec'd it.
            THROW(ERR, "Unknown field name: '%o'", field);
        }
    }

    InverterEntry *entry
        = (InverterEntry*)Vec_Fetch(ivars->entry_pool, (size_t)field_num);
    if (!entry) {
        entry = InvEntry_new(schema, (String*)field, field_num);
        Vec_Store(ivars->entry_pool, (size_t)field_num, (Obj*)entry);
    }
    return entry;
}

void
Inverter_Invert_Doc_IMP(Inverter *self, Doc *doc) {
    InverterIVARS *const ivars = Inverter_IVARS(self);
    Hash *const fields = (Hash*)Doc_Get_Fields(doc);

    // Prepare for the new doc.
    Inverter_Set_Doc(self, doc);

    // Extract and invert the doc's fields.
    HashIterator *iter = HashIter_new(fields);
    while (HashIter_Next(iter)) {
        String *field = HashIter_Get_Key(iter);
        Obj    *obj   = HashIter_Get_Value(iter);

        InverterEntry *inventry = S_fetch_entry(ivars, field);
        InverterEntryIVARS *inventry_ivars = InvEntry_IVARS(inventry);
        FieldType *type = inventry_ivars->type;

        // Get the field value.
        switch (FType_Primitive_ID(type) & FType_PRIMITIVE_ID_MASK) {
            case FType_TEXT: {
                    CERTIFY(obj, STRING);
                    break;
                }
            case FType_BLOB: {
                    CERTIFY(obj, BLOB);
                    break;
                }
            case FType_INT32:
            case FType_INT64: {
                    CERTIFY(obj, INTEGER);
                    break;
                }
            case FType_FLOAT32:
            case FType_FLOAT64: {
                    CERTIFY(obj, FLOAT);
                    break;
                }
            default:
                THROW(ERR, "Unrecognized type: %o", type);
        }

        if (inventry_ivars->value != obj) {
            DECREF(inventry_ivars->value);
            inventry_ivars->value = INCREF(obj);
        }

        Inverter_Add_Field(self, inventry);
    }
    DECREF(iter);
}


