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

/** Clownfish::Symbol - Base class for Clownfish symbols.
 *
 * Clownfish::Symbol serves as a parent class for entities which may live in
 * the global namespace, such as classes, functions, methods, and variables.
 */

#ifndef H_CFCSYMBOL
#define H_CFCSYMBOL

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * @param parcel A Clownfish::Parcel.  If not supplied, will be assigned to the
 * default Parcel.
 * @param exposure The scope in which the symbol is exposed.  Must be
 * 'public', 'parcel', 'private', or 'local'.
 * @param class_name A optional class name, consisting of one or more
 * components separated by "::".  Each component must start with a capital
 * letter, contain at least one lower-case letter, and consist entirely of the
 * characters [A-Za-z0-9].
 * @param class_cnick The C nickname associated with the supplied class
 * name.  If not supplied, will be derived if possible from C<class_name> by
 * extracting the last class name component.
 * @param micro_sym The local identifier for the symbol.
 */
CFCSymbol*
CFCSymbol_new(struct CFCParcel *parcel, const char *exposure, const char *class_name,
              const char *class_cnick, const char *micro_sym);

CFCSymbol*
CFCSymbol_init(CFCSymbol *self, struct CFCParcel *parcel, const char *exposure,
               const char *class_name, const char *class_cnick,
               const char *micro_sym);

void
CFCSymbol_destroy(CFCSymbol *self);

/** Return true if the symbols are "equal", false otherwise.
 */
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

/** Return true if the Symbol's exposure is "public".
 */
int
CFCSymbol_public(CFCSymbol *self);

/** Return true if the Symbol's exposure is "parcel".
 */
int
CFCSymbol_parcel(CFCSymbol *self);

/** Return true if the Symbol's exposure is "private".
 */
int
CFCSymbol_private(CFCSymbol *self);

/** Return true if the Symbol's exposure is "local".
 */
int
CFCSymbol_local(CFCSymbol *self);

/** Accessor for the Symbol's micro_sym.
 */
const char*
CFCSymbol_micro_sym(CFCSymbol *self);

/** Returns the C representation for the symbol minus the parcel's prefix,
 * e.g.  "Lobster_average_lifespan".
 */
const char*
CFCSymbol_short_sym(CFCSymbol *self);

/** Returns the fully qualified C representation for the symbol, e.g.
 * "crust_Lobster_average_lifespan".
 */
const char*
CFCSymbol_full_sym(CFCSymbol *self);

/** Get the Symbol's all-lowercase prefix, delegating to <code>parcel</code>.
 */
const char*
CFCSymbol_get_prefix(CFCSymbol *self);

/** Get the Symbol's Titlecase prefix, delegating to <code>parcel</code>.
 */
const char*
CFCSymbol_get_Prefix(CFCSymbol *self);

/** Get the Symbol's all-uppercase prefix, delegating to <code>parcel</code>.
 */
const char*
CFCSymbol_get_PREFIX(CFCSymbol *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCSYMBOL */

