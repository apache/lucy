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
#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "Lucy/Index/Inverter.h"
#include "Clownfish/ByteBuf.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/Num.h"
#include "Clownfish/VArray.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"

static InverterEntry*
S_fetch_entry(Inverter *self, CharBuf *field) {
    Schema *const schema = self->schema;
    int32_t field_num = Seg_Field_Num(self->segment, field);
    if (!field_num) {
        // This field seems not to be in the segment yet.  Try to find it in
        // the Schema.
        if (Schema_Fetch_Type(schema, field)) {
            // The field is in the Schema.  Get a field num from the Segment.
            field_num = Seg_Add_Field(self->segment, field);
        }
        else {
            // We've truly failed to find the field.  The user must
            // not have spec'd it.
            THROW(ERR, "Unknown field name: '%o'", field);
        }
    }

    InverterEntry *entry
        = (InverterEntry*)VA_Fetch(self->entry_pool, field_num);
    if (!entry) {
        entry = InvEntry_new(schema, (CharBuf*)field, field_num);
        VA_Store(self->entry_pool, field_num, (Obj*)entry);
    }
    return entry;
}

void
Inverter_invert_doc(Inverter *self, Doc *doc) {
    Hash *const fields = (Hash*)Doc_Get_Fields(doc);
    uint32_t   num_keys     = Hash_Iterate(fields);

    // Prepare for the new doc.
    Inverter_Set_Doc(self, doc);

    // Extract and invert the doc's fields.
    while (num_keys--) {
        Obj *key, *obj;
        Hash_Next(fields, &key, &obj);
        CharBuf *field = (CharBuf*)CERTIFY(key, CHARBUF);
        InverterEntry *inv_entry = S_fetch_entry(self, field);
        FieldType *type = inv_entry->type;

        // Get the field value.
        switch (FType_Primitive_ID(type) & FType_PRIMITIVE_ID_MASK) {
            case FType_TEXT: {
                    CharBuf *char_buf
                        = (CharBuf*)CERTIFY(obj, CHARBUF);
                    ViewCharBuf *value
                        = (ViewCharBuf*)inv_entry->value;
                    ViewCB_Assign(value, char_buf);
                    break;
                }
            case FType_BLOB: {
                    ByteBuf *byte_buf
                        = (ByteBuf*)CERTIFY(obj, BYTEBUF);
                    ViewByteBuf *value
                        = (ViewByteBuf*)inv_entry->value;
                    ViewBB_Assign(value, byte_buf);
                    break;
                }
            case FType_INT32: {
                    int32_t int_val = (int32_t)Obj_To_I64(obj);
                    Integer32* value = (Integer32*)inv_entry->value;
                    Int32_Set_Value(value, int_val);
                    break;
                }
            case FType_INT64: {
                    int64_t int_val = Obj_To_I64(obj);
                    Integer64* value = (Integer64*)inv_entry->value;
                    Int64_Set_Value(value, int_val);
                    break;
                }
            case FType_FLOAT32: {
                    float float_val = (float)Obj_To_F64(obj);
                    Float32* value = (Float32*)inv_entry->value;
                    Float32_Set_Value(value, float_val);
                    break;
                }
            case FType_FLOAT64: {
                    double float_val = Obj_To_F64(obj);
                    Float64* value = (Float64*)inv_entry->value;
                    Float64_Set_Value(value, float_val);
                    break;
                }
            default:
                THROW(ERR, "Unrecognized type: %o", type);
        }

        Inverter_Add_Field(self, inv_entry);
    }
}


