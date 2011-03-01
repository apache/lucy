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
        int   is_inline;
    };
#endif


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

int
CFCFunction_void(CFCFunction *self);

const char*
CFCFunction_full_func_sym(CFCFunction *self);

const char*
CFCFunction_short_func_sym(CFCFunction *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCFUNCTION */

