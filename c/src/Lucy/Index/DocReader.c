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
#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "Lucy/Index/DocReader.h"
#include "Clownfish/ByteBuf.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/Num.h"
#include "Clownfish/Util/Memory.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"

HitDoc*
DefDocReader_fetch_doc(DefaultDocReader *self, int32_t doc_id) {
    DefaultDocReaderIVARS *const ivars = DefDocReader_IVARS(self);
    Schema   *const schema = ivars->schema;
    InStream *const dat_in = ivars->dat_in;
    InStream *const ix_in  = ivars->ix_in;
    Hash     *const fields = Hash_new(1);
    int64_t   start;
    uint32_t  num_fields;
    uint32_t  field_name_cap = 31;
    char     *field_name = (char*)MALLOCATE(field_name_cap + 1);

    // Get data file pointer from index, read number of fields.
    InStream_Seek(ix_in, (int64_t)doc_id * 8);
    start = InStream_Read_U64(ix_in);
    InStream_Seek(dat_in, start);
    num_fields = InStream_Read_C32(dat_in);

    // Decode stored data and build up the doc field by field.
    while (num_fields--) {
        uint32_t        field_name_len;
        Obj       *value;
        FieldType *type;

        // Read field name.
        field_name_len = InStream_Read_C32(dat_in);
        if (field_name_len > field_name_cap) {
            field_name_cap = field_name_len;
            field_name     = (char*)REALLOCATE(field_name,
                                                    field_name_cap + 1);
        }
        InStream_Read_Bytes(dat_in, field_name, field_name_len);

        // Find the Field's FieldType.
        ZombieCharBuf *field_name_zcb
            = ZCB_WRAP_STR(field_name, field_name_len);
        type = Schema_Fetch_Type(schema, (CharBuf*)field_name_zcb);

        // Read the field value.
        switch (FType_Primitive_ID(type) & FType_PRIMITIVE_ID_MASK) {
            case FType_TEXT: {
                    uint32_t value_len = InStream_Read_C32(dat_in);
                    char *buf = (char*)MALLOCATE(value_len + 1);
                    InStream_Read_Bytes(dat_in, buf, value_len);
                    buf[value_len] = '\0'; 
                    value = (Obj*)CB_new_steal_from_trusted_str(
                                buf, value_len, value_len + 1);
                    break;
                }
            case FType_BLOB: {
                    uint32_t value_len = InStream_Read_C32(dat_in);
                    char *buf = (char*)MALLOCATE(value_len);
                    InStream_Read_Bytes(dat_in, buf, value_len);
                    value = (Obj*)BB_new_steal_bytes(
                                buf, value_len, value_len);
                    break;
                }
            case FType_FLOAT32:
                value = (Obj*)Float32_new(
                                InStream_Read_F32(dat_in));
                break;
            case FType_FLOAT64:
                value = (Obj*)Float64_new(
                                InStream_Read_F64(dat_in));
                break;
            case FType_INT32:
                value = (Obj*)Int32_new(
                                (int32_t)InStream_Read_C32(dat_in));
                break;
            case FType_INT64:
                value = (Obj*)Int64_new(
                                (int64_t)InStream_Read_C64(dat_in));
                break;
            default:
                value = NULL;
                THROW(ERR, "Unrecognized type: %o", type);
        }

        // Store the value.
        Hash_Store_Str(fields, field_name, field_name_len, value);
    }
    FREEMEM(field_name);

    HitDoc *retval = HitDoc_new(fields, doc_id, 0.0);
    DECREF(fields);
    return retval;
}

