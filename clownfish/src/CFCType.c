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
#include <ctype.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef true
  #define true 1
  #define false 0
#endif

#include "CFCType.h"
#include "CFCParcel.h"

struct CFCType {
    int   flags;
    char *specifier;
    int   indirection;
    struct CFCParcel *parcel;
    char *c_string;
    size_t width;
};

CFCType*
CFCType_new(int flags, struct CFCParcel *parcel, const char *specifier,
            int indirection, const char *c_string)
{
    CFCType *self = (CFCType*)malloc(sizeof(CFCType));
    if (!self) { croak("malloc failed"); }
    return CFCType_init(self, flags, parcel, specifier, indirection, 
        c_string);
}

CFCType*
CFCType_init(CFCType *self, int flags, struct CFCParcel *parcel, 
             const char *specifier, int indirection, const char *c_string)
{
    self->flags       = flags;
    self->parcel      = parcel;
    self->specifier   = savepv(specifier);
    self->indirection = indirection;
    self->c_string    = c_string ? savepv(c_string) : savepv("");
    self->width       = 0;
    return self;
}

CFCType*
CFCType_new_integer(int flags, const char *specifier)
{
    // Validate specifier, find width.
    size_t width; 
    if (!strcmp(specifier, "int8_t") || !strcmp(specifier, "uint8_t")) {
        width = 1;
    }
    else if (!strcmp(specifier, "int16_t") || !strcmp(specifier, "uint16_t")) {
        width = 2;
    }
    else if (!strcmp(specifier, "int32_t") || !strcmp(specifier, "uint32_t")) {
        width = 4;
    }
    else if (!strcmp(specifier, "int64_t") || !strcmp(specifier, "uint64_t")) {
        width = 8;
    }
    else if (   !strcmp(specifier, "char") 
             || !strcmp(specifier, "short")
             || !strcmp(specifier, "int")
             || !strcmp(specifier, "long")
             || !strcmp(specifier, "size_t")
             || !strcmp(specifier, "bool_t") // Charmonizer type.
    ) {
        width = 0;
    }
    else {
        croak("Unknown integer specifier: '%s'", specifier);
    }

    // Add Charmonizer prefix if necessary.
    char full_specifier[32];
    if (strcmp(specifier, "bool_t") == 0) {
        strcpy(full_specifier, "chy_bool_t");
    }
    else {
        strcpy(full_specifier, specifier);
    }

    // Cache the C representation of this type.
    char c_string[32];
    if (flags & CFCTYPE_CONST) {
        sprintf(c_string, "const %s", full_specifier);
    }
    else {
        strcpy(c_string, full_specifier);
    }

    // Add flags.
    flags |= CFCTYPE_PRIMITIVE;
    flags |= CFCTYPE_INTEGER;

    CFCType *self = CFCType_new(flags, NULL, full_specifier, 0, c_string);
    self->width = width;
    return self;
}

static const char *float_specifiers[] = { 
    "float", 
    "double", 
    NULL 
};

CFCType*
CFCType_new_float(int flags, const char *specifier)
{
    // Validate specifier.
    size_t i;
    for (i = 0; ; i++) {
        if (!float_specifiers[i]) {
            croak("Unknown float specifier: '%s'", specifier);
        }
        if (strcmp(float_specifiers[i], specifier) == 0) {
            break;
        }
    }

    // Cache the C representation of this type.
    char c_string[32];
    if (flags & CFCTYPE_CONST) {
        sprintf(c_string, "const %s", specifier);
    }
    else {
        strcpy(c_string, specifier);
    }

    flags |= CFCTYPE_PRIMITIVE;
    flags |= CFCTYPE_FLOATING;

    return CFCType_new(flags, NULL, specifier, 0, c_string);
}

CFCType*
CFCType_new_void(int is_const)
{
    int flags = CFCTYPE_VOID;
    const char *c_string = is_const ? "const void" : "void";
    if (is_const) { flags |= CFCTYPE_CONST; }
    return CFCType_new(flags, NULL, "void", 0, c_string);
}

CFCType*
CFCType_new_va_list(void)
{
    return CFCType_new(CFCTYPE_VA_LIST, NULL, "va_list", 0, "va_list");
}


CFCType*
CFCType_new_arbitrary(CFCParcel *parcel, const char *specifier)
{
    const size_t MAX_SPECIFIER_LEN = 256;

    // Add parcel prefix to what appear to be namespaced types.
    char full_specifier[MAX_SPECIFIER_LEN + 1];
    if (isupper(*specifier) && parcel != NULL) {
        const char *prefix = CFCParcel_get_prefix(parcel);
        size_t full_len = strlen(prefix) + strlen(specifier);
        if (full_len > MAX_SPECIFIER_LEN) {
            croak("Illegal specifier: '%s'", specifier); 
        }
        sprintf(full_specifier, "%s%s", prefix, specifier);
    }
    else {
        if (strlen(specifier) > MAX_SPECIFIER_LEN) {
            croak("Illegal specifier: '%s'", specifier); 
        }
        strcpy(full_specifier, specifier);
    }

    // Validate specifier.
    size_t i, max;
    for (i = 0, max = strlen(full_specifier); i < max; i++) {
        if (!isalnum(full_specifier[i]) && full_specifier[i] != '_') {
            croak("Illegal specifier: '%s'", full_specifier); 
        }
    }

    return CFCType_new(CFCTYPE_ARBITRARY, parcel, full_specifier, 0, 
        full_specifier);
}

void
CFCType_destroy(CFCType *self)
{
    Safefree(self->specifier);
    Safefree(self->c_string);
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

struct CFCParcel*
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

size_t
CFCType_get_width(CFCType *self)
{
    return self->width;
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

