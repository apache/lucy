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
#include <stdio.h>
#include <ctype.h>

#ifndef true
  #define true 1
  #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCType.h"
#include "CFCParcel.h"
#include "CFCSymbol.h"
#include "CFCUtil.h"

struct CFCType {
    CFCBase  base;
    int      flags;
    char    *specifier;
    char    *vtable_var;
    int      indirection;
    struct CFCParcel *parcel;
    char    *c_string;
    size_t   width;
    char    *array;
    struct CFCType *child;
};

const static CFCMeta CFCTYPE_META = {
    "Clownfish::CFC::Model::Type",
    sizeof(CFCType),
    (CFCBase_destroy_t)CFCType_destroy
};

CFCType*
CFCType_new(int flags, struct CFCParcel *parcel, const char *specifier,
            int indirection, const char *c_string) {
    CFCType *self = (CFCType*)CFCBase_allocate(&CFCTYPE_META);
    return CFCType_init(self, flags, parcel, specifier, indirection,
                        c_string);
}

static void
S_check_flags(int supplied, int acceptable, const char *type_name) {
    int bad = (supplied & ~acceptable);
    if (bad) {
        char bad_flag[20];
        if ((bad & CFCTYPE_CONST))            { strcpy(bad_flag, "CONST"); }
        else if ((bad & CFCTYPE_NULLABLE))    { strcpy(bad_flag, "NULLABLE"); }
        else if ((bad & CFCTYPE_INCREMENTED)) { strcpy(bad_flag, "INCREMENTED"); }
        else if ((bad & CFCTYPE_DECREMENTED)) { strcpy(bad_flag, "DECREMENTED"); }
        else if ((bad & CFCTYPE_OBJECT))      { strcpy(bad_flag, "OBJECT"); }
        else if ((bad & CFCTYPE_PRIMITIVE))   { strcpy(bad_flag, "PRIMITIVE"); }
        else if ((bad & CFCTYPE_INTEGER))     { strcpy(bad_flag, "INTEGER"); }
        else if ((bad & CFCTYPE_FLOATING))    { strcpy(bad_flag, "FLOATING"); }
        else if ((bad & CFCTYPE_STRING_TYPE)) { strcpy(bad_flag, "STRING_TYPE"); }
        else if ((bad & CFCTYPE_VA_LIST))     { strcpy(bad_flag, "VA_LIST"); }
        else if ((bad & CFCTYPE_ARBITRARY))   { strcpy(bad_flag, "ARBITRARY"); }
        else if ((bad & CFCTYPE_COMPOSITE))   { strcpy(bad_flag, "COMPOSITE"); }
        else {
            CFCUtil_die("Unknown flags: %d", bad);
        }
        CFCUtil_die("Bad flag for type %s: %s", type_name, bad_flag);
    }
}

CFCType*
CFCType_init(CFCType *self, int flags, struct CFCParcel *parcel,
             const char *specifier, int indirection, const char *c_string) {
    self->flags       = flags;
    self->parcel      = (CFCParcel*)CFCBase_incref((CFCBase*)parcel);
    self->specifier   = CFCUtil_strdup(specifier);
    self->indirection = indirection;
    self->c_string    = c_string ? CFCUtil_strdup(c_string) : CFCUtil_strdup("");
    self->width       = 0;
    self->array       = NULL;
    self->child       = NULL;
    if (flags & CFCTYPE_OBJECT) {
        self->vtable_var = CFCUtil_strdup(specifier);
        for (int i = 0; self->vtable_var[i] != 0; i++) {
            self->vtable_var[i] = toupper(self->vtable_var[i]);
        }
    }
    else {
        self->vtable_var  = NULL;
    }
    return self;
}

CFCType*
CFCType_new_integer(int flags, const char *specifier) {
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
    else if (!strcmp(specifier, "char")
             || !strcmp(specifier, "short")
             || !strcmp(specifier, "int")
             || !strcmp(specifier, "long")
             || !strcmp(specifier, "size_t")
             || !strcmp(specifier, "bool_t") // Charmonizer type.
            ) {
        width = 0;
    }
    else {
        CFCUtil_die("Unknown integer specifier: '%s'", specifier);
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
    S_check_flags(flags, CFCTYPE_CONST | CFCTYPE_PRIMITIVE | CFCTYPE_INTEGER,
                  "Integer");

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
CFCType_new_float(int flags, const char *specifier) {
    // Validate specifier.
    for (size_t i = 0; ; i++) {
        if (!float_specifiers[i]) {
            CFCUtil_die("Unknown float specifier: '%s'", specifier);
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
    S_check_flags(flags, CFCTYPE_CONST | CFCTYPE_PRIMITIVE | CFCTYPE_FLOATING,
                  "Floating");

    return CFCType_new(flags, NULL, specifier, 0, c_string);
}

CFCType*
CFCType_new_object(int flags, CFCParcel *parcel, const char *specifier,
                   int indirection) {
    // Validate params.
    if (indirection != 1) {
        CFCUtil_die("Parameter 'indirection' can only be 1");
    }
    if (!specifier || !strlen(specifier)) {
        CFCUtil_die("Missing required param 'specifier'");
    }
    if ((flags & CFCTYPE_INCREMENTED) && (flags & CFCTYPE_DECREMENTED)) {
        CFCUtil_die("Can't be both incremented and decremented");
    }

    // Use default parcel if none supplied.
    if (!parcel) {
        parcel = CFCParcel_default_parcel();
    }

    // Add flags.
    flags |= CFCTYPE_OBJECT;
    if (strstr(specifier, "CharBuf")) {
        // Determine whether this type is a string type.
        flags |= CFCTYPE_STRING_TYPE;
    }

    const size_t MAX_SPECIFIER_LEN = 256;
    char full_specifier[MAX_SPECIFIER_LEN + 1];
    char small_specifier[MAX_SPECIFIER_LEN + 1];
    if (isupper(*specifier)) {
        const char *prefix = CFCParcel_get_prefix(parcel);
        if (strlen(prefix) + strlen(specifier) > MAX_SPECIFIER_LEN) {
            CFCUtil_die("Specifier and/or parcel prefix too long");
        }
        sprintf(full_specifier, "%s%s", prefix, specifier);
        strcpy(small_specifier, specifier);
    }
    else if (!isalpha(*specifier)) {
        CFCUtil_die("Invalid specifier: '%s'", specifier);
    }
    else {
        if (strlen(specifier) > MAX_SPECIFIER_LEN) {
            CFCUtil_die("Specifier too long");
        }
        const char *probe = specifier;
        while (*probe) {
            if (isupper(*probe)) {
                break;
            }
            else if (!isalnum(*probe) && *probe != '_') {
                CFCUtil_die("Invalid specifier: '%s'", specifier);
            }
            probe++;
        }
        strcpy(full_specifier, specifier);
        strcpy(small_specifier, probe);
    }

    if (!CFCSymbol_validate_class_name_component(small_specifier)) {
        CFCUtil_die("Invalid specifier: '%s'", specifier);
    }

    // Cache C representation.
    char c_string[MAX_SPECIFIER_LEN + 10];
    if (flags & CFCTYPE_CONST) {
        sprintf(c_string, "const %s*", full_specifier);
    }
    else {
        sprintf(c_string, "%s*", full_specifier);
    }

    int acceptable_flags = CFCTYPE_OBJECT
                           | CFCTYPE_STRING_TYPE
                           | CFCTYPE_CONST
                           | CFCTYPE_NULLABLE
                           | CFCTYPE_INCREMENTED
                           | CFCTYPE_DECREMENTED;
    S_check_flags(flags, acceptable_flags, "Object");

    return CFCType_new(flags, parcel, full_specifier, 1, c_string);
}

CFCType*
CFCType_new_composite(int flags, CFCType *child, int indirection,
                      const char *array) {
    if (!child) {
        CFCUtil_die("Missing required param 'child'");
    }
    flags |= CFCTYPE_COMPOSITE;
    S_check_flags(flags, CFCTYPE_COMPOSITE | CFCTYPE_NULLABLE, "Composite");

    // Cache C representation.
    // NOTE: Array postfixes are NOT included.
    const size_t  MAX_LEN        = 256;
    const char   *child_c_string = CFCType_to_c(child);
    size_t        child_c_len    = strlen(child_c_string);
    size_t        amount         = child_c_len + indirection;
    if (amount > MAX_LEN) {
        CFCUtil_die("C representation too long");
    }
    char c_string[MAX_LEN + 1];
    strcpy(c_string, child_c_string);
    for (int i = 0; i < indirection; i++) {
        strncat(c_string, "*", 1);
    }

    CFCType *self = CFCType_new(flags, NULL, CFCType_get_specifier(child),
                                indirection, c_string);
    self->child = (CFCType*)CFCBase_incref((CFCBase*)child);

    // Record array spec.
    const char *array_spec = array ? array : "";
    size_t array_spec_size = strlen(array_spec) + 1;
    self->array = (char*)MALLOCATE(array_spec_size);
    strcpy(self->array, array_spec);

    return self;
}

CFCType*
CFCType_new_void(int is_const) {
    int flags = CFCTYPE_VOID;
    const char *c_string = is_const ? "const void" : "void";
    if (is_const) { flags |= CFCTYPE_CONST; }
    return CFCType_new(flags, NULL, "void", 0, c_string);
}

CFCType*
CFCType_new_va_list(void) {
    return CFCType_new(CFCTYPE_VA_LIST, NULL, "va_list", 0, "va_list");
}

CFCType*
CFCType_new_arbitrary(CFCParcel *parcel, const char *specifier) {
    // Validate specifier.
    for (size_t i = 0, max = strlen(specifier); i < max; i++) {
        if (!isalnum(specifier[i]) && specifier[i] != '_') {
            CFCUtil_die("Illegal specifier: '%s'", specifier);
        }
    }

    return CFCType_new(CFCTYPE_ARBITRARY, parcel, specifier, 0, specifier);
}

void
CFCType_destroy(CFCType *self) {
    if (self->child) {
        CFCBase_decref((CFCBase*)self->child);
    }
    CFCBase_decref((CFCBase*)self->parcel);
    FREEMEM(self->specifier);
    FREEMEM(self->c_string);
    FREEMEM(self->array);
    FREEMEM(self->vtable_var);
    CFCBase_destroy((CFCBase*)self);
}

int
CFCType_equals(CFCType *self, CFCType *other) {
    if ((CFCType_const(self)           ^ CFCType_const(other))
        || (CFCType_nullable(self)     ^ CFCType_nullable(other))
        || (CFCType_is_void(self)      ^ CFCType_is_void(other))
        || (CFCType_is_object(self)    ^ CFCType_is_object(other))
        || (CFCType_is_primitive(self) ^ CFCType_is_primitive(other))
        || (CFCType_is_integer(self)   ^ CFCType_is_integer(other))
        || (CFCType_is_floating(self)  ^ CFCType_is_floating(other))
        || (CFCType_is_va_list(self)   ^ CFCType_is_va_list(other))
        || (CFCType_is_arbitrary(self) ^ CFCType_is_arbitrary(other))
        || (CFCType_is_composite(self) ^ CFCType_is_composite(other))
        || (CFCType_incremented(self)  ^ CFCType_incremented(other))
        || (CFCType_decremented(self)  ^ CFCType_decremented(other))
        || !!self->child ^ !!other->child
        || !!self->array ^ !!other->array
       ) {
        return false;
    }
    if (self->indirection != other->indirection) { return false; }
    if (strcmp(self->specifier, other->specifier) != 0) { return false; }
    if (self->child) {
        if (!CFCType_equals(self->child, other->child)) { return false; }
    }
    if (self->array) {
        if (strcmp(self->array, other->array) != 0) { return false; }
    }
    return true;
}

int
CFCType_similar(CFCType *self, CFCType *other) {
    if (!CFCType_is_object(self)) {
        CFCUtil_die("Attempt to call 'similar' on a non-object type");
    }
    if ((CFCType_const(self)           ^ CFCType_const(other))
        || (CFCType_nullable(self)     ^ CFCType_nullable(other))
        || (CFCType_incremented(self)  ^ CFCType_incremented(other))
        || (CFCType_decremented(self)  ^ CFCType_decremented(other))
        || (CFCType_is_object(self)    ^ CFCType_is_object(other))
       ) {
        return false;
    }
    return true;
}

void
CFCType_set_specifier(CFCType *self, const char *specifier) {
    FREEMEM(self->specifier);
    self->specifier = CFCUtil_strdup(specifier);
}

const char*
CFCType_get_specifier(CFCType *self) {
    return self->specifier;
}

const char*
CFCType_get_vtable_var(CFCType *self) {
    return self->vtable_var;
}

int
CFCType_get_indirection(CFCType *self) {
    return self->indirection;
}

struct CFCParcel*
CFCType_get_parcel(CFCType *self) {
    return self->parcel;
}

void
CFCType_set_c_string(CFCType *self, const char *c_string) {
    FREEMEM(self->c_string);
    self->c_string = CFCUtil_strdup(c_string);
}

const char*
CFCType_to_c(CFCType *self) {
    return self->c_string;
}

size_t
CFCType_get_width(CFCType *self) {
    return self->width;
}

const char*
CFCType_get_array(CFCType *self) {
    return self->array;
}

int
CFCType_const(CFCType *self) {
    return !!(self->flags & CFCTYPE_CONST);
}

void
CFCType_set_nullable(CFCType *self, int nullable) {
    if (nullable) {
        self->flags |= CFCTYPE_NULLABLE;
    }
    else {
        self->flags &= ~CFCTYPE_NULLABLE;
    }
}

int
CFCType_nullable(CFCType *self) {
    return !!(self->flags & CFCTYPE_NULLABLE);
}

int
CFCType_incremented(CFCType *self) {
    return !!(self->flags & CFCTYPE_INCREMENTED);
}

int
CFCType_decremented(CFCType *self) {
    return !!(self->flags & CFCTYPE_DECREMENTED);
}

int
CFCType_is_void(CFCType *self) {
    return !!(self->flags & CFCTYPE_VOID);
}

int
CFCType_is_object(CFCType *self) {
    return !!(self->flags & CFCTYPE_OBJECT);
}

int
CFCType_is_primitive(CFCType *self) {
    return !!(self->flags & CFCTYPE_PRIMITIVE);
}

int
CFCType_is_integer(CFCType *self) {
    return !!(self->flags & CFCTYPE_INTEGER);
}

int
CFCType_is_floating(CFCType *self) {
    return !!(self->flags & CFCTYPE_FLOATING);
}

int
CFCType_is_string_type(CFCType *self) {
    return !!(self->flags & CFCTYPE_STRING_TYPE);
}

int
CFCType_is_va_list(CFCType *self) {
    return !!(self->flags & CFCTYPE_VA_LIST);
}

int
CFCType_is_arbitrary(CFCType *self) {
    return !!(self->flags & CFCTYPE_ARBITRARY);
}

int
CFCType_is_composite(CFCType *self) {
    return !!(self->flags & CFCTYPE_COMPOSITE);
}

