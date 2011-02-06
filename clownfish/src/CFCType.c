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

#ifndef true
  #define true 1
  #define false 0
#endif

#include "CFCType.h"

struct CFCType {
    int   flags;
    char *specifier;
    int   indirection;
    void *parcel;
    char *c_string;
};

CFCType*
CFCType_new(int flags, void *parcel, const char *specifier, int indirection,
            const char *c_string)
{
    CFCType *self = (CFCType*)malloc(sizeof(CFCType));
    if (!self) { croak("malloc failed"); }
    return CFCType_init(self, flags, parcel, specifier, indirection, 
        c_string);
}

CFCType*
CFCType_init(CFCType *self, int flags, void *parcel, const char *specifier,
             int indirection, const char *c_string)
{
    self->flags       = flags;
    self->parcel      = newSVsv((SV*)parcel);
    self->specifier   = savepv(specifier);
    self->indirection = indirection;
    self->c_string    = c_string ? savepv(c_string) : savepv("");
    return self;
}

void
CFCType_destroy(CFCType *self)
{
    Safefree(self->specifier);
    Safefree(self->c_string);
    SvREFCNT_dec(self->parcel);
    free(self);
}

int
CFCType_equals(CFCType *self, CFCType *other)
{
    if (   (CFCType_const(self)        ^ CFCType_const(other))
        || (CFCType_nullable(self)     ^ CFCType_nullable(other))
        || (CFCType_is_void(self)      ^ CFCType_is_void(other))
        || (CFCType_is_object(self)    ^ CFCType_is_object(other))
        || (CFCType_is_primitive(self) ^ CFCType_is_primitive(other))
        || (CFCType_is_integer(self)   ^ CFCType_is_integer(other))
        || (CFCType_is_floating(self)  ^ CFCType_is_floating(other))
        || (CFCType_is_va_list(self)   ^ CFCType_is_va_list(other))
        || (CFCType_is_arbitrary(self) ^ CFCType_is_arbitrary(other))
        || (CFCType_is_composite(self) ^ CFCType_is_composite(other))
    ) { 
        return false; 
    }
    if (self->indirection != other->indirection) { return false; }
    if (strcmp(self->specifier, other->specifier) != 0) { return false; }
    return true;
}

void
CFCType_set_specifier(CFCType *self, const char *specifier)
{
    Safefree(self->specifier);
    self->specifier = savepv(specifier);
}

const char*
CFCType_get_specifier(CFCType *self)
{
    return self->specifier;
}

int
CFCType_get_indirection(CFCType *self)
{
    return self->indirection;
}

void*
CFCType_get_parcel(CFCType *self)
{
    return self->parcel;
}

void
CFCType_set_c_string(CFCType *self, const char *c_string)
{
    Safefree(self->c_string);
    self->c_string = savepv(c_string);
}

const char*
CFCType_to_c(CFCType *self)
{
    return self->c_string;
}

int
CFCType_const(CFCType *self)
{
    return !!(self->flags & CFCTYPE_CONST);
}

int
CFCType_set_nullable(CFCType *self, int nullable)
{
    if (nullable) {
        self->flags |= CFCTYPE_NULLABLE;
    }
    else {
        self->flags &= ~CFCTYPE_NULLABLE;
    }
}

int
CFCType_nullable(CFCType *self)
{
    return !!(self->flags & CFCTYPE_NULLABLE);
}

int
CFCType_is_void(CFCType *self)
{
    return !!(self->flags & CFCTYPE_VOID);
}

int
CFCType_is_object(CFCType *self)
{
    return !!(self->flags & CFCTYPE_OBJECT);
}

int
CFCType_is_primitive(CFCType *self)
{
    return !!(self->flags & CFCTYPE_PRIMITIVE);
}

int
CFCType_is_integer(CFCType *self)
{
    return !!(self->flags & CFCTYPE_INTEGER);
}

int
CFCType_is_floating(CFCType *self)
{
    return !!(self->flags & CFCTYPE_FLOATING);
}

int
CFCType_is_string_type(CFCType *self)
{
    return !!(self->flags & CFCTYPE_STRING_TYPE);
}

int
CFCType_is_va_list(CFCType *self)
{
    return !!(self->flags & CFCTYPE_VA_LIST);
}

int
CFCType_is_arbitrary(CFCType *self)
{
    return !!(self->flags & CFCTYPE_ARBITRARY);
}

int
CFCType_is_composite(CFCType *self)
{
    return !!(self->flags & CFCTYPE_COMPOSITE);
}

