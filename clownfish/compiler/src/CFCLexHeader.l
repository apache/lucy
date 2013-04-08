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

%{
    #include "CFC.h"
    #include "CFCParseHeader.h"

    /* Dupe yytext and invoke Lemon-generated parser. */
    #define PARSE(token_type) \
        CFCParseHeader(CFCParser_current_parser, token_type, \
			CFCParser_dupe(CFCParser_current_state, yytext), \
			CFCParser_current_state)

    struct cfc_StringID {
        const char *string;
        int token_type;
    };
    struct cfc_StringID reserved_word_map[] = {
        {"NULL", CFC_TOKENTYPE_NULL },
        {"abstract", CFC_TOKENTYPE_ABSTRACT },
        {"bool", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"char", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"class", CFC_TOKENTYPE_CLASS },
        {"cnick", CFC_TOKENTYPE_CNICK },
        {"const", CFC_TOKENTYPE_CONST },
        {"decremented", CFC_TOKENTYPE_DECREMENTED },
        {"double", CFC_TOKENTYPE_FLOAT_TYPE_NAME },
        {"false", CFC_TOKENTYPE_FALSE },
        {"final", CFC_TOKENTYPE_FINAL },
        {"float", CFC_TOKENTYPE_FLOAT_TYPE_NAME },
        {"incremented", CFC_TOKENTYPE_INCREMENTED },
        {"inert", CFC_TOKENTYPE_INERT },
        {"inherits", CFC_TOKENTYPE_INHERITS },
        {"inline", CFC_TOKENTYPE_INLINE },
        {"int", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"int16_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"int32_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"int64_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"int8_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"long", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"nullable", CFC_TOKENTYPE_NULLABLE },
        {"parcel", CFC_TOKENTYPE_PARCEL },
        {"private", CFC_TOKENTYPE_PRIVATE },
        {"public", CFC_TOKENTYPE_PUBLIC },
        {"short", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"size_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"true", CFC_TOKENTYPE_TRUE },
        {"uint16_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"uint32_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"uint64_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"uint8_t", CFC_TOKENTYPE_INTEGER_TYPE_NAME },
        {"va_list", CFC_TOKENTYPE_VA_LIST },
        {"void", CFC_TOKENTYPE_VOID },
    };
    #define NUM_RESERVED_WORDS \
        (sizeof(reserved_word_map) / sizeof(struct cfc_StringID))

    static int
    S_compare(const void *va, const void *vb) {
        const char *a = (const char*)va;
        struct cfc_StringID *b
            = (struct cfc_StringID*)vb;
        return strcmp(a, b->string);
    }

    static int
    S_identifier_or_keyword(const char *word) {
        struct cfc_StringID *got = (struct cfc_StringID*)
            bsearch(word, reserved_word_map, NUM_RESERVED_WORDS,
                    sizeof(struct cfc_StringID), S_compare);
        if (got) {
            return got->token_type;
        }
        else {
            return CFC_TOKENTYPE_IDENTIFIER;
        }
    }
%}

%option noyywrap
%option nodefault
%option yylineno
%option never-interactive

%x CBLOCK

%%
::         { PARSE(CFC_TOKENTYPE_SCOPE_OP); }
[*]        { PARSE(CFC_TOKENTYPE_ASTERISK); }
\{         { PARSE(CFC_TOKENTYPE_LEFT_CURLY_BRACE); }
\}         { PARSE(CFC_TOKENTYPE_RIGHT_CURLY_BRACE); }
[\[]       { PARSE(CFC_TOKENTYPE_LEFT_SQUARE_BRACKET); }
[\]]       { PARSE(CFC_TOKENTYPE_RIGHT_SQUARE_BRACKET); }
[\(]       { PARSE(CFC_TOKENTYPE_LEFT_PAREN); }
[\)]       { PARSE(CFC_TOKENTYPE_RIGHT_PAREN); }
\.\.\.     { PARSE(CFC_TOKENTYPE_ELLIPSIS); }
,          { PARSE(CFC_TOKENTYPE_COMMA); }
;          { PARSE(CFC_TOKENTYPE_SEMICOLON); }
:          { PARSE(CFC_TOKENTYPE_COLON); }
=          { PARSE(CFC_TOKENTYPE_EQUALS); }

-?0x[0-9A-Fa-f]+       { PARSE(CFC_TOKENTYPE_HEX_LITERAL); }
-?[0-9]+\.[0-9]+       { PARSE(CFC_TOKENTYPE_FLOAT_LITERAL); }
-?[0-9]+               { PARSE(CFC_TOKENTYPE_INTEGER_LITERAL); }
\"([^\"\\]|\\.)*\"     { PARSE(CFC_TOKENTYPE_STRING_LITERAL); }

[a-zA-Z_][a-zA-Z0-9_]* { PARSE(S_identifier_or_keyword(yytext)); }

__C__[[:space:]]*    { BEGIN(CBLOCK);  PARSE(CFC_TOKENTYPE_CBLOCK_START); }
<CBLOCK>__END_C__    { BEGIN(INITIAL); PARSE(CFC_TOKENTYPE_CBLOCK_CLOSE); }
<CBLOCK>__END_C_[^_]   { PARSE(CFC_TOKENTYPE_BLOB); }
<CBLOCK>[^_]+          { PARSE(CFC_TOKENTYPE_BLOB); }
<CBLOCK>_+             { PARSE(CFC_TOKENTYPE_BLOB); }
           
    /* Parse docucomments, but skip ordinary comments */
"/**"([^*]|"*"[^/])*"*/" { PARSE(CFC_TOKENTYPE_DOCUCOMMENT); }
"/*"([^*]|"*"[^/])*"*/"

[ \t\r\n]  /* Skip whitespace. */
<*>.       { 
                printf("Bad input character '%s' at line %d\n", yytext, yylineno);
                yyterminate();
           }
<<EOF>>    { yyterminate(); }
%%

