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
#include "CFCUtil.h"
#include "CFCParcel.h"
#include "CFCDocuComment.h"

struct CFCMethod {
    CFCFunction function;
    char *macro_sym;
    char *short_typedef;
    int is_final;
    int is_abstract;
    int is_novel;
};

CFCMethod*
CFCMethod_new(CFCParcel *parcel, const char *exposure, const char *class_name,
              const char *class_cnick, const char *micro_sym, 
              void *return_type, void *param_list, 
              CFCDocuComment *docucomment, int is_inline, 
              const char *macro_sym, int is_final, int is_abstract)
{
    CFCMethod *self = (CFCMethod*)CFCBase_allocate(sizeof(CFCMethod),
        "Clownfish::Method");
    return CFCMethod_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, is_inline, macro_sym,
        is_final, is_abstract);
}

CFCMethod*
CFCMethod_init(CFCMethod *self, CFCParcel *parcel, const char *exposure, 
               const char *class_name, const char *class_cnick, 
               const char *micro_sym, void *return_type, void *param_list, 
               CFCDocuComment *docucomment, int is_inline,
               const char *macro_sym, int is_final, int is_abstract)
{
    CFCFunction_init((CFCFunction*)self, parcel, exposure, class_name,
        class_cnick, micro_sym, return_type, param_list, docucomment,
        is_inline);
    self->macro_sym     = CFCUtil_strdup(macro_sym);
    self->short_typedef = NULL;
    self->is_final      = is_final;
    self->is_abstract   = is_abstract;

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
    self->short_typedef = short_typedef ? CFCUtil_strdup(short_typedef) : NULL;
}

const char*
CFCMethod_short_typedef(CFCMethod *self)
{
    return self->short_typedef;
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

