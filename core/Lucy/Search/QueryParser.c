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

#define C_LUCY_QUERYPARSER
#include <stdlib.h>
#include <ctype.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/QueryParser.h"

#include "Clownfish/CharBuf.h"
#include "Lucy/Search/QueryParser/ParserElem.h"
#include "Lucy/Search/QueryParser/QueryLexer.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/LeafQuery.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/MatchAllQuery.h"
#include "Lucy/Search/NoMatchQuery.h"
#include "Lucy/Search/NOTQuery.h"
#include "Lucy/Search/ORQuery.h"
#include "Lucy/Search/PhraseQuery.h"
#include "Lucy/Search/RequiredOptionalQuery.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Search/Query.h"

#define SHOULD            LUCY_QPARSER_SHOULD
#define MUST              LUCY_QPARSER_MUST
#define MUST_NOT          LUCY_QPARSER_MUST_NOT
#define TOKEN_OPEN_PAREN  LUCY_QPARSER_TOKEN_OPEN_PAREN
#define TOKEN_CLOSE_PAREN LUCY_QPARSER_TOKEN_CLOSE_PAREN
#define TOKEN_MINUS       LUCY_QPARSER_TOKEN_MINUS
#define TOKEN_PLUS        LUCY_QPARSER_TOKEN_PLUS
#define TOKEN_NOT         LUCY_QPARSER_TOKEN_NOT
#define TOKEN_OR          LUCY_QPARSER_TOKEN_OR
#define TOKEN_AND         LUCY_QPARSER_TOKEN_AND
#define TOKEN_FIELD       LUCY_QPARSER_TOKEN_FIELD
#define TOKEN_STRING      LUCY_QPARSER_TOKEN_STRING
#define TOKEN_QUERY       LUCY_QPARSER_TOKEN_QUERY

// Helper function for Tree().
static Query*
S_parse_subquery(QueryParser *self, VArray *elems, String *default_field,
                 bool enclosed);

// Drop unmatched right parens and add matching right parens at end to
// close paren groups implicitly.
static void
S_balance_parens(QueryParser *self, VArray *elems);

// Work from the inside out, reducing the leftmost, innermost paren groups
// first, until the array of elems contains no parens.
static void
S_parse_subqueries(QueryParser *self, VArray *elems);

static void
S_compose_inner_queries(QueryParser *self, VArray *elems,
                        String *default_field);

// Apply +, -, NOT.
static void
S_apply_plusses_and_negations(QueryParser *self, VArray *elems);

// Wrap negated queries with NOTQuery objects.
static void
S_compose_not_queries(QueryParser *self, VArray *elems);

// Silently discard non-sensical combos of AND and OR, e.g.
// 'OR a AND AND OR b AND'.
static void
S_winnow_boolops(QueryParser *self, VArray *elems);

// Join ANDQueries.
static void
S_compose_and_queries(QueryParser *self, VArray *elems);

// Join ORQueries.
static void
S_compose_or_queries(QueryParser *self, VArray *elems);

// Derive a single subquery from all Query objects in the clause.
static Query*
S_compose_subquery(QueryParser *self, VArray *elems, bool enclosed);

QueryParser*
QParser_new(Schema *schema, Analyzer *analyzer, String *default_boolop,
            VArray *fields) {
    QueryParser *self = (QueryParser*)Class_Make_Obj(QUERYPARSER);
    return QParser_init(self, schema, analyzer, default_boolop, fields);
}

QueryParser*
QParser_init(QueryParser *self, Schema *schema, Analyzer *analyzer,
             String *default_boolop, VArray *fields) {
    QueryParserIVARS *const ivars = QParser_IVARS(self);
    // Init.
    ivars->heed_colons = false;
    ivars->lexer       = QueryLexer_new();

    // Assign.
    ivars->schema         = (Schema*)INCREF(schema);
    ivars->analyzer       = (Analyzer*)INCREF(analyzer);
    ivars->default_boolop = default_boolop
                           ? Str_Clone(default_boolop)
                           : Str_new_from_trusted_utf8("OR", 2);

    if (fields) {
        ivars->fields = VA_Shallow_Copy(fields);
        for (uint32_t i = 0, max = VA_Get_Size(fields); i < max; i++) {
            CERTIFY(VA_Fetch(fields, i), STRING);
        }
        VA_Sort(ivars->fields, NULL, NULL);
    }
    else {
        VArray *all_fields = Schema_All_Fields(schema);
        uint32_t num_fields = VA_Get_Size(all_fields);
        ivars->fields = VA_new(num_fields);
        for (uint32_t i = 0; i < num_fields; i++) {
            String *field = (String*)VA_Fetch(all_fields, i);
            FieldType *type = Schema_Fetch_Type(schema, field);
            if (type && FType_Indexed(type)) {
                VA_Push(ivars->fields, INCREF(field));
            }
        }
        DECREF(all_fields);
    }
    VA_Sort(ivars->fields, NULL, NULL);

    // Derive default "occur" from default boolean operator.
    if (Str_Equals_Utf8(ivars->default_boolop, "OR", 2)) {
        ivars->default_occur = SHOULD;
    }
    else if (Str_Equals_Utf8(ivars->default_boolop, "AND", 3)) {
        ivars->default_occur = MUST;
    }
    else {
        THROW(ERR, "Invalid value for default_boolop: %o", ivars->default_boolop);
    }

    return self;
}

void
QParser_Destroy_IMP(QueryParser *self) {
    QueryParserIVARS *const ivars = QParser_IVARS(self);
    DECREF(ivars->schema);
    DECREF(ivars->analyzer);
    DECREF(ivars->default_boolop);
    DECREF(ivars->fields);
    DECREF(ivars->lexer);
    SUPER_DESTROY(self, QUERYPARSER);
}

Analyzer*
QParser_Get_Analyzer_IMP(QueryParser *self) {
    return QParser_IVARS(self)->analyzer;
}

Schema*
QParser_Get_Schema_IMP(QueryParser *self) {
    return QParser_IVARS(self)->schema;
}

String*
QParser_Get_Default_BoolOp_IMP(QueryParser *self) {
    return QParser_IVARS(self)->default_boolop;
}

VArray*
QParser_Get_Fields_IMP(QueryParser *self) {
    return QParser_IVARS(self)->fields;
}

bool
QParser_Heed_Colons_IMP(QueryParser *self) {
    return QParser_IVARS(self)->heed_colons;
}

void
QParser_Set_Heed_Colons_IMP(QueryParser *self, bool heed_colons) {
    QueryParserIVARS *const ivars = QParser_IVARS(self);
    ivars->heed_colons = heed_colons;
    QueryLexer_Set_Heed_Colons(ivars->lexer, heed_colons);
}


Query*
QParser_Parse_IMP(QueryParser *self, String *query_string) {
    String *qstring = query_string
                      ? Str_Clone(query_string)
                      : Str_new_from_trusted_utf8("", 0);
    Query *tree     = QParser_Tree(self, qstring);
    Query *expanded = QParser_Expand(self, tree);
    Query *pruned   = QParser_Prune(self, expanded);
    DECREF(expanded);
    DECREF(tree);
    DECREF(qstring);
    return pruned;
}

Query*
QParser_Tree_IMP(QueryParser *self, String *query_string) {
    QueryParserIVARS *const ivars = QParser_IVARS(self);
    VArray *elems = QueryLexer_Tokenize(ivars->lexer, query_string);
    S_balance_parens(self, elems);
    S_parse_subqueries(self, elems);
    Query *query = S_parse_subquery(self, elems, NULL, false);
    DECREF(elems);
    return query;
}

static void
S_parse_subqueries(QueryParser *self, VArray *elems) {
    const int32_t default_occur = QParser_IVARS(self)->default_occur;
    while (1) {
        // Work from the inside out, starting with the leftmost innermost
        // paren group.
        size_t left = SIZE_MAX;
        size_t right = SIZE_MAX;
        String *field = NULL;
        for (size_t i = 0, max = VA_Get_Size(elems); i < max; i++) {
            ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);
            uint32_t type = ParserElem_Get_Type(elem);
            if (type == TOKEN_OPEN_PAREN) {
                left = i;
            }
            else if (type == TOKEN_CLOSE_PAREN) {
                right = i;
                break;
            }
            else if (type == TOKEN_FIELD && i < max - 1) {
                // If a field applies to an enclosing paren, pass it along.
                ParserElem *next_elem = (ParserElem*)VA_Fetch(elems, i + 1);
                uint32_t next_type = ParserElem_Get_Type(next_elem);
                if (next_type == TOKEN_OPEN_PAREN) {
                    field = (String*)ParserElem_As(elem, STRING);
                }
            }
        }

        // Break out of loop when there are no parens left.
        if (right == SIZE_MAX) {
            break;
        }

        // Create the subquery.
        VArray *sub_elems = VA_Slice(elems, left + 1, right - left - 1);
        Query *subquery = S_parse_subquery(self, sub_elems, field, true);
        ParserElem *new_elem = ParserElem_new(TOKEN_QUERY, (Obj*)subquery);
        if (default_occur == MUST) {
            ParserElem_Require(new_elem);
        }
        DECREF(sub_elems);

        // Replace the elements used to create the subquery with the subquery
        // itself.
        if (left > 0) {
            ParserElem *maybe_field = (ParserElem*)VA_Fetch(elems, left - 1);
            uint32_t maybe_field_type = ParserElem_Get_Type(maybe_field);
            if (maybe_field_type == TOKEN_FIELD) {
                left -= 1;
            }
        }
        VA_Excise(elems, left + 1, right - left);
        VA_Store(elems, left, (Obj*)new_elem);
    }
}

static void
S_discard_elems(VArray *elems, uint32_t type) {
    for (size_t i = VA_Get_Size(elems); i--;) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);
        if (ParserElem_Get_Type(elem) == type) { VA_Excise(elems, i, 1); }
    }
}

static Query*
S_parse_subquery(QueryParser *self, VArray *elems, String *default_field,
                 bool enclosed) {
    if (VA_Get_Size(elems)) {
        ParserElem *first = (ParserElem*)VA_Fetch(elems, 0);
        if (ParserElem_Get_Type(first) == TOKEN_OPEN_PAREN) {
            enclosed = true;
            DECREF(VA_Shift(elems));
            DECREF(VA_Pop(elems));
        }
    }
    S_compose_inner_queries(self, elems, default_field);
    S_discard_elems(elems, TOKEN_FIELD);
    S_discard_elems(elems, TOKEN_STRING);
    S_apply_plusses_and_negations(self, elems);
    S_discard_elems(elems, TOKEN_PLUS);
    S_discard_elems(elems, TOKEN_MINUS);
    S_discard_elems(elems, TOKEN_NOT);
    S_compose_not_queries(self, elems);
    S_winnow_boolops(self, elems);
    if (VA_Get_Size(elems) > 2) {
        S_compose_and_queries(self, elems);
        // Don't double wrap '(a AND b)'.
        if (VA_Get_Size(elems) == 1) { enclosed = false; }
    }
    S_discard_elems(elems, TOKEN_AND);
    if (VA_Get_Size(elems) > 2) {
        S_compose_or_queries(self, elems);
        // Don't double wrap '(a OR b)'.
        if (VA_Get_Size(elems) == 1) { enclosed = false; }
    }
    S_discard_elems(elems, TOKEN_OR);
    Query *retval = S_compose_subquery(self, elems, enclosed);

    return retval;
}

static void
S_balance_parens(QueryParser *self, VArray *elems) {
    UNUSED_VAR(self);
    // Count paren balance, eliminate unbalanced right parens.
    int64_t paren_depth = 0;
    size_t i = 0;
    while (i < VA_Get_Size(elems)) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);
        if (ParserElem_Get_Type(elem) == TOKEN_OPEN_PAREN) {
            paren_depth++;
        }
        else if (ParserElem_Get_Type(elem) == TOKEN_CLOSE_PAREN) {
            if (paren_depth > 0) {
                paren_depth--;
            }
            else {
                VA_Excise(elems, i, 1);
                continue;
            }
        }
        i++;
    }

    // Insert implicit parens.
    while (paren_depth--) {
        ParserElem *elem = ParserElem_new(TOKEN_CLOSE_PAREN, NULL);
        VA_Push(elems, (Obj*)elem);
    }
}

static void
S_compose_inner_queries(QueryParser *self, VArray *elems,
                        String *default_field) {
    const int32_t default_occur = QParser_IVARS(self)->default_occur;

    // Generate all queries.  Apply any fields.
    for (uint32_t i = VA_Get_Size(elems); i--;) {
        String *field = default_field;
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);

        // Apply field.
        if (i > 0) {
            // Field specifier must immediately precede any query.
            ParserElem* maybe_field_elem
                = (ParserElem*)VA_Fetch(elems, i - 1);
            if (ParserElem_Get_Type(maybe_field_elem) == TOKEN_FIELD) {
                field = (String*)ParserElem_As(maybe_field_elem, STRING);
            }
        }

        if (ParserElem_Get_Type(elem) == TOKEN_STRING) {
            String *text = (String*)ParserElem_As(elem, STRING);
            LeafQuery *query = LeafQuery_new(field, text);
            ParserElem *new_elem
                = ParserElem_new(TOKEN_QUERY, (Obj*)query);
            if (default_occur == MUST) {
                ParserElem_Require(new_elem);
            }
            VA_Store(elems, i, (Obj*)new_elem);
        }
    }
}

static void
S_apply_plusses_and_negations(QueryParser *self, VArray *elems) {
    UNUSED_VAR(self);
    for (uint32_t i = VA_Get_Size(elems); i--;) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);
        if (ParserElem_Get_Type(elem) == TOKEN_QUERY) {
            for (uint32_t j = i; j--;) {
                ParserElem *prev = (ParserElem*)VA_Fetch(elems, j);
                uint32_t prev_type = ParserElem_Get_Type(prev);
                if (prev_type == TOKEN_MINUS || prev_type == TOKEN_NOT) {
                    ParserElem_Negate(elem);
                }
                else if (prev_type == TOKEN_PLUS) {
                    ParserElem_Require(elem);
                }
                else {
                    break;
                }
            }
        }
    }
}

static void
S_compose_not_queries(QueryParser *self, VArray *elems) {
    for (uint32_t i = 0, max = VA_Get_Size(elems); i < max; i++) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);
        if (ParserElem_Get_Type(elem) == TOKEN_QUERY
            && ParserElem_Negated(elem)
           ) {
            Query *inner_query = (Query*)ParserElem_As(elem, QUERY);
            Query *not_query = QParser_Make_NOT_Query(self, inner_query);
            ParserElem_Set_Value(elem, (Obj*)not_query);
            DECREF(not_query);
        }
    }
}

static void
S_winnow_boolops(QueryParser *self, VArray *elems) {
    UNUSED_VAR(self);
    for (uint32_t i = 0; i < VA_Get_Size(elems); i++) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);
        if (ParserElem_Get_Type(elem) != TOKEN_QUERY) {
            uint32_t num_to_zap = 0;
            ParserElem *preceding = (ParserElem*)VA_Fetch(elems, i - 1);
            ParserElem *following = (ParserElem*)VA_Fetch(elems, i + 1);
            if (!preceding || ParserElem_Get_Type(preceding) != TOKEN_QUERY) {
                num_to_zap = 1;
            }
            if (!following || ParserElem_Get_Type(following) != TOKEN_QUERY) {
                num_to_zap = 1;
            }
            for (uint32_t j = i + 1, jmax = VA_Get_Size(elems); j < jmax; j++) {
                ParserElem *maybe = (ParserElem*)VA_Fetch(elems, j);
                if (ParserElem_Get_Type(maybe) == TOKEN_QUERY) { break; }
                else { num_to_zap++; }
            }
            if (num_to_zap) { VA_Excise(elems, i, num_to_zap); }
        }
    }
}

// Apply AND.
static void
S_compose_and_queries(QueryParser *self, VArray *elems) {
    const int32_t default_occur = QParser_IVARS(self)->default_occur;

    for (uint32_t i = 0; i + 2 < VA_Get_Size(elems); i++) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i + 1);
        if (ParserElem_Get_Type(elem) == TOKEN_AND) {
            ParserElem   *preceding  = (ParserElem*)VA_Fetch(elems, i);
            VArray       *children   = VA_new(2);
            uint32_t      num_to_zap = 0;

            // Add first clause.
            Query *preceding_query = (Query*)ParserElem_As(preceding, QUERY);
            VA_Push(children, INCREF(preceding_query));

            // Add following clauses.
            for (uint32_t j = i + 1, jmax = VA_Get_Size(elems);
                 j < jmax;
                 j += 2, num_to_zap += 2
                ) {
                ParserElem *maybe_and = (ParserElem*)VA_Fetch(elems, j);
                ParserElem *following = (ParserElem*)VA_Fetch(elems, j + 1);
                if (ParserElem_Get_Type(maybe_and) != TOKEN_AND) {
                    break;
                }
                else if (ParserElem_Get_Type(following) == TOKEN_QUERY) {
                    Query *next = (Query*)ParserElem_As(following, QUERY);
                    VA_Push(children, INCREF(next));
                }
                else {
                    THROW(ERR, "Unexpected type: %u32",
                          ParserElem_Get_Type(following));
                }
            }
            Query *and_query = QParser_Make_AND_Query(self, children);
            ParserElem_Set_Value(preceding, (Obj*)and_query);
            if (default_occur == MUST) {
                ParserElem_Require(preceding);
            }
            DECREF(and_query);
            DECREF(children);

            VA_Excise(elems, i + 1, num_to_zap);
        }
    }
}

static void
S_compose_or_queries(QueryParser *self, VArray *elems) {
    const int32_t default_occur = QParser_IVARS(self)->default_occur;

    for (uint32_t i = 0; i + 2 < VA_Get_Size(elems); i++) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, i + 1);
        if (ParserElem_Get_Type(elem) == TOKEN_OR) {
            ParserElem   *preceding  = (ParserElem*)VA_Fetch(elems, i);
            VArray       *children   = VA_new(2);
            uint32_t      num_to_zap = 0;

            // Add first clause.
            Query *preceding_query = (Query*)ParserElem_As(preceding, QUERY);
            VA_Push(children, INCREF(preceding_query));

            // Add following clauses.
            for (uint32_t j = i + 1, jmax = VA_Get_Size(elems);
                 j < jmax;
                 j += 2, num_to_zap += 2
                ) {
                ParserElem *maybe_or  = (ParserElem*)VA_Fetch(elems, j);
                ParserElem *following = (ParserElem*)VA_Fetch(elems, j + 1);
                if (ParserElem_Get_Type(maybe_or) != TOKEN_OR) {
                    break;
                }
                else if (ParserElem_Get_Type(following) == TOKEN_QUERY) {
                    Query *next = (Query*)ParserElem_As(following, QUERY);
                    VA_Push(children, INCREF(next));
                }
                else {
                    THROW(ERR, "Unexpected type: %u32",
                          ParserElem_Get_Type(following));
                }
            }
            Query *or_query = QParser_Make_OR_Query(self, children);
            ParserElem_Set_Value(preceding, (Obj*)or_query);
            if (default_occur == MUST) {
                ParserElem_Require(preceding);
            }
            DECREF(or_query);
            DECREF(children);

            VA_Excise(elems, i + 1, num_to_zap);
        }
    }
}

static Query*
S_compose_subquery(QueryParser *self, VArray *elems, bool enclosed) {
    const int32_t default_occur = QParser_IVARS(self)->default_occur;
    Query *retval;

    if (VA_Get_Size(elems) == 0) {
        // No elems means no query. Maybe the search string was something
        // like 'NOT AND'
        if (enclosed) {
            retval = default_occur == SHOULD
                     ? QParser_Make_OR_Query(self, NULL)
                     : QParser_Make_AND_Query(self, NULL);
        }
        else {
            retval = (Query*)NoMatchQuery_new();
        }
    }
    else if (VA_Get_Size(elems) == 1 && !enclosed) {
        ParserElem *elem = (ParserElem*)VA_Fetch(elems, 0);
        Query *query = (Query*)ParserElem_As(elem, QUERY);
        retval = (Query*)INCREF(query);
    }
    else {
        uint32_t  num_elems = VA_Get_Size(elems);
        VArray   *required  = VA_new(num_elems);
        VArray   *optional  = VA_new(num_elems);
        VArray   *negated   = VA_new(num_elems);
        Query    *req_query = NULL;
        Query    *opt_query = NULL;

        // Demux elems into bins.
        for (uint32_t i = 0; i < num_elems; i++) {
            ParserElem *elem = (ParserElem*)VA_Fetch(elems, i);
            if (ParserElem_Required(elem)) {
                VA_Push(required, INCREF(ParserElem_As(elem, QUERY)));
            }
            else if (ParserElem_Optional(elem)) {
                VA_Push(optional, INCREF(ParserElem_As(elem, QUERY)));
            }
            else if (ParserElem_Negated(elem)) {
                VA_Push(negated, INCREF(ParserElem_As(elem, QUERY)));
            }
        }
        uint32_t num_required = VA_Get_Size(required);
        uint32_t num_negated  = VA_Get_Size(negated);
        uint32_t num_optional = VA_Get_Size(optional);

        // Bind all mandatory matchers together in one Query.
        if (num_required || num_negated) {
            if (enclosed || num_required + num_negated > 1) {
                VArray *children = VA_Shallow_Copy(required);
                VA_Push_VArray(children, negated);
                req_query = QParser_Make_AND_Query(self, children);
                DECREF(children);
            }
            else if (num_required) {
                req_query = (Query*)INCREF(VA_Fetch(required, 0));
            }
            else if (num_negated) {
                req_query = (Query*)INCREF(VA_Fetch(negated, 0));
            }
        }

        // Bind all optional matchers together in one Query.
        if (num_optional) {
            if (!enclosed && num_optional == 1) {
                opt_query = (Query*)INCREF(VA_Fetch(optional, 0));
            }
            else {
                opt_query = QParser_Make_OR_Query(self, optional);
            }
        }

        // Unify required and optional.
        if (req_query && opt_query) {
            if (num_required) { // not just negated elems
                retval = QParser_Make_Req_Opt_Query(self, req_query,
                                                    opt_query);
            }
            else {
                // req_query has only negated queries.
                VArray *children = VA_new(2);
                VA_Push(children, INCREF(req_query));
                VA_Push(children, INCREF(opt_query));
                retval = QParser_Make_AND_Query(self, children);
                DECREF(children);
            }
        }
        else if (opt_query) {
            // Only optional elems.
            retval = (Query*)INCREF(opt_query);
        }
        else if (req_query) {
            // Only required elems.
            retval = (Query*)INCREF(req_query);
        }
        else {
            retval = NULL; // kill "uninitialized" compiler warning
            THROW(ERR, "Unexpected error");
        }

        DECREF(opt_query);
        DECREF(req_query);
        DECREF(negated);
        DECREF(optional);
        DECREF(required);
    }

    return retval;
}

static bool
S_has_valid_clauses(Query *query) {
    if (Query_Is_A(query, NOTQUERY)) {
        return false;
    }
    else if (Query_Is_A(query, MATCHALLQUERY)) {
        return false;
    }
    else if (Query_Is_A(query, ORQUERY) || Query_Is_A(query, ANDQUERY)) {
        PolyQuery *polyquery = (PolyQuery*)query;
        VArray    *children  = PolyQuery_Get_Children(polyquery);
        for (uint32_t i = 0, max = VA_Get_Size(children); i < max; i++) {
            Query *child = (Query*)VA_Fetch(children, i);
            if (S_has_valid_clauses(child)) {
                return true;
            }
        }
        return false;
    }
    return true;
}

static void
S_do_prune(QueryParser *self, Query *query) {
    if (Query_Is_A(query, NOTQUERY)) {
        // Don't allow double negatives.
        NOTQuery *not_query = (NOTQuery*)query;
        Query *neg_query = NOTQuery_Get_Negated_Query(not_query);
        if (!Query_Is_A(neg_query, MATCHALLQUERY)
            && !S_has_valid_clauses(neg_query)
           ) {
            MatchAllQuery *matchall = MatchAllQuery_new();
            NOTQuery_Set_Negated_Query(not_query, (Query*)matchall);
            DECREF(matchall);
        }
    }
    else if (Query_Is_A(query, POLYQUERY)) {
        PolyQuery *polyquery = (PolyQuery*)query;
        VArray    *children  = PolyQuery_Get_Children(polyquery);

        // Recurse.
        for (uint32_t i = 0, max = VA_Get_Size(children); i < max; i++) {
            Query *child = (Query*)VA_Fetch(children, i);
            S_do_prune(self, child);
        }

        if (PolyQuery_Is_A(polyquery, REQUIREDOPTIONALQUERY)
            || PolyQuery_Is_A(polyquery, ORQUERY)
           ) {
            // Don't allow 'foo OR (-bar)'.
            VArray *children = PolyQuery_Get_Children(polyquery);
            for (uint32_t i = 0, max = VA_Get_Size(children); i < max; i++) {
                Query *child = (Query*)VA_Fetch(children, i);
                if (!S_has_valid_clauses(child)) {
                    VA_Store(children, i, (Obj*)NoMatchQuery_new());
                }
            }
        }
        else if (PolyQuery_Is_A(polyquery, ANDQUERY)) {
            // Don't allow '(-bar AND -baz)'.
            if (!S_has_valid_clauses((Query*)polyquery)) {
                VArray *children = PolyQuery_Get_Children(polyquery);
                VA_Clear(children);
            }
        }
    }
}

Query*
QParser_Prune_IMP(QueryParser *self, Query *query) {
    if (!query
        || Query_Is_A(query, NOTQUERY)
        || Query_Is_A(query, MATCHALLQUERY)
       ) {
        return (Query*)NoMatchQuery_new();
    }
    else if (Query_Is_A(query, POLYQUERY)) {
        S_do_prune(self, query);
    }
    return (Query*)INCREF(query);
}

Query*
QParser_Expand_IMP(QueryParser *self, Query *query) {
    Query *retval = NULL;

    if (Query_Is_A(query, LEAFQUERY)) {
        retval = QParser_Expand_Leaf(self, query);
    }
    else if (Query_Is_A(query, ORQUERY) || Query_Is_A(query, ANDQUERY)) {
        PolyQuery *polyquery = (PolyQuery*)query;
        VArray *children = PolyQuery_Get_Children(polyquery);
        VArray *new_kids = VA_new(VA_Get_Size(children));

        for (uint32_t i = 0, max = VA_Get_Size(children); i < max; i++) {
            Query *child = (Query*)VA_Fetch(children, i);
            Query *new_child = QParser_Expand(self, child); // recurse
            if (new_child) {
                if (Query_Is_A(new_child, NOMATCHQUERY)) {
                    bool fails = NoMatchQuery_Get_Fails_To_Match(
                                       (NoMatchQuery*)new_child);
                    if (fails) {
                        VA_Push(new_kids, (Obj*)new_child);
                    }
                    else {
                        DECREF(new_child);
                    }
                }
                else {
                    VA_Push(new_kids, (Obj*)new_child);
                }
            }
        }

        if (VA_Get_Size(new_kids) == 0) {
            retval = (Query*)NoMatchQuery_new();
        }
        else if (VA_Get_Size(new_kids) == 1) {
            retval = (Query*)INCREF(VA_Fetch(new_kids, 0));
        }
        else {
            PolyQuery_Set_Children(polyquery, new_kids);
            retval = (Query*)INCREF(query);
        }

        DECREF(new_kids);
    }
    else if (Query_Is_A(query, NOTQUERY)) {
        NOTQuery *not_query     = (NOTQuery*)query;
        Query    *negated_query = NOTQuery_Get_Negated_Query(not_query);
        negated_query = QParser_Expand(self, negated_query);
        if (negated_query) {
            NOTQuery_Set_Negated_Query(not_query, negated_query);
            DECREF(negated_query);
            retval = (Query*)INCREF(query);
        }
        else {
            retval = (Query*)MatchAllQuery_new();
        }
    }
    else if (Query_Is_A(query, REQUIREDOPTIONALQUERY)) {
        RequiredOptionalQuery *req_opt_query = (RequiredOptionalQuery*)query;
        Query *req_query = ReqOptQuery_Get_Required_Query(req_opt_query);
        Query *opt_query = ReqOptQuery_Get_Optional_Query(req_opt_query);

        req_query = QParser_Expand(self, req_query);
        opt_query = QParser_Expand(self, opt_query);

        if (req_query && opt_query) {
            ReqOptQuery_Set_Required_Query(req_opt_query, req_query);
            ReqOptQuery_Set_Optional_Query(req_opt_query, opt_query);
            retval = (Query*)INCREF(query);
        }
        else if (req_query) { retval = (Query*)INCREF(req_query); }
        else if (opt_query) { retval = (Query*)INCREF(opt_query); }
        else { retval = (Query*)NoMatchQuery_new(); }

        DECREF(opt_query);
        DECREF(req_query);
    }
    else {
        retval = (Query*)INCREF(query);
    }

    return retval;
}

static String*
S_unescape(QueryParser *self, String *orig, CharBuf *buf) {
    StringIterator *iter = Str_Top(orig);
    int32_t code_point;
    UNUSED_VAR(self);

    CB_Set_Size(buf, 0);
    CB_Grow(buf, Str_Get_Size(orig) + 4);

    while (STRITER_DONE != (code_point = StrIter_Next(iter))) {
        if (code_point == '\\') {
            int32_t next_code_point = StrIter_Next(iter);
            if (next_code_point == ':'
                || next_code_point == '"'
                || next_code_point == '\\'
               ) {
                CB_Cat_Char(buf, next_code_point);
            }
            else {
                CB_Cat_Char(buf, code_point);
                if (next_code_point != STRITER_DONE) {
                    CB_Cat_Char(buf, next_code_point);
                }
            }
        }
        else {
            CB_Cat_Char(buf, code_point);
        }
    }

    DECREF(iter);
    return CB_To_String(buf);
}

Query*
QParser_Expand_Leaf_IMP(QueryParser *self, Query *query) {
    QueryParserIVARS *const ivars = QParser_IVARS(self);
    LeafQuery *leaf_query = (LeafQuery*)query;
    Schema    *schema     = ivars->schema;
    bool       is_phrase  = false;
    bool       ambiguous  = false;

    // Determine whether we can actually process the input.
    if (!Query_Is_A(query, LEAFQUERY)) { return NULL; }
    String *full_text = LeafQuery_Get_Text(leaf_query);
    if (!Str_Get_Size(full_text)) { return NULL; }

    // If quoted, always generate PhraseQuery.
    StringIterator *top  = Str_Top(full_text);
    StringIterator *tail = Str_Tail(full_text);
    StrIter_Skip_Next_Whitespace(top);
    StrIter_Skip_Prev_Whitespace(tail);
    if (StrIter_Starts_With_Utf8(top, "\"", 1)) {
        is_phrase = true;
        StrIter_Advance(top, 1);
        if (StrIter_Ends_With_Utf8(tail, "\"", 1)
            && !StrIter_Ends_With_Utf8(tail, "\\\"", 2)
        ) {
            StrIter_Recede(tail, 1);
        }
    }
    String *source_text = StrIter_substring(top, tail);

    // Either use LeafQuery's field or default to Parser's list.
    VArray *fields;
    if (LeafQuery_Get_Field(leaf_query)) {
        fields = VA_new(1);
        VA_Push(fields, INCREF(LeafQuery_Get_Field(leaf_query)));
    }
    else {
        fields = (VArray*)INCREF(ivars->fields);
    }

    CharBuf *unescape_buf = CB_new(Str_Get_Size(source_text));
    VArray  *queries      = VA_new(VA_Get_Size(fields));
    for (uint32_t i = 0, max = VA_Get_Size(fields); i < max; i++) {
        String   *field    = (String*)VA_Fetch(fields, i);
        Analyzer *analyzer = ivars->analyzer
                             ? ivars->analyzer
                             : Schema_Fetch_Analyzer(schema, field);

        if (!analyzer) {
            VA_Push(queries,
                    (Obj*)QParser_Make_Term_Query(self, field,
                                                  (Obj*)source_text));
        }
        else {
            // Extract token texts.
            String *split_source = S_unescape(self, source_text, unescape_buf);
            VArray *maybe_texts = Analyzer_Split(analyzer, split_source);
            uint32_t num_maybe_texts = VA_Get_Size(maybe_texts);
            VArray *token_texts = VA_new(num_maybe_texts);

            // Filter out zero-length token texts.
            for (uint32_t j = 0; j < num_maybe_texts; j++) {
                String *token_text = (String*)VA_Fetch(maybe_texts, j);
                if (Str_Get_Size(token_text)) {
                    VA_Push(token_texts, INCREF(token_text));
                }
            }

            if (VA_Get_Size(token_texts) == 0) {
                /* Query might include stop words.  Who knows? */
                ambiguous = true;
            }

            // Add either a TermQuery or a PhraseQuery.
            if (is_phrase || VA_Get_Size(token_texts) > 1) {
                VA_Push(queries, (Obj*)
                        QParser_Make_Phrase_Query(self, field, token_texts));
            }
            else if (VA_Get_Size(token_texts) == 1) {
                VA_Push(queries,
                        (Obj*)QParser_Make_Term_Query(self, field, VA_Fetch(token_texts, 0)));
            }

            DECREF(token_texts);
            DECREF(maybe_texts);
            DECREF(split_source);
        }
    }

    Query *retval;
    if (VA_Get_Size(queries) == 0) {
        retval = (Query*)NoMatchQuery_new();
        if (ambiguous) {
            NoMatchQuery_Set_Fails_To_Match((NoMatchQuery*)retval, false);
        }
    }
    else if (VA_Get_Size(queries) == 1) {
        retval = (Query*)INCREF(VA_Fetch(queries, 0));
    }
    else {
        retval = QParser_Make_OR_Query(self, queries);
    }

    // Clean up.
    DECREF(unescape_buf);
    DECREF(queries);
    DECREF(fields);
    DECREF(source_text);
    DECREF(tail);
    DECREF(top);

    return retval;
}

Query*
QParser_Make_Term_Query_IMP(QueryParser *self, String *field,
                            Obj *term) {
    UNUSED_VAR(self);
    return (Query*)TermQuery_new(field, term);
}

Query*
QParser_Make_Phrase_Query_IMP(QueryParser *self, String *field,
                              VArray *terms) {
    UNUSED_VAR(self);
    return (Query*)PhraseQuery_new(field, terms);
}

Query*
QParser_Make_OR_Query_IMP(QueryParser *self, VArray *children) {
    UNUSED_VAR(self);
    return (Query*)ORQuery_new(children);
}

Query*
QParser_Make_AND_Query_IMP(QueryParser *self, VArray *children) {
    UNUSED_VAR(self);
    return (Query*)ANDQuery_new(children);
}

Query*
QParser_Make_NOT_Query_IMP(QueryParser *self, Query *negated_query) {
    UNUSED_VAR(self);
    return (Query*)NOTQuery_new(negated_query);
}

Query*
QParser_Make_Req_Opt_Query_IMP(QueryParser *self, Query *required_query,
                               Query *optional_query) {
    UNUSED_VAR(self);
    return (Query*)ReqOptQuery_new(required_query, optional_query);
}

