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
#include "CFCParcel.h"
#include "CFCUtil.h"

CFCSymbol*
CFCSymbol_new(struct CFCParcel *parcel, const char *exposure, 
              const char *class_name, const char *class_cnick, 
              const char *micro_sym)
{
    CFCSymbol *self = (CFCSymbol*)CFCBase_allocate(sizeof(CFCSymbol),
        "Clownfish::Symbol");
    return CFCSymbol_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym);
}

static int
S_validate_exposure(const char *exposure)
{
    if (!exposure) { return false; }
    if (   strcmp(exposure, "public")
        && strcmp(exposure, "parcel")
        && strcmp(exposure, "private")
        && strcmp(exposure, "local")
    ) {
        return false;
    }
    return true;
}

static int
S_validate_class_name(const char *class_name)
{
    const char *ptr;

    // Must be UpperCamelCase, separated by "::".
    for (ptr = class_name; *ptr != 0; ) {
        if (!isupper(*ptr)) { return false; }

        // Each component must contain lowercase letters.
        const char *substring;
        for (substring = ptr; ; substring++) {
            if      (*substring == 0)     { return false; }
            else if (*substring == ':')   { return false; }
            else if (islower(*substring)) { break; }
        }

        while(*ptr != 0) {
            if      (*ptr == 0) { break; }
            else if (*ptr == ':') {
                ptr++;
                if (*ptr != ':') { return false; }
                ptr++;
                if (*ptr == 0) { return false; }
                break;
            }
            else if (!isalnum(*ptr)) { return false; }
            ptr++;
        }
    }

    return true;
}

int
CFCSymbol_validate_class_name_component(const char *name)
{
    if (!S_validate_class_name(name)) { return false; }
    if (strchr(name, ':') != NULL) { return false; }
    return true;
}

static int
S_validate_class_cnick(const char *class_cnick)
{
    // Allow all caps.
    const char *ptr;
    for (ptr = class_cnick; ; ptr++) {
        if (*ptr == 0) {
            if (strlen(class_cnick)) { return true; }
            else { break; }
        }
        else if (!isupper(*ptr)) { break; }
    }

    // Same as one component of a class name.
    return CFCSymbol_validate_class_name_component(class_cnick);
}

static int
S_validate_identifier(const char *identifier)
{
    const char *ptr = identifier;
    if (!isalpha(*ptr) && *ptr != '_') { return false; }
    for ( ; *ptr != 0; ptr++) {
        if (!isalnum(*ptr) && *ptr != '_') { return false; }
    }
    return true;
}

CFCSymbol*
CFCSymbol_init(CFCSymbol *self, struct CFCParcel *parcel, 
               const char *exposure, const char *class_name, 
               const char *class_cnick, const char *micro_sym)
{
    self->parcel = parcel;

    // Validate exposure.
    if (!S_validate_exposure(exposure)) {
        croak("Invalid exposure: '%s'", exposure ? exposure : "[NULL]");
    }
    self->exposure = CFCUtil_strdup(exposure);

    // Validate class name (if supplied);
    if (class_name && !S_validate_class_name(class_name)) {
        croak("Invalid class_name: '%s'", class_name);
    }
    self->class_name = CFCUtil_strdup(class_name);

    // Derive class_cnick if necessary, then validate.
    if (class_name) {
        if (class_cnick) {
            self->class_cnick = CFCUtil_strdup(class_cnick);
        }
        else {
            const char *last_colon = strrchr(class_name, ':');
            const char *cnick = last_colon ? last_colon + 1 : class_name;
            self->class_cnick = CFCUtil_strdup(cnick);
        }
    }
    else if (class_cnick) {
        // Sanity check class_cnick without class_name.
        croak("Can't supply class_cnick without class_name");
    }
    else {
        self->class_cnick = NULL;
    }
    if (self->class_cnick && !S_validate_class_cnick(self->class_cnick)) {
        croak("Invalid class_cnick: '%s'", self->class_cnick);
    }

    if (!micro_sym || !S_validate_identifier(micro_sym)) {
        croak("Invalid micro_sym: '%s'",  micro_sym ? micro_sym : "[NULL]");
    }
    self->micro_sym = CFCUtil_strdup(micro_sym);

    // Derive short_sym.
    size_t class_cnick_len = self->class_cnick 
                           ? strlen(self->class_cnick) 
                           : 0;
    size_t short_sym_len = class_cnick_len
                         + strlen("_") 
                         + strlen(self->micro_sym);
    self->short_sym = (char*)malloc(short_sym_len + 1);
    if (self->class_cnick) {
        memcpy((void*)self->short_sym, self->class_cnick, class_cnick_len);
    }
    *((char*)&self->short_sym[class_cnick_len]) = '_';
    memcpy((void*)&self->short_sym[class_cnick_len + 1], 
        self->micro_sym, strlen(micro_sym));
    *((char*)&self->short_sym[short_sym_len]) = '\0';

    // Derive full_sym;
    const char *prefix = CFCParcel_get_prefix(self->parcel);
    size_t prefix_len = strlen(prefix);
    size_t full_sym_len = prefix_len + short_sym_len;
    self->full_sym = (char*)malloc(full_sym_len + 1);
    memcpy((void*)self->full_sym, prefix, prefix_len);
    memcpy((void*)&self->full_sym[prefix_len], self->short_sym, short_sym_len);
    *((char*)&self->full_sym[full_sym_len]) = '\0';

    return self;
}

void
CFCSymbol_destroy(CFCSymbol *self)
{
    // SvREFCNT_dec((SV*)self->parcel);
    free(self->exposure);
    free(self->class_name);
    free(self->class_cnick);
    free(self->micro_sym);
    free((void*)self->short_sym);
    free((void*)self->full_sym);
    CFCBase_destroy((CFCBase*)self);
}

int
CFCSymbol_equals(CFCSymbol *self, CFCSymbol *other)
{
    if (strcmp(self->micro_sym, other->micro_sym) != 0) { return false; }
    if (!CFCParcel_equals(self->parcel, other->parcel)) { return false; }
    if (strcmp(self->exposure, other->exposure) != 0) { return false; }
    if (self->class_name) {
        if (!other->class_name) { return false; }
        if (strcmp(self->class_name, other->class_name) != 0) { 
            return false;
        }
    }
    else if (other->class_name) {
        return false; 
    }
    return true;
}

int
CFCSymbol_public(CFCSymbol *self)
{
    return !strcmp(self->exposure, "public");
}

int
CFCSymbol_parcel(CFCSymbol *self)
{
    return !strcmp(self->exposure, "parcel");
}

int
CFCSymbol_private(CFCSymbol *self)
{
    return !strcmp(self->exposure, "private");
}

int
CFCSymbol_local(CFCSymbol *self)
{
    return !strcmp(self->exposure, "local");
}

const char*
CFCSymbol_full_sym(CFCSymbol *self)
{
    return self->full_sym;
}

const char*
CFCSymbol_short_sym(CFCSymbol *self)
{
    return self->short_sym;
}

struct CFCParcel*
CFCSymbol_get_parcel(CFCSymbol *self)
{
    return self->parcel;
}

const char*
CFCSymbol_get_class_name(CFCSymbol *self)
{
    return self->class_name;
}

const char*
CFCSymbol_get_class_cnick(CFCSymbol *self)
{
    return self->class_cnick;
}

const char*
CFCSymbol_get_exposure(CFCSymbol *self)
{
    return self->exposure;
}

const char*
CFCSymbol_micro_sym(CFCSymbol *self)
{
    return self->micro_sym;
}

const char*
CFCSymbol_get_prefix(CFCSymbol *self)
{
    return CFCParcel_get_prefix(self->parcel);
}

const char*
CFCSymbol_get_Prefix(CFCSymbol *self)
{
    return CFCParcel_get_Prefix(self->parcel);
}

const char*
CFCSymbol_get_PREFIX(CFCSymbol *self)
{
    return CFCParcel_get_PREFIX(self->parcel);
}

