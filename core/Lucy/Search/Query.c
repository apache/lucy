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

#define C_LUCY_QUERY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Query.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

Query*
Query_init(Query *self, float boost) {
    Query_IVARS(self)->boost = boost;
    ABSTRACT_CLASS_CHECK(self, QUERY);
    return self;
}

void
Query_Set_Boost_IMP(Query *self, float boost) {
    Query_IVARS(self)->boost = boost;
}

float
Query_Get_Boost_IMP(Query *self) {
    return Query_IVARS(self)->boost;
}

void
Query_Serialize_IMP(Query *self, OutStream *outstream) {
    OutStream_Write_F32(outstream, Query_IVARS(self)->boost);
}

Query*
Query_Deserialize_IMP(Query *self, InStream *instream) {
    float boost = InStream_Read_F32(instream);
    return Query_init(self, boost);
}

Obj*
Query_Dump_IMP(Query *self) {
    QueryIVARS *ivars = Query_IVARS(self);
    Hash *dump = Hash_new(0);
    Hash_Store_Utf8(dump, "_class", 6,
                    (Obj*)Str_Clone(Obj_Get_Class_Name((Obj*)self)));
    Hash_Store_Utf8(dump, "boost", 5,
                    (Obj*)Str_newf("%f64", (double)ivars->boost));
    return (Obj*)dump;
}

Obj*
Query_Load_IMP(Query *self, Obj *dump) {
    UNUSED_VAR(self);
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    String *class_name
        = (String*)CERTIFY(Hash_Fetch_Utf8(source, "_class", 6), STRING);
    Class *klass  = Class_singleton(class_name, NULL);
    Query *loaded = (Query*)Class_Make_Obj(klass);
    Obj *boost = CERTIFY(Hash_Fetch_Utf8(source, "boost", 5), OBJ);
    Query_IVARS(loaded)->boost = (float)Obj_To_F64(boost);
    return (Obj*)loaded;
}

