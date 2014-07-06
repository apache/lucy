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
#include "Lucy/Util/Freezer.h"

LeafQuery*
LeafQuery_new(String *field, String *text) {
    LeafQuery *self = (LeafQuery*)Class_Make_Obj(LEAFQUERY);
    return LeafQuery_init(self, field, text);
}

LeafQuery*
LeafQuery_init(LeafQuery *self, String *field, String *text) {
    LeafQueryIVARS *const ivars = LeafQuery_IVARS(self);
    Query_init((Query*)self, 1.0f);
    ivars->field       = field ? Str_Clone(field) : NULL;
    ivars->text        = Str_Clone(text);
    return self;
}

void
LeafQuery_Destroy_IMP(LeafQuery *self) {
    LeafQueryIVARS *const ivars = LeafQuery_IVARS(self);
    DECREF(ivars->field);
    DECREF(ivars->text);
    SUPER_DESTROY(self, LEAFQUERY);
}

String*
LeafQuery_Get_Field_IMP(LeafQuery *self) {
    return LeafQuery_IVARS(self)->field;
}

String*
LeafQuery_Get_Text_IMP(LeafQuery *self) {
    return LeafQuery_IVARS(self)->text;
}

bool
LeafQuery_Equals_IMP(LeafQuery *self, Obj *other) {
    if ((LeafQuery*)other == self)     { return true; }
    if (!Obj_Is_A(other, LEAFQUERY))   { return false; }
    LeafQueryIVARS *const ivars = LeafQuery_IVARS(self);
    LeafQueryIVARS *const ovars = LeafQuery_IVARS((LeafQuery*)other);
    if (ivars->boost != ovars->boost)    { return false; }
    if (!!ivars->field ^ !!ovars->field) { return false; }
    if (ivars->field) {
        if (!Str_Equals(ivars->field, (Obj*)ovars->field)) { return false; }
    }
    if (!Str_Equals(ivars->text, (Obj*)ovars->text)) { return false; }
    return true;
}

String*
LeafQuery_To_String_IMP(LeafQuery *self) {
    LeafQueryIVARS *const ivars = LeafQuery_IVARS(self);
    if (ivars->field) {
        return Str_newf("%o:%o", ivars->field, ivars->text);
    }
    else {
        return Str_Clone(ivars->text);
    }
}

void
LeafQuery_Serialize_IMP(LeafQuery *self, OutStream *outstream) {
    LeafQueryIVARS *const ivars = LeafQuery_IVARS(self);
    if (ivars->field) {
        OutStream_Write_U8(outstream, true);
        Freezer_serialize_string(ivars->field, outstream);
    }
    else {
        OutStream_Write_U8(outstream, false);
    }
    Freezer_serialize_string(ivars->text, outstream);
    OutStream_Write_F32(outstream, ivars->boost);
}

LeafQuery*
LeafQuery_Deserialize_IMP(LeafQuery *self, InStream *instream) {
    LeafQueryIVARS *const ivars = LeafQuery_IVARS(self);
    if (InStream_Read_U8(instream)) {
        ivars->field = Freezer_read_string(instream);
    }
    else {
        ivars->field = NULL;
    }
    ivars->text = Freezer_read_string(instream);
    ivars->boost = InStream_Read_F32(instream);
    return self;
}

Obj*
LeafQuery_Dump_IMP(LeafQuery *self) {
    LeafQueryIVARS *ivars = LeafQuery_IVARS(self);
    LeafQuery_Dump_t super_dump
        = SUPER_METHOD_PTR(LEAFQUERY, LUCY_LeafQuery_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    if (ivars->field) {
        Hash_Store_Utf8(dump, "field", 5, Freezer_dump((Obj*)ivars->field));
    }
    Hash_Store_Utf8(dump, "text", 4, Freezer_dump((Obj*)ivars->text));
    return (Obj*)dump;
}

Obj*
LeafQuery_Load_IMP(LeafQuery *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    LeafQuery_Load_t super_load
        = SUPER_METHOD_PTR(LEAFQUERY, LUCY_LeafQuery_Load);
    LeafQuery *loaded = (LeafQuery*)super_load(self, dump);
    LeafQueryIVARS *loaded_ivars = LeafQuery_IVARS(loaded);
    Obj *field = Hash_Fetch_Utf8(source, "field", 5);
    if (field) {
        loaded_ivars->field
            = (String*)CERTIFY(Freezer_load(field), STRING);
    }
    Obj *text = CERTIFY(Hash_Fetch_Utf8(source, "text", 4), OBJ);
    loaded_ivars->text = (String*)CERTIFY(Freezer_load(text), STRING);
    return (Obj*)loaded;
}

Compiler*
LeafQuery_Make_Compiler_IMP(LeafQuery *self, Searcher *searcher, float boost,
                            bool subordinate) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    UNUSED_VAR(boost);
    UNUSED_VAR(subordinate);
    THROW(ERR, "Can't Make_Compiler() from LeafQuery");
    UNREACHABLE_RETURN(Compiler*);
}


