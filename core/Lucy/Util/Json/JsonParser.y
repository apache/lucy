%name LucyParseJson

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

%token_type { cfish_Obj* }
%token_destructor { CFISH_DECREF($$); }
%token_prefix LUCY_JSON_TOKENTYPE_

%include {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Lucy/Object/Hash.h"
#include "Lucy/Object/VArray.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Util/Json.h"
}

%extra_argument { lucy_JsonParserState *state }

%syntax_error {
    state->errors = true;
}

result ::= top_level_value(A).
{
     state->result = A;
}

/* Allow any "value" as a top-level construct.  This "loose", tolerant grammar
 * makes testing somewhat easier.  A strict JSON parser would only allow JSON
 * Objects and Arrays at the top level.
 */
top_level_value(A) ::= value(B).  { A = B; }

/* Structural characters. */
begin_array     ::= LEFT_SQUARE_BRACKET.
end_array       ::= RIGHT_SQUARE_BRACKET.
begin_object    ::= LEFT_CURLY_BRACKET.
end_object      ::= RIGHT_CURLY_BRACKET.
name_separator  ::= COLON.
value_separator ::= COMMA.

/* Values */
%type STRING { cfish_CharBuf* }

value(A) ::= FALSE(B).   { A = B; }
value(A) ::= NULL(B).    { A = B; }
value(A) ::= TRUE(B).    { A = B; }
value(A) ::= object(B).  { A = (cfish_Obj*)B; }
value(A) ::= array(B).   { A = (cfish_Obj*)B; }
value(A) ::= NUMBER(B).  { A = (cfish_Obj*)B; }
value(A) ::= STRING(B).  { A = B; }

/* Javascript Objects, implemented as Clownfish Hashes. */
%type object                    { cfish_Hash* }
%type empty_object              { cfish_Hash* }
%type single_pair_object        { cfish_Hash* }
%type multi_pair_object         { cfish_Hash* }
%type key_value_pair_list       { cfish_Hash* }
%destructor object              { CFISH_DECREF($$); }
%destructor empty_object        { CFISH_DECREF($$); }
%destructor single_pair_object  { CFISH_DECREF($$); }
%destructor multi_pair_object   { CFISH_DECREF($$); }
%destructor key_value_pair_list { CFISH_DECREF($$); }

object(A) ::= empty_object(B).         { A = B; }
object(A) ::= single_pair_object(B).   { A = B; }
object(A) ::= multi_pair_object(B).    { A = B; }

empty_object(A) ::= begin_object end_object.
{ 
    A = cfish_Hash_new(0);
}

single_pair_object(A) ::= begin_object STRING(B) name_separator value(C) end_object.
{
    A = cfish_Hash_new(1);
    Cfish_Hash_Store(A, (cfish_Obj*)B, C);
    CFISH_DECREF(B);
}

multi_pair_object(A) ::= begin_object key_value_pair_list(B) STRING(C) name_separator value(D) end_object.
{
    A = B;
    Cfish_Hash_Store(A, (cfish_Obj*)C, D);
    CFISH_DECREF(C);
}

key_value_pair_list(A) ::= key_value_pair_list(B) STRING(C) name_separator value(D) value_separator.
{ 
    A = B; 
    Cfish_Hash_Store(A, (cfish_Obj*)C, D);
    CFISH_DECREF(C);
}

key_value_pair_list(A) ::= STRING(B) name_separator value(C) value_separator.
{
    A = cfish_Hash_new(0);
    Cfish_Hash_Store(A, (cfish_Obj*)B, C);
    CFISH_DECREF(B);
}

/* Arrays. */
%type array                     { cfish_VArray* }
%type empty_array               { cfish_VArray* }
%type single_elem_array         { cfish_VArray* }
%type multi_elem_array          { cfish_VArray* }
%type array_elem_list           { cfish_VArray* }
%destructor array               { CFISH_DECREF($$); }
%destructor single_elem_array   { CFISH_DECREF($$); }
%destructor multi_elem_array    { CFISH_DECREF($$); }
%destructor array_elem_list     { CFISH_DECREF($$); }

array(A) ::= empty_array(B).       { A = B; }
array(A) ::= single_elem_array(B). { A = B; }
array(A) ::= multi_elem_array(B).  { A = B; }

empty_array(A) ::= begin_array end_array.
{
    A = cfish_VA_new(0);
}

single_elem_array(A) ::= begin_array value(B) end_array.
{
    A = cfish_VA_new(1);
    Cfish_VA_Push(A, B);
}

multi_elem_array(A) ::= begin_array array_elem_list(B) value(C) end_array.
{
    A = B;
    Cfish_VA_Push(A, C);
}

array_elem_list(A) ::= array_elem_list(B) value(C) value_separator. 
{ 
    A = B; 
    Cfish_VA_Push(A, C);
}

array_elem_list(A) ::= value(B) value_separator.
{
    A = cfish_VA_new(1);
    Cfish_VA_Push(A, B);
}

