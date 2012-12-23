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

/** Clownfish::CFC::Model::Function - Metadata describing a function.
 */

#ifndef H_CFCFUNCTION
#define H_CFCFUNCTION

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCFunction CFCFunction;
struct CFCParcel;
struct CFCType;
struct CFCDocuComment;
struct CFCParamList;

#ifdef CFC_NEED_FUNCTION_STRUCT_DEF
#define CFC_NEED_SYMBOL_STRUCT_DEF
#include "CFCSymbol.h"
struct CFCFunction {
    CFCSymbol symbol;
    struct CFCType *return_type;
    struct CFCParamList *param_list;
    struct CFCDocuComment *docucomment;
    int is_inline;
};
#endif

/**
 * @param parcel A Clownfish::CFC::Model::Parcel.
 * @param exposure The function's exposure (see
 * L<Clownfish::CFC::Model::Symbol>).
 * @param class_name The full name of the class in whose namespace the
 * function resides.
 * @param class_cnick The C nickname for the class.
 * @param micro_sym The lower case name of the function, without any
 * namespacing prefixes.
 * @param return_type A Clownfish::CFC::Model::Type representing the
 * function's return type.
 * @param param_list A Clownfish::CFC::Model::ParamList representing the
 * function's argument list.
 * @param docucomment A Clownfish::CFC::Model::DocuComment describing the
 * function.
 * @param is_inline Should be true if the function should be inlined by the
 * compiler.
 */
CFCFunction*
CFCFunction_new(struct CFCParcel *parcel, const char *exposure,
                const char *class_name, const char *class_cnick,
                const char *micro_sym, struct CFCType *return_type,
                struct CFCParamList *param_list,
                struct CFCDocuComment *docucomment, int is_inline);

CFCFunction*
CFCFunction_init(CFCFunction *self, struct CFCParcel *parcel,
                 const char *exposure, const char *class_name,
                 const char *class_cnick, const char *micro_sym,
                 struct CFCType *return_type, struct CFCParamList *param_list,
                 struct CFCDocuComment *docucomment, int is_inline);

void
CFCFunction_destroy(CFCFunction *self);

struct CFCType*
CFCFunction_get_return_type(CFCFunction *self);

struct CFCParamList*
CFCFunction_get_param_list(CFCFunction *self);

struct CFCDocuComment*
CFCFunction_get_docucomment(CFCFunction *self);

int
CFCFunction_inline(CFCFunction *self);

/** Returns true if the function has a void return type, false otherwise.
 */
int
CFCFunction_void(CFCFunction *self);

/** A synonym for full_sym().
 */
const char*
CFCFunction_full_func_sym(CFCFunction *self);

/** A synonym for short_sym().
 */
const char*
CFCFunction_short_func_sym(CFCFunction *self);

const char*
CFCFunction_micro_sym(CFCFunction *self);

int
CFCFunction_public(CFCFunction *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCFUNCTION */

