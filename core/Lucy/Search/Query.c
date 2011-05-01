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
    self->boost = boost;
    ABSTRACT_CLASS_CHECK(self, QUERY);
    return self;
}

void
Query_set_boost(Query *self, float boost) {
    self->boost = boost;
}

float
Query_get_boost(Query *self) {
    return self->boost;
}

void
Query_serialize(Query *self, OutStream *outstream) {
    OutStream_Write_F32(outstream, self->boost);
}

Query*
Query_deserialize(Query *self, InStream *instream) {
    float boost = InStream_Read_F32(instream);
    self = self ? self : (Query*)VTable_Make_Obj(QUERY);
    Query_init(self, boost);
    return self;
}


