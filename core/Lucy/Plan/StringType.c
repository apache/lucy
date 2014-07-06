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

#define C_LUCY_STRINGTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/StringType.h"
#include "Lucy/Index/Posting/ScorePosting.h"
#include "Lucy/Index/Similarity.h"

StringType*
StringType_new() {
    StringType *self = (StringType*)Class_Make_Obj(STRINGTYPE);
    return StringType_init(self);
}

StringType*
StringType_init(StringType *self) {
    return StringType_init2(self, 1.0, true, true, false);
}

StringType*
StringType_init2(StringType *self, float boost, bool indexed,
                 bool stored, bool sortable) {
    FType_init((FieldType*)self);
    StringTypeIVARS *const ivars = StringType_IVARS(self);
    ivars->boost      = boost;
    ivars->indexed    = indexed;
    ivars->stored     = stored;
    ivars->sortable   = sortable;
    return self;
}

bool
StringType_Equals_IMP(StringType *self, Obj *other) {
    if ((StringType*)other == self) { return true; }
    StringType_Equals_t super_equals
        = (StringType_Equals_t)SUPER_METHOD_PTR(STRINGTYPE,
                                                LUCY_StringType_Equals);
    if (!super_equals(self, other)) { return false; }
    return true;
}

Hash*
StringType_Dump_For_Schema_IMP(StringType *self) {
    StringTypeIVARS *const ivars = StringType_IVARS(self);
    Hash *dump = Hash_new(0);
    Hash_Store_Utf8(dump, "type", 4, (Obj*)Str_newf("string"));

    // Store attributes that override the defaults.
    if (ivars->boost != 1.0) {
        Hash_Store_Utf8(dump, "boost", 5, (Obj*)Str_newf("%f64", ivars->boost));
    }
    if (!ivars->indexed) {
        Hash_Store_Utf8(dump, "indexed", 7, (Obj*)CFISH_FALSE);
    }
    if (!ivars->stored) {
        Hash_Store_Utf8(dump, "stored", 6, (Obj*)CFISH_FALSE);
    }
    if (ivars->sortable) {
        Hash_Store_Utf8(dump, "sortable", 8, (Obj*)CFISH_TRUE);
    }

    return dump;
}

Hash*
StringType_Dump_IMP(StringType *self) {
    Hash *dump = StringType_Dump_For_Schema(self);
    Hash_Store_Utf8(dump, "_class", 6,
                    (Obj*)Str_Clone(StringType_Get_Class_Name(self)));
    DECREF(Hash_Delete_Utf8(dump, "type", 4));
    return dump;
}

StringType*
StringType_Load_IMP(StringType *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    String *class_name = (String*)Hash_Fetch_Utf8(source, "_class", 6);
    Class *klass
        = (class_name != NULL && Obj_Is_A((Obj*)class_name, STRING))
          ? Class_singleton(class_name, NULL)
          : STRINGTYPE;
    StringType *loaded   = (StringType*)Class_Make_Obj(klass);
    Obj *boost_dump      = Hash_Fetch_Utf8(source, "boost", 5);
    Obj *indexed_dump    = Hash_Fetch_Utf8(source, "indexed", 7);
    Obj *stored_dump     = Hash_Fetch_Utf8(source, "stored", 6);
    Obj *sortable_dump   = Hash_Fetch_Utf8(source, "sortable", 8);
    UNUSED_VAR(self);

    float boost    = boost_dump    ? (float)Obj_To_F64(boost_dump) : 1.0f;
    bool  indexed  = indexed_dump  ? Obj_To_Bool(indexed_dump)     : true;
    bool  stored   = stored_dump   ? Obj_To_Bool(stored_dump)      : true;
    bool  sortable = sortable_dump ? Obj_To_Bool(sortable_dump)    : false;

    return StringType_init2(loaded, boost, indexed, stored, sortable);
}

Similarity*
StringType_Make_Similarity_IMP(StringType *self) {
    UNUSED_VAR(self);
    return Sim_new();
}

Posting*
StringType_Make_Posting_IMP(StringType *self, Similarity *similarity) {
    if (similarity) {
        return (Posting*)ScorePost_new(similarity);
    }
    else {
        Similarity *sim = StringType_Make_Similarity(self);
        Posting *posting = (Posting*)ScorePost_new(sim);
        DECREF(sim);
        return posting;
    }
}


