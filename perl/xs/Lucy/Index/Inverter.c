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

#define CFP_LUCY
#define C_LUCY_INVERTER
#define C_LUCY_INVERTERENTRY
#include "XSBind.h"
#include "Lucy/Index/Inverter.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Segment.h"
#include "Clownfish/Blob.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/BlobType.h"
#include "Lucy/Plan/NumericType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Plan/TextType.h"
#include "Clownfish/Util/StringHelper.h"

static lucy_InverterEntry*
S_fetch_entry(pTHX_ lucy_Inverter *self, HE *hash_entry) {
    lucy_InverterIVARS *const ivars = lucy_Inverter_IVARS(self);
    lucy_Schema *const schema = ivars->schema;
    STRLEN key_size;
    const char *key = XSBind_hash_key_to_utf8(aTHX_ hash_entry, &key_size);

    cfish_String *field = CFISH_SSTR_WRAP_UTF8(key, key_size);
    int32_t field_num = LUCY_Seg_Field_Num(ivars->segment, field);
    if (!field_num) {
        // This field seems not to be in the segment yet.  Try to find it in
        // the Schema.
        if (LUCY_Schema_Fetch_Type(schema, field)) {
            // The field is in the Schema.  Get a field num from the Segment.
            field_num = LUCY_Seg_Add_Field(ivars->segment, field);
        }
        else {
            // We've truly failed to find the field.  The user must
            // not have spec'd it.
            THROW(CFISH_ERR, "Unknown field name: '%s'", key);
        }
    }

    lucy_InverterEntry *entry
        = (lucy_InverterEntry*)CFISH_Vec_Fetch(ivars->entry_pool, field_num);
    if (!entry) {
        entry = lucy_InvEntry_new(schema, field, field_num);
        CFISH_Vec_Store(ivars->entry_pool, field_num, (cfish_Obj*)entry);
    }
    return entry;
}

void
LUCY_Inverter_Invert_Doc_IMP(lucy_Inverter *self, lucy_Doc *doc) {
    dTHX;
    HV  *const fields = (HV*)LUCY_Doc_Get_Fields(doc);
    I32  num_keys     = hv_iterinit(fields);

    // Prepare for the new doc.
    LUCY_Inverter_Set_Doc(self, doc);

    // Extract and invert the doc's fields.
    while (num_keys--) {
        HE *hash_entry = hv_iternext(fields);
        lucy_InverterEntry *inv_entry = S_fetch_entry(aTHX_ self, hash_entry);
        SV *value_sv = HeVAL(hash_entry);
        lucy_InverterEntryIVARS *const entry_ivars
            = lucy_InvEntry_IVARS(inv_entry);
        lucy_FieldType *type = entry_ivars->type;
        cfish_Obj *obj = NULL;

        // Get the field value, forcing text fields to UTF-8.
        switch (LUCY_FType_Primitive_ID(type) & lucy_FType_PRIMITIVE_ID_MASK) {
            case lucy_FType_TEXT: {
                    STRLEN val_len;
                    char *val_ptr = SvPVutf8(value_sv, val_len);
                    obj = (cfish_Obj*)cfish_Str_new_wrap_trusted_utf8(val_ptr,
                                                                      val_len);
                    break;
                }
            case lucy_FType_BLOB: {
                    STRLEN val_len;
                    char *val_ptr = SvPV(value_sv, val_len);
                    obj = (cfish_Obj*)cfish_Blob_new_wrap(val_ptr, val_len);
                    break;
                }
            case lucy_FType_INT32: {
                    obj = (cfish_Obj*)cfish_Int_new(SvIV(value_sv));
                    break;
                }
            case lucy_FType_INT64: {
                    // nwellnhof: Using SvNOK could avoid a int/float/int
                    // round-trip with 32-bit IVs.
                    int64_t val = sizeof(IV) == 8
                                  ? SvIV(value_sv)
                                  : (int64_t)SvNV(value_sv); // lossy
                    obj = (cfish_Obj*)cfish_Int_new(val);
                    break;
                }
            case lucy_FType_FLOAT32:
            case lucy_FType_FLOAT64: {
                    obj = (cfish_Obj*)cfish_Float_new(SvNV(value_sv));
                    break;
                }
            default:
                THROW(CFISH_ERR, "Unrecognized type: %o", type);
        }

        CFISH_DECREF(entry_ivars->value);
        entry_ivars->value = obj;

        LUCY_Inverter_Add_Field(self, inv_entry);
    }
}


