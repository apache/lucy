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

#include <string.h>
#include <ctype.h>

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_FUNCTION_STRUCT_DEF
#include "CFCFunction.h"
#include "CFCParcel.h"
#include "CFCType.h"
#include "CFCParamList.h"
#include "CFCDocuComment.h"
#include "CFCUtil.h"

const static CFCMeta CFCFUNCTION_META = {
    "Clownfish::Function",
    sizeof(CFCFunction),
    (CFCBase_destroy_t)CFCFunction_destroy
};

CFCFunction*
CFCFunction_new(CFCParcel *parcel, const char *exposure,
                const char *class_name, const char *class_cnick,
                const char *micro_sym, CFCType *return_type,
                CFCParamList *param_list, CFCDocuComment *docucomment,
                int is_inline) {
    CFCFunction *self = (CFCFunction*)CFCBase_allocate(&CFCFUNCTION_META);
    return CFCFunction_init(self, parcel, exposure, class_name, class_cnick,
                            micro_sym, return_type, param_list, docucomment,
                            is_inline);
}

static int
S_validate_micro_sym(const char *micro_sym) {
    size_t i;
    size_t len = strlen(micro_sym);
    if (!len) { return false; }
    for (i = 0; i < len; i++) {
        char c = micro_sym[i];
        if (!islower(c) && !isdigit(c) && c != '_') { return false; }
    }
    return true;
}

CFCFunction*
CFCFunction_init(CFCFunction *self, CFCParcel *parcel, const char *exposure,
                 const char *class_name, const char *class_cnick,
                 const char *micro_sym, CFCType *return_type,
                 CFCParamList *param_list, CFCDocuComment *docucomment,
                 int is_inline) {

    exposure = exposure ? exposure : "parcel";
    CFCSymbol_init((CFCSymbol*)self, parcel, exposure, class_name,
                   class_cnick, micro_sym);
    CFCUTIL_NULL_CHECK(class_name);
    CFCUTIL_NULL_CHECK(return_type);
    CFCUTIL_NULL_CHECK(param_list);
    if (!S_validate_micro_sym(micro_sym)) {
        CFCUtil_die("Invalid micro_sym: '%s'", micro_sym);
    }
    self->return_type = (CFCType*)CFCBase_incref((CFCBase*)return_type);
    self->param_list  = (CFCParamList*)CFCBase_incref((CFCBase*)param_list);
    self->docucomment = (CFCDocuComment*)CFCBase_incref((CFCBase*)docucomment);
    self->is_inline   = is_inline;
    return self;
}

void
CFCFunction_destroy(CFCFunction *self) {
    CFCBase_decref((CFCBase*)self->return_type);
    CFCBase_decref((CFCBase*)self->param_list);
    CFCBase_decref((CFCBase*)self->docucomment);
    CFCSymbol_destroy((CFCSymbol*)self);
}

CFCType*
CFCFunction_get_return_type(CFCFunction *self) {
    return self->return_type;
}

CFCParamList*
CFCFunction_get_param_list(CFCFunction *self) {
    return self->param_list;
}

CFCDocuComment*
CFCFunction_get_docucomment(CFCFunction *self) {
    return self->docucomment;
}

int
CFCFunction_inline(CFCFunction *self) {
    return self->is_inline;
}

int
CFCFunction_void(CFCFunction *self) {
    return CFCType_is_void(self->return_type);
}

const char*
CFCFunction_full_func_sym(CFCFunction *self) {
    return CFCSymbol_full_sym((CFCSymbol*)self);
}

const char*
CFCFunction_short_func_sym(CFCFunction *self) {
    return CFCSymbol_short_sym((CFCSymbol*)self);
}

const char*
CFCFunction_micro_sym(CFCFunction *self) {
    return CFCSymbol_micro_sym((CFCSymbol*)self);
}

