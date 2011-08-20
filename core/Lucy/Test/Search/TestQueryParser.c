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

#define C_LUCY_TESTQUERYPARSER
#include "Lucy/Util/ToolSet.h"
#include <string.h>

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
        = (TestQueryParser*)VTable_Make_Obj(TESTQUERYPARSER);
    return TestQP_init(self, query_string, tree, expanded, num_hits);
}

TestQueryParser*
TestQP_init(TestQueryParser *self, const char *query_string, Query *tree,
            Query *expanded, uint32_t num_hits) {
    self->query_string = query_string ? TestUtils_get_cb(query_string) : NULL;
    self->tree         = tree     ? tree     : NULL;
    self->expanded     = expanded ? expanded : NULL;
    self->num_hits     = num_hits;
    return self;
}

void
TestQP_destroy(TestQueryParser *self) {
    DECREF(self->query_string);
    DECREF(self->tree);
    DECREF(self->expanded);
    SUPER_DESTROY(self, TESTQUERYPARSER);
}

CharBuf*
TestQP_get_query_string(TestQueryParser *self) {
    return self->query_string;
}

Query*
TestQP_get_tree(TestQueryParser *self) {
    return self->tree;
}

Query*
TestQP_get_expanded(TestQueryParser *self) {
    return self->expanded;
}

uint32_t
TestQP_get_num_hits(TestQueryParser *self) {
    return self->num_hits;
}



