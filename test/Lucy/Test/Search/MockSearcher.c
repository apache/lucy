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

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/Search/MockSearcher.h"
#include "Lucy/Test/TestSchema.h"

MockSearcher*
MockSearcher_new() {
    MockSearcher *self = (MockSearcher*)Class_Make_Obj(MOCKSEARCHER);
    return MockSearcher_init(self);
}

MockSearcher*
MockSearcher_init(MockSearcher *self) {
    Schema *schema = (Schema*)TestSchema_new(false);
    Searcher_init((Searcher*)self, schema);
    DECREF(schema);
    return self;
}

int32_t
MockSearcher_Doc_Max_IMP(MockSearcher *self) {
    UNUSED_VAR(self);
    return 2500;
}

uint32_t
MockSearcher_Doc_Freq_IMP(MockSearcher *self, String *field, Obj *term) {
    UNUSED_VAR(self);
    UNUSED_VAR(field);
    UNUSED_VAR(term);
    return 10;
}

