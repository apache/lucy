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

#define C_LUCY_QUERYLEXER
#include <stdlib.h>
#include <ctype.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/QueryParser/QueryLexer.h"
#include "Lucy/Search/QueryParser.h"
#include "Lucy/Search/QueryParser/ParserElem.h"

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

static ParserElem*
S_consume_keyword(StringIterator *iter, const char *keyword,
                  size_t keyword_len, int type);

static ParserElem*
S_consume_field(StringIterator *iter);

static ParserElem*
S_consume_text(StringIterator *iter);

static ParserElem*
S_consume_quoted_string(StringIterator *iter);

QueryLexer*
QueryLexer_new() {
    QueryLexer *self = (QueryLexer*)Class_Make_Obj(QUERYLEXER);
    return QueryLexer_init(self);
}

QueryLexer*
QueryLexer_init(QueryLexer *self) {
    QueryLexerIVARS *const ivars = QueryLexer_IVARS(self);
    ivars->heed_colons = false;
    return self;
}

bool
QueryLexer_Heed_Colons_IMP(QueryLexer *self) {
    return QueryLexer_IVARS(self)->heed_colons;
}

void
QueryLexer_Set_Heed_Colons_IMP(QueryLexer *self, bool heed_colons) {
    QueryLexer_IVARS(self)->heed_colons = heed_colons;
}

VArray*
QueryLexer_Tokenize_IMP(QueryLexer *self, String *query_string) {
    QueryLexerIVARS *const ivars = QueryLexer_IVARS(self);

    VArray *elems = VA_new(0);
    if (!query_string) { return elems; }

    StringIterator *iter = Str_Top(query_string);

    while (StrIter_Has_Next(iter)) {
        ParserElem *elem = NULL;

        if (StrIter_Skip_Next_Whitespace(iter)) {
            // Fast-forward past whitespace.
            continue;
        }

        if (ivars->heed_colons) {
            ParserElem *elem = S_consume_field(iter);
            if (elem) {
                VA_Push(elems, (Obj*)elem);
            }
        }

        int32_t code_point = StrIter_Next(iter);
        switch (code_point) {
            case '(':
                elem = ParserElem_new(TOKEN_OPEN_PAREN, NULL);
                break;
            case ')':
                elem = ParserElem_new(TOKEN_CLOSE_PAREN, NULL);
                break;
            case '+':
                if (StrIter_Has_Next(iter)
                    && !StrIter_Skip_Next_Whitespace(iter)
                   ) {
                    elem = ParserElem_new(TOKEN_PLUS, NULL);
                }
                else {
                    elem = ParserElem_new(TOKEN_STRING, (Obj*)Str_newf("+"));
                }
                break;
            case '-':
                if (StrIter_Has_Next(iter)
                    && !StrIter_Skip_Next_Whitespace(iter)
                   ) {
                    elem = ParserElem_new(TOKEN_MINUS, NULL);
                }
                else {
                    elem = ParserElem_new(TOKEN_STRING, (Obj*)Str_newf("-"));
                }
                break;
            case '"':
                StrIter_Recede(iter, 1);
                elem = S_consume_quoted_string(iter);
                break;
            case 'O':
                StrIter_Recede(iter, 1);
                elem = S_consume_keyword(iter, "OR", 2, TOKEN_OR);
                if (!elem) {
                    elem = S_consume_text(iter);
                }
                break;
            case 'A':
                StrIter_Recede(iter, 1);
                elem = S_consume_keyword(iter, "AND", 3, TOKEN_AND);
                if (!elem) {
                    elem = S_consume_text(iter);
                }
                break;
            case 'N':
                StrIter_Recede(iter, 1);
                elem = S_consume_keyword(iter, "NOT", 3, TOKEN_NOT);
                if (!elem) {
                    elem = S_consume_text(iter);
                }
                break;
            default:
                StrIter_Recede(iter, 1);
                elem = S_consume_text(iter);
                break;
        }
        VA_Push(elems, (Obj*)elem);
    }

    DECREF(iter);
    return elems;
}


static ParserElem*
S_consume_keyword(StringIterator *iter, const char *keyword,
                  size_t keyword_len, int type) {
    if (!StrIter_Starts_With_Utf8(iter, keyword, keyword_len)) {
        return NULL;
    }
    StringIterator *temp = StrIter_Clone(iter);
    StrIter_Advance(temp, keyword_len);
    int32_t lookahead = StrIter_Next(temp);
    if (lookahead == STRITER_DONE) {
        DECREF(temp);
        return NULL;
    }
    if (StrHelp_is_whitespace(lookahead)
        || lookahead == '"'
        || lookahead == '('
        || lookahead == ')'
        || lookahead == '+'
        || lookahead == '-'
       ) {
        StrIter_Recede(temp, 1);
        StrIter_Assign(iter, temp);
        DECREF(temp);
        return ParserElem_new(type, NULL);
    }
    DECREF(temp);
    return NULL;
}

static ParserElem*
S_consume_field(StringIterator *iter) {
    StringIterator *temp = StrIter_Clone(iter);

    // Field names constructs must start with a letter or underscore.
    int32_t code_point = StrIter_Next(temp);
    if (code_point == STRITER_DONE) {
        DECREF(temp);
        return NULL;
    }
    if (!(isalpha(code_point) || code_point == '_')) {
        DECREF(temp);
        return NULL;
    }

    // Only alphanumerics and underscores are allowed in field names.
    while (':' != (code_point = StrIter_Next(temp))) {
        if (code_point == STRITER_DONE) {
            DECREF(temp);
            return NULL;
        }
        if (!(isalnum(code_point) || code_point == '_')) {
            DECREF(temp);
            return NULL;
        }
    }

    // Field name constructs must be followed by something sensible.
    int32_t lookahead = StrIter_Next(temp);
    if (lookahead == STRITER_DONE) {
        DECREF(temp);
        return NULL;
    }
    if (!(isalnum(lookahead)
          || lookahead == '_'
          || lookahead > 127
          || lookahead == '"'
          || lookahead == '('
         )
       ) {
        DECREF(temp);
        return NULL;
    }

    // Consume string data.
    StrIter_Recede(temp, 2); // Back up over lookahead and colon.
    String *field = StrIter_substring(iter, temp);
    StrIter_Advance(temp, 1); // Skip colon.
    StrIter_Assign(iter, temp);
    DECREF(temp);
    return ParserElem_new(TOKEN_FIELD, (Obj*)field);
}

static ParserElem*
S_consume_text(StringIterator *iter) {
    StringIterator *temp = StrIter_Clone(iter);

    while (1) {
        int32_t code_point = StrIter_Next(temp);
        if (code_point == '\\') {
            code_point = StrIter_Next(temp);
            if (code_point == STRITER_DONE) {
                break;
            }
        }
        else if (code_point == STRITER_DONE) {
            break;
        }
        else if (StrHelp_is_whitespace(code_point)
            || code_point == '"'
            || code_point == '('
            || code_point == ')'
           ) {
            StrIter_Recede(temp, 1);
            break;
        }
    }

    String *text = StrIter_substring(iter, temp);
    StrIter_Assign(iter, temp);
    DECREF(temp);
    return ParserElem_new(TOKEN_STRING, (Obj*)text);
}

static ParserElem*
S_consume_quoted_string(StringIterator *iter) {
    StringIterator *temp = StrIter_Clone(iter);

    if (StrIter_Next(temp) != '"') {
        THROW(ERR, "Internal error: expected a quote");
    }

    while (1) {
        int32_t code_point = StrIter_Next(temp);
        if (code_point == STRITER_DONE || code_point == '"') {
            break;
        }
        else if (code_point == '\\') {
            StrIter_Next(temp);
        }
    }

    String *text = StrIter_substring(iter, temp);
    StrIter_Assign(iter, temp);
    DECREF(temp);
    return ParserElem_new(TOKEN_STRING, (Obj*)text);
}

