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

#define C_LUCY_DOC

#include "CFBind.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

lucy_Doc*
lucy_Doc_init(lucy_Doc *self, void *fields, int32_t doc_id) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Doc*);
}

void
lucy_Doc_set_fields(lucy_Doc *self, void *fields) {
    THROW(LUCY_ERR, "TODO");
}

uint32_t
lucy_Doc_get_size(lucy_Doc *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(uint32_t);
}

void
lucy_Doc_store(lucy_Doc *self, const lucy_CharBuf *field, lucy_Obj *value) {
    THROW(LUCY_ERR, "TODO");
}

void
lucy_Doc_serialize(lucy_Doc *self, lucy_OutStream *outstream) {
    THROW(LUCY_ERR, "TODO");
}

lucy_Doc*
lucy_Doc_deserialize(lucy_Doc *self, lucy_InStream *instream) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Doc*);
}

lucy_Obj*
lucy_Doc_extract(lucy_Doc *self, lucy_CharBuf *field) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Obj*);
}

void*
lucy_Doc_to_host(lucy_Doc *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

lucy_Hash*
lucy_Doc_dump(lucy_Doc *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Hash*);
}

lucy_Doc*
lucy_Doc_load(lucy_Doc *self, lucy_Obj *dump) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Doc*);
}

bool
lucy_Doc_equals(lucy_Doc *self, lucy_Obj *other) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(bool);
}

void
lucy_Doc_destroy(lucy_Doc *self) {
    THROW(LUCY_ERR, "TODO");
}


