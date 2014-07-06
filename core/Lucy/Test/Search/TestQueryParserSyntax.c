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

#define C_TESTLUCY_TESTQUERYPARSERSYNTAX
#define C_TESTLUCY_TESTQUERYPARSER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <string.h>

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Search/TestQueryParserSyntax.h"
#include "Lucy/Test/Search/TestQueryParser.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Analysis/PolyAnalyzer.h"
#include "Lucy/Analysis/RegexTokenizer.h"
#include "Lucy/Analysis/SnowballStopFilter.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/QueryParser.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Search/PhraseQuery.h"
#include "Lucy/Search/LeafQuery.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/NOTQuery.h"
#include "Lucy/Search/ORQuery.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/RAMFolder.h"

#define make_term_query   (Query*)TestUtils_make_term_query
#define make_phrase_query (Query*)TestUtils_make_phrase_query
#define make_leaf_query   (Query*)TestUtils_make_leaf_query
#define make_not_query    (Query*)TestUtils_make_not_query
#define make_poly_query   (Query*)TestUtils_make_poly_query

TestQueryParserSyntax*
TestQPSyntax_new() {
    return (TestQueryParserSyntax*)Class_Make_Obj(TESTQUERYPARSERSYNTAX);
}

static Folder*
build_index() {
    // Plain type.
    String         *pattern   = Str_newf("\\S+");
    RegexTokenizer *tokenizer = RegexTokenizer_new(pattern);
    FullTextType   *plain     = FullTextType_new((Analyzer*)tokenizer);

    // Fancy type.

    String         *word_pattern   = Str_newf("\\w+");
    RegexTokenizer *word_tokenizer = RegexTokenizer_new(word_pattern);

    Hash *stop_list = Hash_new(0);
    Hash_Store_Utf8(stop_list, "x", 1, (Obj*)CFISH_TRUE);
    SnowballStopFilter *stop_filter = SnowStop_new(NULL, stop_list);

    VArray *analyzers = VA_new(0);
    VA_Push(analyzers, (Obj*)word_tokenizer);
    VA_Push(analyzers, (Obj*)stop_filter);
    PolyAnalyzer *fancy_analyzer = PolyAnalyzer_new(NULL, analyzers);

    FullTextType *fancy = FullTextType_new((Analyzer*)fancy_analyzer);

    // Schema.
    Schema *schema   = Schema_new();
    String *plain_str = Str_newf("plain");
    String *fancy_str = Str_newf("fancy");
    Schema_Spec_Field(schema, plain_str, (FieldType*)plain);
    Schema_Spec_Field(schema, fancy_str, (FieldType*)fancy);

    // Indexer.
    RAMFolder *folder  = RAMFolder_new(NULL);
    Indexer   *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

    // Index documents.
    VArray *doc_set = TestUtils_doc_set();
    for (uint32_t i = 0; i < VA_Get_Size(doc_set); ++i) {
        String *content_string = (String*)VA_Fetch(doc_set, i);
        Doc *doc = Doc_new(NULL, 0);
        Doc_Store(doc, plain_str, (Obj*)content_string);
        Doc_Store(doc, fancy_str, (Obj*)content_string);
        Indexer_Add_Doc(indexer, doc, 1.0);
        DECREF(doc);
    }
    Indexer_Commit(indexer);

    // Clean up.
    DECREF(doc_set);
    DECREF(indexer);
    DECREF(fancy_str);
    DECREF(plain_str);
    DECREF(schema);
    DECREF(fancy);
    DECREF(fancy_analyzer);
    DECREF(analyzers);
    DECREF(stop_list);
    DECREF(word_pattern);
    DECREF(plain);
    DECREF(tokenizer);
    DECREF(pattern);

    return (Folder*)folder;
}

static TestQueryParser*
leaf_test_simple_term() {
    Query   *tree     = make_leaf_query(NULL, "a");
    Query   *plain_q  = make_term_query("plain", "a");
    Query   *fancy_q  = make_term_query("fancy", "a");
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("a", tree, expanded, 4);
}

static TestQueryParser*
leaf_test_simple_phrase() {
    Query   *tree     = make_leaf_query(NULL, "\"a b\"");
    Query   *plain_q  = make_phrase_query("plain", "a", "b", NULL);
    Query   *fancy_q  = make_phrase_query("fancy", "a", "b", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\"a b\"", tree, expanded, 3);
}

static TestQueryParser*
leaf_test_unclosed_quote() {
    Query   *tree     = make_leaf_query(NULL, "\"a b");
    Query   *plain_q  = make_phrase_query("plain", "a", "b", NULL);
    Query   *fancy_q  = make_phrase_query("fancy", "a", "b", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\"a b", tree, expanded, 3);
}

static TestQueryParser*
leaf_test_escaped_quotes_inside() {
    Query   *tree     = make_leaf_query(NULL, "\"\\\"a b\\\"\"");
    Query   *plain_q  = make_phrase_query("plain", "\"a", "b\"", NULL);
    Query   *fancy_q  = make_phrase_query("fancy", "a", "b", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\"\\\"a b\\\"\"", tree, expanded, 3);
}

static TestQueryParser*
leaf_test_escaped_quotes_outside() {
    Query   *tree = make_leaf_query(NULL, "\\\"a");
    Query   *plain_q  = make_term_query("plain", "\"a");
    Query   *fancy_q  = make_term_query("fancy", "a");
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\\\"a", tree, expanded, 4);
}

static TestQueryParser*
leaf_test_single_term_phrase() {
    Query   *tree     = make_leaf_query(NULL, "\"a\"");
    Query   *plain_q  = make_phrase_query("plain", "a", NULL);
    Query   *fancy_q  = make_phrase_query("fancy", "a", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\"a\"", tree, expanded, 4);
}

static TestQueryParser*
leaf_test_longer_phrase() {
    Query   *tree     = make_leaf_query(NULL, "\"a b c\"");
    Query   *plain_q  = make_phrase_query("plain", "a", "b", "c", NULL);
    Query   *fancy_q  = make_phrase_query("fancy", "a", "b", "c", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\"a b c\"", tree, expanded, 2);
}

static TestQueryParser*
leaf_test_empty_phrase() {
    Query   *tree     = make_leaf_query(NULL, "\"\"");
    Query   *plain_q  = make_phrase_query("plain", NULL);
    Query   *fancy_q  = make_phrase_query("fancy", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\"\"", tree, expanded, 0);
}

static TestQueryParser*
leaf_test_phrase_with_stopwords() {
    Query   *tree     = make_leaf_query(NULL, "\"x a\"");
    Query   *plain_q  = make_phrase_query("plain", "x", "a", NULL);
    Query   *fancy_q  = make_phrase_query("fancy", "a", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("\"x a\"", tree, expanded, 4);
}

static TestQueryParser*
leaf_test_different_tokenization() {
    Query   *tree     = make_leaf_query(NULL, "a.b");
    Query   *plain_q  = make_term_query("plain", "a.b");
    Query   *fancy_q  = make_phrase_query("fancy", "a", "b", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new("a.b", tree, expanded, 3);
}

static TestQueryParser*
leaf_test_http() {
    char address[] = "http://www.foo.com/bar.html";
    Query *tree = make_leaf_query(NULL, address);
    Query *plain_q = make_term_query("plain", address);
    Query *fancy_q = make_phrase_query("fancy", "http", "www", "foo",
                                       "com", "bar", "html", NULL);
    Query   *expanded = make_poly_query(BOOLOP_OR, fancy_q, plain_q, NULL);
    return TestQP_new(address, tree, expanded, 0);
}

static TestQueryParser*
leaf_test_field() {
    Query *tree     = make_leaf_query("plain", "b");
    Query *expanded = make_term_query("plain", "b");
    return TestQP_new("plain:b", tree, expanded, 3);
}

static TestQueryParser*
leaf_test_unrecognized_field() {
    Query *tree     = make_leaf_query("bogusfield", "b");
    Query *expanded = make_term_query("bogusfield", "b");
    return TestQP_new("bogusfield:b", tree, expanded, 0);
}

static TestQueryParser*
leaf_test_unescape_colons() {
    Query *tree     = make_leaf_query("plain", "a\\:b");
    Query *expanded = make_term_query("plain", "a:b");
    return TestQP_new("plain:a\\:b", tree, expanded, 0);
}

static TestQueryParser*
syntax_test_minus_plus() {
    Query *leaf = make_leaf_query(NULL, "a");
    Query *tree = make_not_query(leaf);
    return TestQP_new("-+a", tree, NULL, 0);
}

static TestQueryParser*
syntax_test_plus_minus() {
    // Not a perfect result, but then it's not a good query string.
    Query *leaf = make_leaf_query(NULL, "a");
    Query *tree = make_not_query(leaf);
    return TestQP_new("+-a", tree, NULL, 0);
}

static TestQueryParser*
syntax_test_minus_minus() {
    // Not a perfect result, but then it's not a good query string.
    Query *tree = make_leaf_query(NULL, "a");
    return TestQP_new("--a", tree, NULL, 4);
}

static TestQueryParser*
syntax_test_not_minus() {
    Query *tree = make_leaf_query(NULL, "a");
    return TestQP_new("NOT -a", tree, NULL, 4);
}

static TestQueryParser*
syntax_test_not_plus() {
    // Not a perfect result, but then it's not a good query string.
    Query *leaf = make_leaf_query(NULL, "a");
    Query *tree = make_not_query(leaf);
    return TestQP_new("NOT +a", tree, NULL, 0);
}

static TestQueryParser*
syntax_test_padded_plus() {
    Query *plus = make_leaf_query(NULL, "+");
    Query *a = make_leaf_query(NULL, "a");
    Query *tree = make_poly_query(BOOLOP_OR, plus, a, NULL);
    return TestQP_new("+ a", tree, NULL, 4);
}

static TestQueryParser*
syntax_test_padded_minus() {
    Query *minus = make_leaf_query(NULL, "-");
    Query *a = make_leaf_query(NULL, "a");
    Query *tree = make_poly_query(BOOLOP_OR, minus, a, NULL);
    return TestQP_new("- a", tree, NULL, 4);
}

static TestQueryParser*
syntax_test_unclosed_parens() {
    // Not a perfect result, but then it's not a good query string.
    Query *inner = make_poly_query(BOOLOP_OR, NULL);
    Query *tree = make_poly_query(BOOLOP_OR, inner, NULL);
    return TestQP_new("((", tree, NULL, 0);
}

static TestQueryParser*
syntax_test_unmatched_parens() {
    Query *tree = make_leaf_query(NULL, "a");
    return TestQP_new(")a)", tree, NULL, 4);
}

static TestQueryParser*
syntax_test_escaped_quotes_outside() {
    Query *tree = make_leaf_query(NULL, "\\\"a\\\"");
    return TestQP_new("\\\"a\\\"", tree, NULL, 4);
}

static TestQueryParser*
syntax_test_escaped_quotes_inside() {
    Query *tree = make_leaf_query(NULL, "\"\\\"a\\\"\"");
    return TestQP_new("\"\\\"a\\\"\"", tree, NULL, 4);
}

static TestQueryParser*
syntax_test_identifier_field_name() {
    // Field names must be identifiers, i.e. they cannot start with a number.
    Query *tree = make_leaf_query(NULL, "10:30");
    return TestQP_new("10:30", tree, NULL, 0);
}

static TestQueryParser*
syntax_test_double_colon() {
    Query *tree = make_leaf_query(NULL, "PHP::Interpreter");
    return TestQP_new("PHP::Interpreter", tree, NULL, 0);
}

/***************************************************************************/

typedef TestQueryParser*
(*LUCY_TestQPSyntax_Test_t)();

static LUCY_TestQPSyntax_Test_t leaf_test_funcs[] = {
    leaf_test_simple_term,
    leaf_test_simple_phrase,
    leaf_test_unclosed_quote,
    leaf_test_escaped_quotes_inside,
    leaf_test_escaped_quotes_outside,
    leaf_test_single_term_phrase,
    leaf_test_longer_phrase,
    leaf_test_empty_phrase,
    leaf_test_different_tokenization,
    leaf_test_phrase_with_stopwords,
    leaf_test_http,
    leaf_test_field,
    leaf_test_unrecognized_field,
    leaf_test_unescape_colons,
    NULL
};

static LUCY_TestQPSyntax_Test_t syntax_test_funcs[] = {
    syntax_test_minus_plus,
    syntax_test_plus_minus,
    syntax_test_minus_minus,
    syntax_test_not_minus,
    syntax_test_not_plus,
    syntax_test_padded_plus,
    syntax_test_padded_minus,
    syntax_test_unclosed_parens,
    syntax_test_unmatched_parens,
    syntax_test_escaped_quotes_outside,
    syntax_test_escaped_quotes_inside,
    syntax_test_identifier_field_name,
    syntax_test_double_colon,
    NULL
};

static void
test_query_parser_syntax(TestBatchRunner *runner) {
    if (!RegexTokenizer_is_available()) {
        for (uint32_t i = 0; leaf_test_funcs[i] != NULL; i++) {
            SKIP(runner, "RegexTokenizer not available");
            SKIP(runner, "RegexTokenizer not available");
            SKIP(runner, "RegexTokenizer not available");
        }

        for (uint32_t i = 0; syntax_test_funcs[i] != NULL; i++) {
            SKIP(runner, "RegexTokenizer not available");
            SKIP(runner, "RegexTokenizer not available");
        }

        return;
    }

    Folder        *index    = build_index();
    IndexSearcher *searcher = IxSearcher_new((Obj*)index);
    QueryParser   *qparser  = QParser_new(IxSearcher_Get_Schema(searcher),
                                          NULL, NULL, NULL);
    QParser_Set_Heed_Colons(qparser, true);

    for (uint32_t i = 0; leaf_test_funcs[i] != NULL; i++) {
        LUCY_TestQPSyntax_Test_t test_func = leaf_test_funcs[i];
        TestQueryParser *test_case = test_func();
        TestQueryParserIVARS *ivars = TestQP_IVARS(test_case);
        Query *tree     = QParser_Tree(qparser, ivars->query_string);
        Query *expanded = QParser_Expand_Leaf(qparser, ivars->tree);
        Query *parsed   = QParser_Parse(qparser, ivars->query_string);
        Hits  *hits     = IxSearcher_Hits(searcher, (Obj*)parsed, 0, 10, NULL);

        TEST_TRUE(runner, Query_Equals(tree, (Obj*)ivars->tree),
                  "tree()    %s", Str_Get_Ptr8(ivars->query_string));
        TEST_TRUE(runner, Query_Equals(expanded, (Obj*)ivars->expanded),
                  "expand_leaf()    %s", Str_Get_Ptr8(ivars->query_string));
        TEST_INT_EQ(runner, Hits_Total_Hits(hits), ivars->num_hits,
                    "hits:    %s", Str_Get_Ptr8(ivars->query_string));
        DECREF(hits);
        DECREF(parsed);
        DECREF(expanded);
        DECREF(tree);
        DECREF(test_case);
    }

    for (uint32_t i = 0; syntax_test_funcs[i] != NULL; i++) {
        LUCY_TestQPSyntax_Test_t test_func = syntax_test_funcs[i];
        TestQueryParser *test_case = test_func();
        TestQueryParserIVARS *ivars = TestQP_IVARS(test_case);
        Query *tree   = QParser_Tree(qparser, ivars->query_string);
        Query *parsed = QParser_Parse(qparser, ivars->query_string);
        Hits  *hits   = IxSearcher_Hits(searcher, (Obj*)parsed, 0, 10, NULL);

        TEST_TRUE(runner, Query_Equals(tree, (Obj*)ivars->tree),
                  "tree()    %s", Str_Get_Ptr8(ivars->query_string));
        TEST_INT_EQ(runner, Hits_Total_Hits(hits), ivars->num_hits,
                    "hits:    %s", Str_Get_Ptr8(ivars->query_string));
        DECREF(hits);
        DECREF(parsed);
        DECREF(tree);
        DECREF(test_case);
    }

    DECREF(searcher);
    DECREF(qparser);
    DECREF(index);
}

void
TestQPSyntax_Run_IMP(TestQueryParserSyntax *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 68);
    test_query_parser_syntax(runner);
}


