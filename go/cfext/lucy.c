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

DefDocReader_Fetch_Doc_t GOLUCY_DefDocReader_Fetch_Doc_BRIDGE;

HitDoc*
DefDocReader_Fetch_Doc_IMP(DefaultDocReader *self, int32_t doc_id) {
    return GOLUCY_DefDocReader_Fetch_Doc_BRIDGE(self, doc_id);
}

/**************************** Inverter *****************************/

Inverter_Invert_Doc_t GOLUCY_Inverter_Invert_Doc_BRIDGE;

void
Inverter_Invert_Doc_IMP(Inverter *self, Doc *doc) {
    GOLUCY_Inverter_Invert_Doc_BRIDGE(self, doc);
}


