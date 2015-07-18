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



#define C_LUCY_REGEXTOKENIZER
#define C_LUCY_DOC
#define C_LUCY_DOCREADER
#define C_LUCY_DEFAULTDOCREADER
#define C_LUCY_INVERTER
#define C_LUCY_INVERTERENTRY
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES



#include <string.h>

#include "charmony.h"

#include "Lucy/Analysis/RegexTokenizer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/Inverter.h"
#include "Clownfish/Blob.h"
#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/HashIterator.h"
#include "Clownfish/Num.h"
#include "Clownfish/Vector.h"
#include "Clownfish/Class.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/Util/StringHelper.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

bool
RegexTokenizer_is_available(void) {
    return false;
}

RegexTokenizer*
(*GOLUCY_RegexTokenizer_init_BRIDGE)(RegexTokenizer *self, String *pattern);

RegexTokenizer*
RegexTokenizer_init(RegexTokenizer *self, String *pattern) {
    return GOLUCY_RegexTokenizer_init_BRIDGE(self, pattern);
}

RegexTokenizer_Destroy_t GOLUCY_RegexTokenizer_Destroy_BRIDGE;

void
RegexTokenizer_Destroy_IMP(RegexTokenizer *self) {
    GOLUCY_RegexTokenizer_Destroy_BRIDGE(self);
}

RegexTokenizer_Tokenize_Utf8_t GOLUCY_RegexTokenizer_Tokenize_Utf8_BRIDGE;

void
RegexTokenizer_Tokenize_Utf8_IMP(RegexTokenizer *self, const char *string,
                                 size_t string_len, Inversion *inversion) {
    GOLUCY_RegexTokenizer_Tokenize_Utf8_BRIDGE(self, string, string_len, inversion);
}

/********************************** Doc ********************************/

Doc*
(*GOLUCY_Doc_init_BRIDGE)(Doc *self, void *fields, int32_t doc_id);

Doc*
Doc_init(Doc *self, void *fields, int32_t doc_id) {
    return GOLUCY_Doc_init_BRIDGE(self, fields, doc_id);
}

Doc_Set_Fields_t GOLUCY_Doc_Set_Fields_BRIDGE;

void
Doc_Set_Fields_IMP(Doc *self, void *fields) {
    GOLUCY_Doc_Set_Fields_BRIDGE(self, fields);
}

Doc_Get_Size_t GOLUCY_Doc_Get_Size_BRIDGE;

uint32_t
Doc_Get_Size_IMP(Doc *self) {
    return GOLUCY_Doc_Get_Size_BRIDGE(self);
}

Doc_Store_t GOLUCY_Doc_Store_BRIDGE;

void
Doc_Store_IMP(Doc *self, String *field, Obj *value) {
    GOLUCY_Doc_Store_BRIDGE(self, field, value);
}

Doc_Serialize_t GOLUCY_Doc_Serialize_BRIDGE;

void
Doc_Serialize_IMP(Doc *self, OutStream *outstream) {
    GOLUCY_Doc_Serialize_BRIDGE(self, outstream);
}

Doc_Deserialize_t GOLUCY_Doc_Deserialize_BRIDGE;

Doc*
Doc_Deserialize_IMP(Doc *self, InStream *instream) {
    return GOLUCY_Doc_Deserialize_BRIDGE(self, instream);
}

Doc_Extract_t GOLUCY_Doc_Extract_BRIDGE;

Obj*
Doc_Extract_IMP(Doc *self, String *field) {
    return GOLUCY_Doc_Extract_BRIDGE(self, field);
}

Hash*
Doc_Dump_IMP(Doc *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(Hash*);
}

Doc*
Doc_Load_IMP(Doc *self, Obj *dump) {
    UNUSED_VAR(self);
    UNUSED_VAR(dump);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(Doc*);
}

Doc_Equals_t GOLUCY_Doc_Equals_BRIDGE;

bool
Doc_Equals_IMP(Doc *self, Obj *other) {
    return GOLUCY_Doc_Equals_BRIDGE(self, other);
}

Doc_Destroy_t GOLUCY_Doc_Destroy_BRIDGE;

void
Doc_Destroy_IMP(Doc *self) {
    GOLUCY_Doc_Destroy_BRIDGE(self);
}

/**************************** DocReader *****************************/

HitDoc*
DefDocReader_Fetch_Doc_IMP(DefaultDocReader *self, int32_t doc_id) {
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
        String *field_name_str = SSTR_WRAP_UTF8(field_name, field_name_len);
        type = Schema_Fetch_Type(schema, field_name_str);

        // Read the field value.
        switch (FType_Primitive_ID(type) & FType_PRIMITIVE_ID_MASK) {
            case FType_TEXT: {
                    uint32_t value_len = InStream_Read_C32(dat_in);
                    char *buf = (char*)MALLOCATE(value_len + 1);
                    InStream_Read_Bytes(dat_in, buf, value_len);
                    buf[value_len] = '\0';
                    value = (Obj*)Str_new_steal_utf8(buf, value_len);
                    break;
                }
            case FType_BLOB: {
                    uint32_t value_len = InStream_Read_C32(dat_in);
                    char *buf = (char*)MALLOCATE(value_len);
                    InStream_Read_Bytes(dat_in, buf, value_len);
                    value = (Obj*)Blob_new_steal(buf, value_len);
                    break;
                }
            case FType_FLOAT32:
                value = (Obj*)Float_new(InStream_Read_F32(dat_in));
                break;
            case FType_FLOAT64:
                value = (Obj*)Float_new(InStream_Read_F64(dat_in));
                break;
            case FType_INT32:
                value = (Obj*)Int_new((int32_t)InStream_Read_C32(dat_in));
                break;
            case FType_INT64:
                value = (Obj*)Int_new((int64_t)InStream_Read_C64(dat_in));
                break;
            default:
                value = NULL;
                THROW(ERR, "Unrecognized type: %o", type);
        }

        // Store the value.
        Hash_Store_Utf8(fields, field_name, field_name_len, value);
    }
    FREEMEM(field_name);

    HitDoc *retval = HitDoc_new(fields, doc_id, 0.0);
    DECREF(fields);
    return retval;
}

/**************************** Inverter *****************************/

Inverter_Invert_Doc_t GOLUCY_Inverter_Invert_Doc_BRIDGE;

void
Inverter_Invert_Doc_IMP(Inverter *self, Doc *doc) {
    GOLUCY_Inverter_Invert_Doc_BRIDGE(self, doc);
}


