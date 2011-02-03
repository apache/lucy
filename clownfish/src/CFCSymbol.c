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

CFCSymbol*
CFCSymbol_new(struct CFCParcel *parcel, const char *exposure, 
              const char *class_name, const char *class_cnick, 
              const char *micro_sym)
{
    CFCSymbol *self = (CFCSymbol*)malloc(sizeof(CFCSymbol));
    if (!self) { croak("malloc failed"); }
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
    if (!S_validate_class_name(class_cnick)) { return false; }
    if (strchr(class_cnick, ':') != NULL) { return false; }
    return true;
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
    self->exposure = savepv(exposure);

    // Validate class name (if supplied);
    if (class_name && !S_validate_class_name(class_name)) {
        croak("Invalid class_name: '%s'", class_name);
    }
    self->class_name = savepv(class_name);

    // Derive class_cnick if necessary, then validate.
    if (class_name) {
        if (class_cnick) {
            self->class_cnick = savepv(class_cnick);
        }
        else {
            const char *last_colon = strrchr(class_name, ':');
            const char *cnick = last_colon ? last_colon + 1 : class_name;
            self->class_cnick = savepv(cnick);
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
    self->micro_sym = savepv(micro_sym);

    return self;
}

void
CFCSymbol_destroy(CFCSymbol *self)
{
    // SvREFCNT_dec((SV*)self->parcel);
    Safefree(self->exposure);
    Safefree(self->class_name);
    Safefree(self->class_cnick);
    Safefree(self->micro_sym);
    free(self);
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

