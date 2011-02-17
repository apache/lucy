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

#ifndef H_CFCSYMBOL
#define H_CFCSYMBOL

typedef struct CFCSymbol CFCSymbol;
struct CFCParcel;

#ifdef CFC_NEED_SYMBOL_STRUCT_DEF
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
struct CFCSymbol {
    CFCBase base;
    struct CFCParcel *parcel;
    char *exposure;
    char *class_name;
    char *class_cnick;
    char *micro_sym;
    char *short_sym;
    char *full_sym;
};
#endif

/** Return true if the supplied string is comprised solely of alphanumeric
 * characters, begins with an uppercase letter, and contains at least one
 * lower case letter.
 */
int
CFCSymbol_validate_class_name_component(const char *name);

CFCSymbol*
CFCSymbol_new(struct CFCParcel *parcel, const char *exposure, const char *class_name, 
              const char *class_cnick, const char *micro_sym);

CFCSymbol*
CFCSymbol_init(CFCSymbol *self, struct CFCParcel *parcel, const char *exposure, 
               const char *class_name, const char *class_cnick, 
               const char *micro_sym);

void
CFCSymbol_destroy(CFCSymbol *self);

int
CFCSymbol_equals(CFCSymbol *self, CFCSymbol *other);

struct CFCParcel*
CFCSymbol_get_parcel(CFCSymbol *self);

// May be NULL.
const char*
CFCSymbol_get_class_name(CFCSymbol *self);

// May be NULL.
const char*
CFCSymbol_get_class_cnick(CFCSymbol *self);

const char*
CFCSymbol_get_exposure(CFCSymbol *self);

int
CFCSymbol_public(CFCSymbol *self);

int
CFCSymbol_parcel(CFCSymbol *self);

int
CFCSymbol_private(CFCSymbol *self);

int
CFCSymbol_local(CFCSymbol *self);

const char*
CFCSymbol_micro_sym(CFCSymbol *self);

const char*
CFCSymbol_short_sym(CFCSymbol *self);

const char*
CFCSymbol_full_sym(CFCSymbol *self);

const char*
CFCSymbol_get_prefix(CFCSymbol *self);

const char*
CFCSymbol_get_Prefix(CFCSymbol *self);

const char*
CFCSymbol_get_PREFIX(CFCSymbol *self);

#endif /* H_CFCSYMBOL */

