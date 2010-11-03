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

#define C_LUCY_DOCREADER
#define C_LUCY_DEFAULTDOCREADER
#define C_LUCY_ZOMBIECHARBUF
#include "xs/XSBind.h"

#include "KinoSearch/Index/DocReader.h"
#include "KinoSearch/Document/HitDoc.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/BlobType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Plan/TextType.h"
#include "KinoSearch/Plan/NumericType.h"
#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Store/InStream.h"

kino_HitDoc*
kino_DefDocReader_fetch_doc(kino_DefaultDocReader *self, int32_t doc_id)
{
    kino_Schema   *const schema = self->schema;
    kino_InStream *const dat_in = self->dat_in;
    kino_InStream *const ix_in  = self->ix_in;
    HV *fields = newHV();
    int64_t start;
    uint32_t num_fields;
    SV *field_name_sv = newSV(1);

    // Get data file pointer from index, read number of fields. 
    Kino_InStream_Seek(ix_in, (int64_t)doc_id * 8);
    start = Kino_InStream_Read_U64(ix_in);
    Kino_InStream_Seek(dat_in, start);
    num_fields = Kino_InStream_Read_C32(dat_in);

    // Decode stored data and build up the doc field by field. 
    while (num_fields--) {
        STRLEN  field_name_len;
        char   *field_name_ptr;
        SV     *value_sv;
        kino_FieldType *type;

        // Read field name. 
        field_name_len = Kino_InStream_Read_C32(dat_in);
        field_name_ptr = SvGROW(field_name_sv, field_name_len + 1);
        Kino_InStream_Read_Bytes(dat_in, field_name_ptr, field_name_len);
        SvPOK_on(field_name_sv);
        SvCUR_set(field_name_sv, field_name_len);
        SvUTF8_on(field_name_sv);
        *SvEND(field_name_sv) = '\0';

        // Find the Field's FieldType. 
        kino_ZombieCharBuf *field_name_zcb 
            = KINO_ZCB_WRAP_STR(field_name_ptr, field_name_len);
        Kino_ZCB_Assign_Str(field_name_zcb, field_name_ptr, field_name_len);
        type = Kino_Schema_Fetch_Type(schema, (kino_CharBuf*)field_name_zcb);

        // Read the field value. 
        switch(Kino_FType_Primitive_ID(type) & lucy_FType_PRIMITIVE_ID_MASK) {
            case lucy_FType_TEXT: {
                STRLEN  value_len = Kino_InStream_Read_C32(dat_in);
                value_sv  = newSV((value_len ? value_len : 1));
                Kino_InStream_Read_Bytes(dat_in, SvPVX(value_sv), value_len);
                SvCUR_set(value_sv, value_len);
                *SvEND(value_sv) = '\0';
                SvPOK_on(value_sv);
                SvUTF8_on(value_sv);
                break;
            }
            case lucy_FType_BLOB: {
                STRLEN  value_len = Kino_InStream_Read_C32(dat_in);
                value_sv  = newSV((value_len ? value_len : 1));
                Kino_InStream_Read_Bytes(dat_in, SvPVX(value_sv), value_len);
                SvCUR_set(value_sv, value_len);
                *SvEND(value_sv) = '\0';
                SvPOK_on(value_sv);
                break;
            }
            case lucy_FType_FLOAT32:
                value_sv = newSVnv(Kino_InStream_Read_F32(dat_in));
                break;
            case lucy_FType_FLOAT64:
                value_sv = newSVnv(Kino_InStream_Read_F64(dat_in));
                break;
            case lucy_FType_INT32:
                value_sv = newSViv((int32_t)Kino_InStream_Read_C32(dat_in));
                break;
            case lucy_FType_INT64:
                if (sizeof(IV) == 8) {
                    int64_t val = (int64_t)Kino_InStream_Read_C64(dat_in);
                    value_sv = newSViv((IV)val);
                }
                else { // (lossy) 
                    int64_t val = (int64_t)Kino_InStream_Read_C64(dat_in);
                    value_sv = newSVnv((double)val);
                }
                break;
            default:
                value_sv = NULL; 
                CFISH_THROW(KINO_ERR, "Unrecognized type: %o", type);
        }

        // Store the value. 
        hv_store_ent(fields, field_name_sv, value_sv, 0);
    }
    SvREFCNT_dec(field_name_sv);

    {
        kino_HitDoc *retval = kino_HitDoc_new(fields, doc_id, 0.0);
        SvREFCNT_dec((SV*)fields);
        return retval;
    }
}


