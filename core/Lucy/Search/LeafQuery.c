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

#define C_LUCY_LEAFQUERY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/LeafQuery.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

LeafQuery*
LeafQuery_new(const CharBuf *field, const CharBuf *text) {
    LeafQuery *self = (LeafQuery*)VTable_Make_Obj(LEAFQUERY);
    return LeafQuery_init(self, field, text);
}

LeafQuery*
LeafQuery_init(LeafQuery *self, const CharBuf *field, const CharBuf *text) {
    Query_init((Query*)self, 1.0f);
    self->field       = field ? CB_Clone(field) : NULL;
    self->text        = CB_Clone(text);
    return self;
}

void
LeafQuery_destroy(LeafQuery *self) {
    DECREF(self->field);
    DECREF(self->text);
    SUPER_DESTROY(self, LEAFQUERY);
}

CharBuf*
LeafQuery_get_field(LeafQuery *self) {
    return self->field;
}

CharBuf*
LeafQuery_get_text(LeafQuery *self) {
    return self->text;
}

bool_t
LeafQuery_equals(LeafQuery *self, Obj *other) {
    LeafQuery *twin = (LeafQuery*)other;
    if (twin == self)                  { return true; }
    if (!Obj_Is_A(other, LEAFQUERY))   { return false; }
    if (self->boost != twin->boost)    { return false; }
    if (!!self->field ^ !!twin->field) { return false; }
    if (self->field) {
        if (!CB_Equals(self->field, (Obj*)twin->field)) { return false; }
    }
    if (!CB_Equals(self->text, (Obj*)twin->text)) { return false; }
    return true;
}

CharBuf*
LeafQuery_to_string(LeafQuery *self) {
    if (self->field) {
        return CB_newf("%o:%o", self->field, self->text);
    }
    else {
        return CB_Clone(self->text);
    }
}

void
LeafQuery_serialize(LeafQuery *self, OutStream *outstream) {
    if (self->field) {
        OutStream_Write_U8(outstream, true);
        CB_Serialize(self->field, outstream);
    }
    else {
        OutStream_Write_U8(outstream, false);
    }
    CB_Serialize(self->text, outstream);
    OutStream_Write_F32(outstream, self->boost);
}

LeafQuery*
LeafQuery_deserialize(LeafQuery *self, InStream *instream) {
    self = self ? self : (LeafQuery*)VTable_Make_Obj(LEAFQUERY);
    self->field = InStream_Read_U8(instream)
                  ? CB_deserialize(NULL, instream)
                  : NULL;
    self->text  = CB_deserialize(NULL, instream);
    self->boost = InStream_Read_F32(instream);
    return self;
}

Compiler*
LeafQuery_make_compiler(LeafQuery *self, Searcher *searcher, float boost,
                        bool_t subordinate) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    UNUSED_VAR(boost);
    UNUSED_VAR(subordinate);
    THROW(ERR, "Can't Make_Compiler() from LeafQuery");
    UNREACHABLE_RETURN(Compiler*);
}


