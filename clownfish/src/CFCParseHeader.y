%name CFCParseHeader

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

%token_type {char*}
%token_prefix CFC_TOKENTYPE_

%extra_argument { CFCParser *state }

%include {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "CFC.h"
#ifndef true
  #define true 1
  #define false 0
#endif

static CFCClass*
S_start_class(CFCParser *state, CFCDocuComment *docucomment, char *exposure,
              char *declaration_modifier_list, char *class_name,
              char *class_cnick, char *inheritance) {
    CFCFileSpec *file_spec = CFCParser_get_file_spec(state);
    int is_final = false;
    int is_inert = false;
    if (declaration_modifier_list) {
        is_final = !!strstr(declaration_modifier_list, "final");
        is_inert = !!strstr(declaration_modifier_list, "inert");
    }
    CFCParser_set_class_name(state, class_name);
    CFCParser_set_class_cnick(state, class_cnick);
    CFCClass *klass = CFCClass_create(CFCParser_get_parcel(state), exposure,
                                      class_name, class_cnick, NULL,
                                      docucomment, file_spec, inheritance,
                                      is_final, is_inert);
    CFCBase_decref((CFCBase*)docucomment);
    return klass;
}

static CFCVariable*
S_new_var(CFCParser *state, char *exposure, char *modifiers, CFCType *type,
          char *name) {
    int inert = false;
    if (modifiers) {
        if (strcmp(modifiers, "inert") != 0) {
            CFCUtil_die("Illegal variable modifiers: '%s'", modifiers);
        }
        inert = true;
    }

    CFCParcel  *parcel      = NULL;
    const char *class_name  = NULL;
    const char *class_cnick = NULL;
    if (exposure && strcmp(exposure, "local") != 0) {
        parcel      = CFCParser_get_parcel(state);
        class_name  = CFCParser_get_class_name(state);
        class_cnick = CFCParser_get_class_cnick(state);
    }
    CFCVariable *var = CFCVariable_new(parcel, exposure, class_name,
                                       class_cnick, name, type, inert);

    /* Consume tokens. */
    CFCBase_decref((CFCBase*)type);

    return var;
}

static CFCBase*
S_new_sub(CFCParser *state, CFCDocuComment *docucomment, 
          char *exposure, char *declaration_modifier_list,
          CFCType *type, char *name, CFCParamList *param_list) {
    CFCParcel  *parcel      = CFCParser_get_parcel(state);
    const char *class_name  = CFCParser_get_class_name(state);
    const char *class_cnick = CFCParser_get_class_cnick(state);

    /* Find modifiers by scanning the list. */
    int is_abstract = false;
    int is_final    = false;
    int is_inline   = false;
    int is_inert    = false;
    if (declaration_modifier_list) {
        is_abstract = !!strstr(declaration_modifier_list, "abstract");
        is_final    = !!strstr(declaration_modifier_list, "final");
        is_inline   = !!strstr(declaration_modifier_list, "inline");
        is_inert    = !!strstr(declaration_modifier_list, "inert");
    }

    /* If "inert", it's a function, otherwise it's a method. */
    CFCBase *sub;
    if (is_inert) {
        sub = (CFCBase*)CFCFunction_new(parcel, exposure, class_name,
                                         class_cnick, name, type, param_list,
                                         docucomment, is_inline);
    }
    else {
        sub = (CFCBase*)CFCMethod_new(parcel, exposure, class_name,
                                       class_cnick, name, type, param_list,
                                       docucomment, is_final, is_abstract);
    }

    /* Consume tokens. */
    CFCBase_decref((CFCBase*)docucomment);
    CFCBase_decref((CFCBase*)type);
    CFCBase_decref((CFCBase*)param_list);

    return sub;
}

static CFCType*
S_new_type(CFCParser *state, int flags, char *type_name,
           char *asterisk_postfix, char *array_postfix) {
    (void)state; /* unused */
    CFCType *type = NULL;
    size_t type_name_len = strlen(type_name);
    int indirection = asterisk_postfix ? strlen(asterisk_postfix) : 0;

    /* Apply "nullable" to outermost pointer, but "const", etc to innermost
     * type. This is an ugly kludge and the Clownfish header language needs to
     * be fixed, either to support C's terrible pointer type syntax, or to do
     * something better. */
    int composite_flags = 0;
    if (indirection) {
        composite_flags = flags & CFCTYPE_NULLABLE;
        flags &= ~CFCTYPE_NULLABLE;
    }

    if (!strcmp(type_name, "int8_t")
        || !strcmp(type_name, "int16_t")
        || !strcmp(type_name, "int32_t")
        || !strcmp(type_name, "int64_t")
        || !strcmp(type_name, "uint8_t")
        || !strcmp(type_name, "uint16_t")
        || !strcmp(type_name, "uint32_t")
        || !strcmp(type_name, "uint64_t")
        || !strcmp(type_name, "char")
        || !strcmp(type_name, "short")
        || !strcmp(type_name, "int")
        || !strcmp(type_name, "long")
        || !strcmp(type_name, "size_t")
        || !strcmp(type_name, "bool_t")
       ) {
        type = CFCType_new_integer(flags, type_name);
    }
    else if (!strcmp(type_name, "float")
             || !strcmp(type_name, "double")
            ) {
        type = CFCType_new_float(flags, type_name);
    }
    else if (!strcmp(type_name, "void")) {
        type = CFCType_new_void(!!(flags & CFCTYPE_CONST));
    }
    else if (!strcmp(type_name, "va_list")) {
        type = CFCType_new_va_list();
    }
    else if (type_name_len > 2
             && !strcmp(type_name + type_name_len - 2, "_t")
            ) {
        type = CFCType_new_arbitrary(CFCParser_get_parcel(state), type_name);
    }
    else if (indirection > 0) {
        /* The only remaining possibility is an object type, and we can let
         * the constructor perform the complex validation of the type name. */
        indirection--;
        if (indirection == 0) {
            flags |= composite_flags;
            composite_flags = 0;
        }
        type = CFCType_new_object(flags, CFCParser_get_parcel(state), type_name, 1);
    }
    else {
        CFCUtil_die("Invalid type specification at/near '%s'", type_name);
    }

    if (indirection) {
        CFCType *composite = CFCType_new_composite(composite_flags, type,
                                                   indirection, NULL);
        CFCBase_decref((CFCBase*)type);
        type = composite;
    }
    else if (array_postfix) {
        CFCType *composite = CFCType_new_composite(composite_flags, type,
                                                   0, array_postfix);
        CFCBase_decref((CFCBase*)type);
        type = composite;
    }

    return type;
}

} /* End include block. */

%syntax_error {
    CFCParser_set_errors(state, true);
}

%type result                            {CFCBase*}
%type file                              {CFCFile*}
%type major_block                       {CFCBase*}
%type parcel_definition                 {CFCParcel*}
%type class_declaration                 {CFCClass*}
%type class_head                        {CFCClass*}
%type class_defs                        {CFCClass*}
%type var_declaration_statement         {CFCVariable*}
%type subroutine_declaration_statement  {CFCBase*}
%type type                              {CFCType*}
%type param_variable                    {CFCVariable*}
%type param_list                        {CFCParamList*}
%type param_list_elems                  {CFCParamList*}
%type docucomment                       {CFCDocuComment*}
%type cblock                            {CFCCBlock*}
%type type_qualifier                    {int}
%type type_qualifier_list               {int}

%destructor result                            { CFCBase_decref((CFCBase*)$$); }
%destructor file                              { CFCBase_decref((CFCBase*)$$); }
%destructor major_block                       { CFCBase_decref((CFCBase*)$$); }
%destructor parcel_definition                 { CFCBase_decref((CFCBase*)$$); }
%destructor class_declaration                 { CFCBase_decref((CFCBase*)$$); }
%destructor class_head                        { CFCBase_decref((CFCBase*)$$); }
%destructor class_defs                        { CFCBase_decref((CFCBase*)$$); }
%destructor var_declaration_statement         { CFCBase_decref((CFCBase*)$$); }
%destructor subroutine_declaration_statement  { CFCBase_decref((CFCBase*)$$); }
%destructor type                              { CFCBase_decref((CFCBase*)$$); }
%destructor param_variable                    { CFCBase_decref((CFCBase*)$$); }
%destructor param_list                        { CFCBase_decref((CFCBase*)$$); }
%destructor param_list_elems                  { CFCBase_decref((CFCBase*)$$); }
%destructor docucomment                       { CFCBase_decref((CFCBase*)$$); }
%destructor cblock                            { CFCBase_decref((CFCBase*)$$); }

result ::= type(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= param_list(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= param_variable(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= docucomment(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= parcel_definition(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= cblock(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= var_declaration_statement(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= subroutine_declaration_statement(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= class_declaration(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}
result ::= file(A).
{
    CFCParser_set_result(state, (CFCBase*)A);
    CFCBase_decref((CFCBase*)A);
}

file(A) ::= FILE_START. /* Pseudo token, not passed by lexer. */
{
    A = CFCFile_new(CFCParser_get_file_spec(state));
}
file(A) ::= file(B) major_block(C).
{
    A = B;
    CFCFile_add_block(A, C);
    CFCBase_decref((CFCBase*)C);
}

major_block(A) ::= class_declaration(B). { A = (CFCBase*)B; }
major_block(A) ::= cblock(B).            { A = (CFCBase*)B; }
major_block(A) ::= parcel_definition(B). { A = (CFCBase*)B; }

parcel_definition(A) ::= exposure_specifier(B) qualified_id(C) SEMICOLON.
{
    if (strcmp(B, "parcel") != 0) {
        /* Instead of this kludgy post-parse error trigger, we should require
         * PARCEL in this production as opposed to exposure_specifier.
         * However, that causes a parsing conflict because the keyword
         * "parcel" has two meanings in the Clownfish header language (parcel
         * declaration and exposure specifier). */
         CFCUtil_die("A syntax error was detected when parsing '%s'", B);
    }
    A = CFCParcel_fetch(C);
    if (!A) {
        A = CFCParcel_new(C, NULL);
        CFCParcel_register(A);
        CFCBase_decref((CFCBase*)A);
    }
    CFCBase_incref((CFCBase*)A);
    CFCParser_set_parcel(state, A);
}

class_declaration(A) ::= class_defs(B) RIGHT_CURLY_BRACE.
{
    A = B;
    CFCParser_set_class_name(state, NULL);
    CFCParser_set_class_cnick(state, NULL);
}

class_head(A) ::= docucomment(B) exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, B,    C,    D,    E,    F,    G   ); }
class_head(A) ::=                exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, NULL, C,    D,    E,    F,    G   ); }
class_head(A) ::= docucomment(B)                       declaration_modifier_list(D) CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, B,    NULL, D,    E,    F,    G   ); }
class_head(A) ::=                                      declaration_modifier_list(D) CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, NULL, NULL, D,    E,    F,    G   ); }
class_head(A) ::= docucomment(B) exposure_specifier(C)                              CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, B,    C,    NULL, E,    F,    G   ); }
class_head(A) ::=                exposure_specifier(C)                              CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, NULL, C,    NULL, E,    F,    G   ); }
class_head(A) ::= docucomment(B)                                                    CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, B,    NULL, NULL, E,    F,    G   ); }
class_head(A) ::=                                                                   CLASS qualified_id(E) cnick(F) class_inheritance(G).  { A = S_start_class(state, NULL, NULL, NULL, E,    F,    G   ); }
class_head(A) ::= docucomment(B) exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, B,    C,    D,    E,    NULL, G   ); }
class_head(A) ::=                exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, NULL, C,    D,    E,    NULL, G   ); }
class_head(A) ::= docucomment(B)                       declaration_modifier_list(D) CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, B,    NULL, D,    E,    NULL, G   ); }
class_head(A) ::=                                      declaration_modifier_list(D) CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, NULL, NULL, D,    E,    NULL, G   ); }
class_head(A) ::= docucomment(B) exposure_specifier(C)                              CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, B,    C,    NULL, E,    NULL, G   ); }
class_head(A) ::=                exposure_specifier(C)                              CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, NULL, C,    NULL, E,    NULL, G   ); }
class_head(A) ::= docucomment(B)                                                    CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, B,    NULL, NULL, E,    NULL, G   ); }
class_head(A) ::=                                                                   CLASS qualified_id(E)          class_inheritance(G).  { A = S_start_class(state, NULL, NULL, NULL, E,    NULL, G   ); }
class_head(A) ::= docucomment(B) exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, B,    C,    D,    E,    F,    NULL ); }
class_head(A) ::=                exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, NULL, C,    D,    E,    F,    NULL ); }
class_head(A) ::= docucomment(B)                       declaration_modifier_list(D) CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, B,    NULL, D,    E,    F,    NULL ); }
class_head(A) ::=                                      declaration_modifier_list(D) CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, NULL, NULL, D,    E,    F,    NULL ); }
class_head(A) ::= docucomment(B) exposure_specifier(C)                              CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, B,    C,    NULL, E,    F,    NULL ); }
class_head(A) ::=                exposure_specifier(C)                              CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, NULL, C,    NULL, E,    F,    NULL ); }
class_head(A) ::= docucomment(B)                                                    CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, B,    NULL, NULL, E,    F,    NULL ); }
class_head(A) ::=                                                                   CLASS qualified_id(E) cnick(F)                     .  { A = S_start_class(state, NULL, NULL, NULL, E,    F,    NULL ); }
class_head(A) ::= docucomment(B) exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E)                              .  { A = S_start_class(state, B,    C,    D,    E,    NULL, NULL ); }
class_head(A) ::=                exposure_specifier(C) declaration_modifier_list(D) CLASS qualified_id(E)                              .  { A = S_start_class(state, NULL, C,    D,    E,    NULL, NULL ); }
class_head(A) ::= docucomment(B)                       declaration_modifier_list(D) CLASS qualified_id(E)                              .  { A = S_start_class(state, B,    NULL, D,    E,    NULL, NULL ); }
class_head(A) ::=                                      declaration_modifier_list(D) CLASS qualified_id(E)                              .  { A = S_start_class(state, NULL, NULL, D,    E,    NULL, NULL ); }
class_head(A) ::= docucomment(B) exposure_specifier(C)                              CLASS qualified_id(E)                              .  { A = S_start_class(state, B,    C,    NULL, E,    NULL, NULL ); }
class_head(A) ::=                exposure_specifier(C)                              CLASS qualified_id(E)                              .  { A = S_start_class(state, NULL, C,    NULL, E,    NULL, NULL ); }
class_head(A) ::= docucomment(B)                                                    CLASS qualified_id(E)                              .  { A = S_start_class(state, B,    NULL, NULL, E,    NULL, NULL ); }
class_head(A) ::=                                                                   CLASS qualified_id(E)                              .  { A = S_start_class(state, NULL, NULL, NULL, E,    NULL, NULL ); }

class_head(A) ::= class_head(B) COLON IDENTIFIER(C).
{
    A = B;
    CFCClass_add_attribute(A, C, "1");
}

class_defs(A) ::= class_head(B) LEFT_CURLY_BRACE.
{
    A = B;
}
class_defs(A) ::= class_defs(B) var_declaration_statement(C).
{
    A = B;
    if (CFCVariable_inert(C)) {
        CFCClass_add_inert_var(A, C);
    }
    else {
        CFCClass_add_member_var(A, C);
    }
    CFCBase_decref((CFCBase*)C);
}
class_defs(A) ::= class_defs(B) subroutine_declaration_statement(C).
{
    A = B;
    if (strcmp(CFCBase_get_cfc_class(C), "Clownfish::CFC::Model::Function") == 0) {
        CFCClass_add_function(A, (CFCFunction*)C);
    }
    else {
        CFCClass_add_method(A, (CFCMethod*)C);
    }
    CFCBase_decref((CFCBase*)C);
}

var_declaration_statement(A) ::= 
    type(D) declarator(E) SEMICOLON.
{
    A = S_new_var(state, CFCParser_dupe(state, "parcel"), NULL, D, E);
}
var_declaration_statement(A) ::= 
    exposure_specifier(B)
    type(D) declarator(E) SEMICOLON.
{
    A = S_new_var(state, B, NULL, D, E);
}
var_declaration_statement(A) ::= 
    declaration_modifier_list(C)
    type(D) declarator(E) SEMICOLON.
{
    A = S_new_var(state, CFCParser_dupe(state, "parcel"), C, D, E);
}
var_declaration_statement(A) ::= 
    exposure_specifier(B)
    declaration_modifier_list(C)
    type(D) declarator(E) SEMICOLON.
{
    A = S_new_var(state, B, C, D, E);
}

subroutine_declaration_statement(A) ::= 
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, NULL, NULL, NULL, E, F, G);
}
subroutine_declaration_statement(A) ::= 
    declaration_modifier_list(D)
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, NULL, NULL, D, E, F, G);
}
subroutine_declaration_statement(A) ::= 
    exposure_specifier(C)
    declaration_modifier_list(D)
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, NULL, C, D, E, F, G);
}
subroutine_declaration_statement(A) ::= 
    exposure_specifier(C)
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, NULL, C, NULL, E, F, G);
}
subroutine_declaration_statement(A) ::= 
    docucomment(B)
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, B, NULL, NULL, E, F, G);
}
subroutine_declaration_statement(A) ::= 
    docucomment(B)
    declaration_modifier_list(D)
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, B, NULL, D, E, F, G);
}
subroutine_declaration_statement(A) ::= 
    docucomment(B)
    exposure_specifier(C)
    declaration_modifier_list(D)
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, B, C, D, E, F, G);
}
subroutine_declaration_statement(A) ::= 
    docucomment(B)
    exposure_specifier(C)
    type(E) declarator(F) param_list(G) SEMICOLON.
{
    A = S_new_sub(state, B, C, NULL, E, F, G);
}

type(A) ::= type_name(C).
{
    A = S_new_type(state, 0, C, NULL, NULL);
}
type(A) ::= type_name(C) asterisk_postfix(D).
{
    A = S_new_type(state, 0, C, D, NULL);
}
type(A) ::= type_name(C) array_postfix(E).
{
    A = S_new_type(state, 0, C, NULL, E);
}
type(A) ::= type_qualifier_list(B) type_name(C).
{
    A = S_new_type(state, B, C, NULL, NULL);
}
type(A) ::= type_qualifier_list(B) type_name(C) asterisk_postfix(D).
{
    A = S_new_type(state, B, C, D, NULL);
}
type(A) ::= type_qualifier_list(B) type_name(C) array_postfix(E).
{
    A = S_new_type(state, B, C, NULL, E);
}

type_name(A) ::= VOID(B).               { A = B; }
type_name(A) ::= VA_LIST(B).            { A = B; }
type_name(A) ::= INTEGER_TYPE_NAME(B).  { A = B; }
type_name(A) ::= FLOAT_TYPE_NAME(B).    { A = B; }
type_name(A) ::= IDENTIFIER(B).         { A = B; }

exposure_specifier(A) ::= PUBLIC(B).  { A = B; }
exposure_specifier(A) ::= PRIVATE(B). { A = B; }
exposure_specifier(A) ::= PARCEL(B).  { A = B; }
exposure_specifier(A) ::= LOCAL(B).   { A = B; }

type_qualifier(A) ::= CONST.       { A = CFCTYPE_CONST; }
type_qualifier(A) ::= NULLABLE.    { A = CFCTYPE_NULLABLE; }
type_qualifier(A) ::= INCREMENTED. { A = CFCTYPE_INCREMENTED; }
type_qualifier(A) ::= DECREMENTED. { A = CFCTYPE_DECREMENTED; }

type_qualifier_list(A) ::= type_qualifier(B).
{
    A = B;
}
type_qualifier_list(A) ::= type_qualifier_list(B) type_qualifier(C).
{
    A = B;
    A |= C;
}

declaration_modifier(A) ::= INERT(B).      { A = B; }
declaration_modifier(A) ::= INLINE(B).     { A = B; }
declaration_modifier(A) ::= ABSTRACT(B).   { A = B; }
declaration_modifier(A) ::= FINAL(B).      { A = B; }

declaration_modifier_list(A) ::= declaration_modifier(B). { A = B; }
declaration_modifier_list(A) ::= declaration_modifier_list(B) declaration_modifier(C).
{
    size_t size = strlen(B) + strlen(C) + 2;
    A = (char*)CFCParser_allocate(state, size);
    sprintf(A, "%s %s", B, C);
}

asterisk_postfix(A) ::= ASTERISK(B). { A = B; }
asterisk_postfix(A) ::= asterisk_postfix(B) ASTERISK.
{
    size_t size = strlen(B) + 2;
    A = (char*)CFCParser_allocate(state, size);
    sprintf(A, "%s*", B);
}

array_postfix_elem(A) ::= LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET.
{
    A = CFCParser_dupe(state, "[]");
}
array_postfix_elem(A) ::= LEFT_SQUARE_BRACKET INTEGER_LITERAL(B) RIGHT_SQUARE_BRACKET.
{
    size_t size = strlen(B) + 3;
    A = (char*)CFCParser_allocate(state, size);
    sprintf(A, "[%s]", B);
}

array_postfix(A) ::= array_postfix_elem(B). { A = B; }
array_postfix(A) ::= array_postfix(B) array_postfix_elem(C).
{
    size_t size = strlen(B) + strlen(C) + 1;
    A = (char*)CFCParser_allocate(state, size);
    sprintf(A, "%s%s", B, C);
}

scalar_constant(A) ::= HEX_LITERAL(B).     { A = B; }
scalar_constant(A) ::= FLOAT_LITERAL(B).   { A = B; }
scalar_constant(A) ::= INTEGER_LITERAL(B). { A = B; }
scalar_constant(A) ::= STRING_LITERAL(B).  { A = B; }
scalar_constant(A) ::= TRUE(B).            { A = B; }
scalar_constant(A) ::= FALSE(B).           { A = B; }
scalar_constant(A) ::= NULL(B).            { A = B; }

declarator(A) ::= IDENTIFIER(B).
{
    A = B;
}

param_variable(A) ::= type(B) declarator(C).
{
    A = S_new_var(state, NULL, NULL, B, C);
}

param_list(A) ::= LEFT_PAREN RIGHT_PAREN.
{
    A = CFCParamList_new(false);
}
param_list(A) ::= LEFT_PAREN param_list_elems(B) RIGHT_PAREN.
{
    A = B;
}
param_list(A) ::= LEFT_PAREN param_list_elems(B) COMMA ELLIPSIS RIGHT_PAREN.
{
    A = B;
    CFCParamList_set_variadic(A, true);
}
param_list_elems(A) ::= param_list_elems(B) COMMA param_variable(C).
{
    A = B;
    CFCParamList_add_param(A, C, NULL);
    CFCBase_decref((CFCBase*)C);
}
param_list_elems(A) ::= param_list_elems(B) COMMA param_variable(C) EQUALS scalar_constant(D).
{
    A = B;
    CFCParamList_add_param(A, C, D);
    CFCBase_decref((CFCBase*)C);
}
param_list_elems(A) ::= param_variable(B).
{
    A = CFCParamList_new(false);
    CFCParamList_add_param(A, B, NULL);
    CFCBase_decref((CFCBase*)B);
}
param_list_elems(A) ::= param_variable(B) EQUALS scalar_constant(C).
{
    A = CFCParamList_new(false);
    CFCParamList_add_param(A, B, C);
    CFCBase_decref((CFCBase*)B);
}

qualified_id(A) ::= IDENTIFIER(B). { A = B; }
qualified_id(A) ::= qualified_id(B) SCOPE_OP IDENTIFIER(C).
{
    size_t size = strlen(B) + strlen(C) + 3;
    A = (char*)CFCParser_allocate(state, size);
    sprintf(A, "%s::%s", B, C);
}

docucomment(A)       ::= DOCUCOMMENT(B).                     { A = CFCDocuComment_parse(B); }
class_inheritance(A) ::= INHERITS qualified_id(B).           { A = B; }
cnick(A)             ::= CNICK IDENTIFIER(B).                { A = B; }
cblock(A)            ::= CBLOCK_START blob(B) CBLOCK_CLOSE.  { A = CFCCBlock_new(B); }
cblock(A)            ::= CBLOCK_START CBLOCK_CLOSE.          { A = CFCCBlock_new(""); }

blob(A) ::= BLOB(B). { A = B; }
blob(A) ::= blob(B) BLOB(C).
{
    size_t size = strlen(B) + strlen(C) + 1;
    A = (char*)CFCParser_allocate(state, size);
    sprintf(A, "%s%s", B, C);
}

