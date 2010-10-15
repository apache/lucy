#define C_KINO_TESTQUERYPARSER
#include "KinoSearch/Util/ToolSet.h"
#include <stdarg.h>
#include <string.h>

#include "KinoSearch/Test/TestQueryParser.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Search/TermQuery.h"
#include "KinoSearch/Search/PhraseQuery.h"
#include "KinoSearch/Search/LeafQuery.h"
#include "KinoSearch/Search/ANDQuery.h"
#include "KinoSearch/Search/NOTQuery.h"
#include "KinoSearch/Search/ORQuery.h"

TestQueryParser*
TestQP_new(const char *query_string, Query *tree, Query *expanded, 
           uint32_t num_hits)
{
    TestQueryParser *self 
        = (TestQueryParser*)VTable_Make_Obj(TESTQUERYPARSER);
    return TestQP_init(self, query_string, tree, expanded, num_hits);
}

TestQueryParser*
TestQP_init(TestQueryParser *self, const char *query_string, Query *tree, 
            Query *expanded, uint32_t num_hits)
{
    self->query_string = query_string ? TestUtils_get_cb(query_string) : NULL;
    self->tree         = tree     ? tree     : NULL;
    self->expanded     = expanded ? expanded : NULL;
    self->num_hits     = num_hits;
    return self;
}

void
TestQP_destroy(TestQueryParser *self)
{
    DECREF(self->query_string);
    DECREF(self->tree);
    DECREF(self->expanded);
    SUPER_DESTROY(self, TESTQUERYPARSER);
}

CharBuf*
TestQP_get_query_string(TestQueryParser *self) { return self->query_string; }
Query*
TestQP_get_tree(TestQueryParser *self)         { return self->tree; }
Query*
TestQP_get_expanded(TestQueryParser *self)     { return self->expanded; }
uint32_t
TestQP_get_num_hits(TestQueryParser *self)     { return self->num_hits; }



