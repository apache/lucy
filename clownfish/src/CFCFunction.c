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
#include "CFCParcel.h"
#include "CFCType.h"
#include "CFCParamList.h"
#include "CFCDocuComment.h"
#include "CFCUtil.h"

CFCFunction*
CFCFunction_new(CFCParcel *parcel, const char *exposure,
                const char *class_name, const char *class_cnick,
                const char *micro_sym, CFCType *return_type, 
                CFCParamList *param_list, CFCDocuComment *docucomment, 
                int is_inline)
{
    CFCFunction *self = (CFCFunction*)CFCBase_allocate(sizeof(CFCFunction),
        "Clownfish::Function");
    return CFCFunction_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, is_inline);
}

CFCFunction*
CFCFunction_init(CFCFunction *self, CFCParcel *parcel, const char *exposure,
               const char *class_name, const char *class_cnick, 
               const char *micro_sym, CFCType *return_type, 
               CFCParamList *param_list, CFCDocuComment *docucomment, 
               int is_inline)
{
    CFCSymbol_init((CFCSymbol*)self, parcel, exposure, class_name,
        class_cnick, micro_sym);
    CFCUTIL_NULL_CHECK(return_type);
    CFCUTIL_NULL_CHECK(param_list);
    self->return_type = (CFCType*)CFCBase_incref((CFCBase*)return_type);
    self->param_list  = (CFCParamList*)CFCBase_incref((CFCBase*)param_list);
    self->docucomment = (CFCDocuComment*)CFCBase_incref((CFCBase*)docucomment);
    self->is_inline   = is_inline;
    return self;
}

void
CFCFunction_destroy(CFCFunction *self)
{
    CFCBase_decref((CFCBase*)self->return_type);
    CFCBase_decref((CFCBase*)self->param_list);
    CFCBase_decref((CFCBase*)self->docucomment);
    CFCSymbol_destroy((CFCSymbol*)self);
}

CFCType*
CFCFunction_get_return_type(CFCFunction *self)
{
    return self->return_type;
}

CFCParamList*
CFCFunction_get_param_list(CFCFunction *self)
{
    return self->param_list;
}

CFCDocuComment*
CFCFunction_get_docucomment(CFCFunction *self)
{
    return self->docucomment;
}

int
CFCFunction_inline(CFCFunction *self)
{
    return self->is_inline;
}

