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
#include <string.h>
#include <ctype.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef true
  #define true 1
  #define false 0
#endif

#define CFC_NEED_SYMBOL_STRUCT_DEF
#include "CFCSymbol.h"
#include "CFCVariable.h"
#include "CFCParcel.h"
#include "CFCType.h"
#include "CFCUtil.h"

struct CFCVariable {
    struct CFCSymbol symbol;
    CFCType *type;
    char *local_c;
    char *global_c;
    char *local_dec;
    void *perl_obj;
};

CFCVariable*
CFCVariable_new(struct CFCParcel *parcel, const char *exposure, 
                const char *class_name, const char *class_cnick, 
                const char *micro_sym, struct CFCType *type)
{
    CFCVariable *self = (CFCVariable*)calloc(sizeof(CFCVariable), 1);
    if (!self) { croak("malloc failed"); }
    return CFCVariable_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym, type);
}

CFCVariable*
CFCVariable_init(CFCVariable *self, struct CFCParcel *parcel, 
                 const char *exposure, const char *class_name, 
                 const char *class_cnick, const char *micro_sym, 
                 struct CFCType *type)
{
    // Default exposure to "local".
    const char *real_exposure = exposure ? exposure : "local";

    CFCSymbol_init((CFCSymbol*)self, parcel, real_exposure, class_name,
        class_cnick, micro_sym);

    // Cache perl object SV.
    self->perl_obj = CFCUtil_make_perl_obj(self, "Clownfish::Variable");

    // Assign type.
    self->type = type;
    SV *type_sv = CFCType_get_perl_obj(type);
    SvREFCNT_inc(type_sv);

    // Cache various C string representations.
    const char *type_str = CFCType_to_c(type);
    const char *postfix  = "";
    if (CFCType_is_composite(type) && CFCType_get_array(type) != NULL) {
        postfix = CFCType_get_array(type);
    }
    {
        size_t size = strlen(type_str) + sizeof(" ") + strlen(micro_sym) +
            strlen(postfix) + 1;
        self->local_c = (char*)malloc(size);
        if (!self->local_c) { croak("malloc failed"); }
        sprintf(self->local_c, "%s %s%s", type_str, micro_sym, postfix);
    }
    {
        self->local_dec = (char*)malloc(strlen(self->local_c) + sizeof(";\0"));
        if (!self->local_dec) { croak("malloc failed"); }
        sprintf(self->local_dec, "%s;", self->local_c);
    }
    {
        const char *full_sym = CFCSymbol_full_sym((CFCSymbol*)self);
        size_t size = strlen(type_str) + sizeof(" ") + strlen(full_sym) +
            strlen(postfix) + 1;
        self->global_c = (char*)malloc(size);
        if (!self->global_c) { croak("malloc failed"); }
        sprintf(self->global_c, "%s %s%s", type_str, full_sym, postfix);
    }

    return self;
}

void
CFCVariable_destroy(CFCVariable *self)
{
    if (self->perl_obj) {
        int refcount = SvREFCNT((SV*)self->perl_obj);
        if (refcount > 0) {
            if (refcount == 1) {
                // Trigger Perl destructor, which causes recursion.
                SV *perl_obj = (SV*)self->perl_obj;
                self->perl_obj = NULL;
                SvREFCNT_dec((SV*)self->perl_obj);
                return;
            }
            else {
                SvREFCNT_dec((SV*)self->perl_obj);
                return;
            }
        }
    }
    SV *type_sv = CFCType_get_perl_obj(self->type);
    SvREFCNT_dec(type_sv);
    free(self->local_c);
    free(self->global_c);
    free(self->local_dec);
    CFCSymbol_destroy((CFCSymbol*)self);
}

int
CFCVariable_equals(CFCVariable *self, CFCVariable *other)
{
    if (!CFCType_equals(self->type, other->type)) { return false; }
    return CFCSymbol_equals((CFCSymbol*)self, (CFCSymbol*)other);
}

CFCType*
CFCVariable_get_type(CFCVariable *self)
{
    return self->type;
}

const char*
CFCVariable_local_c(CFCVariable *self)
{
    return self->local_c;
}

const char*
CFCVariable_global_c(CFCVariable *self)
{
    return self->global_c;
}

const char*
CFCVariable_local_declaration(CFCVariable *self)
{
    return self->local_dec;
}

void*
CFCVariable_get_perl_obj(CFCVariable *self)
{
    return self->perl_obj;
}

