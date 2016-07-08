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

#define C_TESTLUCY_TESTQUERYPARSER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <string.h>

#include "Clownfish/TestHarness/TestUtils.h"
#include "Lucy/Test/Search/TestQueryParser.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Search/PhraseQuery.h"
#include "Lucy/Search/LeafQuery.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/NOTQuery.h"
#include "Lucy/Search/ORQuery.h"

TestQueryParser*
TestQP_new(const char *query_string, Query *tree, Query *expanded,
           uint32_t num_hits) {
    TestQueryParser *self
        = (TestQueryParser*)Class_Make_Obj(TESTQUERYPARSER);
    return TestQP_init(self, query_string, tree, expanded, num_hits);
}

TestQueryParser*
TestQP_init(TestQueryParser *self, const char *query_string, Query *tree,
            Query *expanded, uint32_t num_hits) {
    TestQueryParserIVARS *const ivars = TestQP_IVARS(self);
    ivars->query_string = query_string ? TestUtils_get_str(query_string) : NULL;
    ivars->tree         = tree     ? tree     : NULL;
    ivars->expanded     = expanded ? expanded : NULL;
    ivars->num_hits     = num_hits;
    return self;
}

void
TestQP_Destroy_IMP(TestQueryParser *self) {
    TestQueryParserIVARS *const ivars = TestQP_IVARS(self);
    DECREF(ivars->query_string);
    DECREF(ivars->tree);
    DECREF(ivars->expanded);
    SUPER_DESTROY(self, TESTQUERYPARSER);
}

String*
TestQP_Get_Query_String_IMP(TestQueryParser *self) {
    return TestQP_IVARS(self)->query_string;
}

Query*
TestQP_Get_Tree_IMP(TestQueryParser *self) {
    return TestQP_IVARS(self)->tree;
}

Query*
TestQP_Get_Expanded_IMP(TestQueryParser *self) {
    return TestQP_IVARS(self)->expanded;
}

uint32_t
TestQP_Get_Num_Hits_IMP(TestQueryParser *self) {
    return TestQP_IVARS(self)->num_hits;
}



