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

#define C_KINO_INVERTER
#define C_KINO_ZOMBIECHARBUF
#define C_KINO_INVERTERENTRY
#include "xs/XSBind.h"
#include "KinoSearch/Index/Inverter.h"
#include "KinoSearch/Document/Doc.h"
#include "KinoSearch/Index/Segment.h"
#include "KinoSearch/Object/ByteBuf.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/BlobType.h"
#include "KinoSearch/Plan/NumericType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Plan/TextType.h"
#include "KinoSearch/Util/StringHelper.h"

static kino_InverterEntry*
S_fetch_entry(kino_Inverter *self, HE *hash_entry)
{
    kino_Schema *const schema = self->schema;
    char *key;
    STRLEN key_len;
    STRLEN he_key_len = HeKLEN(hash_entry);

    // Force field name to UTF-8 if necessary.
    if (he_key_len == (STRLEN)HEf_SVKEY) {
        SV *key_sv = HeKEY_sv(hash_entry);
        key = SvPVutf8(key_sv, key_len);
    }
    else {
        key = HeKEY(hash_entry);
        key_len = he_key_len;
        if (!kino_StrHelp_utf8_valid(key, key_len)) {
            SV *key_sv = HeSVKEY_force(hash_entry);
            key = SvPVutf8(key_sv, key_len);
        }
    }

    kino_ZombieCharBuf *field = KINO_ZCB_WRAP_STR(key, key_len);
    int32_t field_num 
        = Kino_Seg_Field_Num(self->segment, (kino_CharBuf*)field);
    if (!field_num) {
        // This field seems not to be in the segment yet.  Try to find it in
        // the Schema.
        if (Kino_Schema_Fetch_Type(schema, (kino_CharBuf*)field)) {
            // The field is in the Schema.  Get a field num from the Segment. 
            field_num = Kino_Seg_Add_Field(self->segment,
                (kino_CharBuf*)field);
        }
        else {
            // We've truly failed to find the field.  The user must
            // not have spec'd it.
            THROW(KINO_ERR, "Unknown field name: '%s'", key);
        }
    }

    {
        kino_InverterEntry *entry 
            = (kino_InverterEntry*)Kino_VA_Fetch(self->entry_pool, field_num);
        if (!entry) {
            entry 
                = kino_InvEntry_new(schema, (kino_CharBuf*)field, field_num);
            Kino_VA_Store(self->entry_pool, field_num, (kino_Obj*)entry);
        }
        return entry;
    }
}

void
kino_Inverter_invert_doc(kino_Inverter *self, kino_Doc *doc)
{
    HV  *const fields = (HV*)Kino_Doc_Get_Fields(doc);
    I32  num_keys     = hv_iterinit(fields);

    // Prepare for the new doc. 
    Kino_Inverter_Set_Doc(self, doc);

    // Extract and invert the doc's fields. 
    while (num_keys--) {
        HE *hash_entry = hv_iternext(fields);
        kino_InverterEntry *inv_entry = S_fetch_entry(self, hash_entry);
        SV *value_sv = HeVAL(hash_entry);
        kino_FieldType *type = inv_entry->type;

        // Get the field value, forcing text fields to UTF-8. 
        switch (
            Kino_FType_Primitive_ID(type) & kino_FType_PRIMITIVE_ID_MASK
        ) {
            case kino_FType_TEXT: {
                STRLEN val_len;
                char *val_ptr = SvPVutf8(value_sv, val_len);
                kino_ViewCharBuf *value = (kino_ViewCharBuf*)inv_entry->value;
                Kino_ViewCB_Assign_Str(value, val_ptr, val_len);
                break;
            }
            case kino_FType_BLOB: {
                STRLEN val_len;
                char *val_ptr = SvPV(value_sv, val_len);
                kino_ViewByteBuf *value = (kino_ViewByteBuf*)inv_entry->value;
                Kino_ViewBB_Assign_Bytes(value, val_ptr, val_len);
                break;
            }
            case kino_FType_INT32: {
                kino_Integer32* value = (kino_Integer32*)inv_entry->value;
                Kino_Int32_Set_Value(value, SvIV(value_sv));
                break;
            }
            case kino_FType_INT64: {
                kino_Integer64* value = (kino_Integer64*)inv_entry->value;
                int64_t val = sizeof(IV) == 8 
                              ? SvIV(value_sv) 
                              : (int64_t)SvNV(value_sv); // lossy 
                Kino_Int64_Set_Value(value, val);
                break;
            }
            case kino_FType_FLOAT32: {
                kino_Float32* value = (kino_Float32*)inv_entry->value;
                Kino_Float32_Set_Value(value, (float)SvNV(value_sv));
                break;
            }
            case kino_FType_FLOAT64: {
                kino_Float64* value = (kino_Float64*)inv_entry->value;
                Kino_Float64_Set_Value(value, SvNV(value_sv));
                break;
            }
            default:
                THROW(KINO_ERR, "Unrecognized type: %o", type);
        }

        Kino_Inverter_Add_Field(self, inv_entry);
    }
}


