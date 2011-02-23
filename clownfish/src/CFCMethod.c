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

#include <stdlib.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define CFC_NEED_FUNCTION_STRUCT_DEF
#include "CFCFunction.h"
#include "CFCMethod.h"
#include "CFCType.h"
#include "CFCUtil.h"
#include "CFCParamList.h"
#include "CFCParcel.h"
#include "CFCDocuComment.h"
#include "CFCVariable.h"

#ifndef true
    #define true 1
    #define false 0
#endif

struct CFCMethod {
    CFCFunction function;
    char *macro_sym;
    char *short_typedef;
    char *full_typedef;
    char *full_callback_sym;
    char *full_override_sym;
    int is_final;
    int is_abstract;
    int is_novel;
};

CFCMethod*
CFCMethod_new(CFCParcel *parcel, const char *exposure, const char *class_name,
              const char *class_cnick, const char *micro_sym, 
              CFCType *return_type, CFCParamList *param_list, 
              CFCDocuComment *docucomment, const char *macro_sym, int is_final, 
              int is_abstract)
{
    CFCMethod *self = (CFCMethod*)CFCBase_allocate(sizeof(CFCMethod),
        "Clownfish::Method");
    return CFCMethod_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, macro_sym,
        is_final, is_abstract);
}

CFCMethod*
CFCMethod_init(CFCMethod *self, CFCParcel *parcel, const char *exposure, 
               const char *class_name, const char *class_cnick, 
               const char *micro_sym, CFCType *return_type, 
               CFCParamList *param_list, CFCDocuComment *docucomment, 
               const char *macro_sym, int is_final, int is_abstract)
{
    CFCFunction_init((CFCFunction*)self, parcel, exposure, class_name,
        class_cnick, micro_sym, return_type, param_list, docucomment,
        false);
    self->macro_sym     = CFCUtil_strdup(macro_sym);
    self->short_typedef = NULL;
    self->full_typedef  = NULL;
    self->is_final      = is_final;
    self->is_abstract   = is_abstract;

    const char *full_func_sym = CFCFunction_full_func_sym((CFCFunction*)self);
    size_t amount = strlen(full_func_sym) + sizeof("_OVERRIDE") + 1;
    self->full_callback_sym = (char*)malloc(amount);
    self->full_override_sym = (char*)malloc(amount);
    if (!self->full_callback_sym || !self->full_override_sym) {
        croak("malloc failed");
    }
    int check = sprintf(self->full_callback_sym, "%s_CALLBACK", 
        full_func_sym);
    if (check < 0) { croak("sprintf failed"); }
    check = sprintf(self->full_override_sym, "%s_OVERRIDE", full_func_sym);
    if (check < 0) { croak("sprintf failed"); }

    // Assume that this method is novel until we discover when applying
    // inheritance that it was overridden.
    self->is_novel = 1;

    return self;
}

void
CFCMethod_destroy(CFCMethod *self)
{
    free(self->macro_sym);
    free(self->short_typedef);
    free(self->full_typedef);
    free(self->full_callback_sym);
    free(self->full_override_sym);
    CFCFunction_destroy((CFCFunction*)self);
}

const char*
CFCMethod_get_macro_sym(CFCMethod *self)
{
    return self->macro_sym;
}

void
CFCMethod_set_short_typedef(CFCMethod *self, const char *short_typedef)
{
    free(self->short_typedef);
    free(self->full_typedef);
    if (short_typedef) {
        self->short_typedef = CFCUtil_strdup(short_typedef);
        const char *prefix = CFCSymbol_get_prefix((CFCSymbol*)self);
        size_t amount = strlen(prefix) + strlen(short_typedef) + 1;
        self->full_typedef = (char*)malloc(amount);
        if (!self->full_typedef) { croak("malloc failed"); }
        int check = sprintf(self->full_typedef, "%s%s", prefix,
            short_typedef);
        if (check < 0) { croak("sprintf failed"); }
    }
    else {
        self->short_typedef = NULL;
        self->full_typedef = NULL;
    }
}

const char*
CFCMethod_short_typedef(CFCMethod *self)
{
    return self->short_typedef;
}

const char*
CFCMethod_full_typedef(CFCMethod *self)
{
    return self->full_typedef;
}

const char*
CFCMethod_full_callback_sym(CFCMethod *self)
{
    return self->full_callback_sym;
}

const char*
CFCMethod_full_override_sym(CFCMethod *self)
{
    return self->full_override_sym;
}

int
CFCMethod_final(CFCMethod *self)
{
    return self->is_final;
}

int
CFCMethod_abstract(CFCMethod *self)
{
    return self->is_abstract;
}

void
CFCMethod_set_novel(CFCMethod *self, int is_novel)
{
    self->is_novel = !!is_novel;
}

int
CFCMethod_novel(CFCMethod *self)
{
    return self->is_novel;
}

CFCType*
CFCMethod_self_type(CFCMethod *self)
{
    CFCVariable **vars = CFCParamList_get_variables(self->function.param_list);
    return CFCVariable_get_type(vars[0]);
}

