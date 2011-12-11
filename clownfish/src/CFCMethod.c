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
#include <ctype.h>
#include <stdio.h>

#define CFC_NEED_FUNCTION_STRUCT_DEF
#include "CFCFunction.h"
#include "CFCMethod.h"
#include "CFCType.h"
#include "CFCUtil.h"
#include "CFCParamList.h"
#include "CFCParcel.h"
#include "CFCDocuComment.h"
#include "CFCVariable.h"

#ifndef true
    #define true 1
    #define false 0
#endif

struct CFCMethod {
    CFCFunction function;
    char *macro_sym;
    char *short_typedef;
    char *full_typedef;
    char *full_callback_sym;
    char *full_override_sym;
    int is_final;
    int is_abstract;
    int is_novel;
};

static void
S_update_typedefs(CFCMethod *self, const char *short_sym);

CFCMethod*
CFCMethod_new(CFCParcel *parcel, const char *exposure, const char *class_name,
              const char *class_cnick, const char *macro_sym,
              CFCType *return_type, CFCParamList *param_list,
              CFCDocuComment *docucomment, int is_final, int is_abstract) {
    CFCMethod *self = (CFCMethod*)CFCBase_allocate(sizeof(CFCMethod),
                                                   "Clownfish::Method");
    return CFCMethod_init(self, parcel, exposure, class_name, class_cnick,
                          macro_sym, return_type, param_list, docucomment,
                          is_final, is_abstract);
}

static int
S_validate_macro_sym(const char *macro_sym) {
    if (!macro_sym || !strlen(macro_sym)) { return false; }

    int need_upper  = true;
    int need_letter = true;
    for (;; macro_sym++) {
        if (need_upper  && !isupper(*macro_sym)) { return false; }
        if (need_letter && !isalpha(*macro_sym)) { return false; }
        need_upper  = false;
        need_letter = false;

        // We've reached NULL-termination without problems, so succeed.
        if (!*macro_sym) { return true; }

        if (!isalnum(*macro_sym)) {
            if (*macro_sym != '_') { return false; }
            need_upper  = true;
        }
    }
}

CFCMethod*
CFCMethod_init(CFCMethod *self, CFCParcel *parcel, const char *exposure,
               const char *class_name, const char *class_cnick,
               const char *macro_sym, CFCType *return_type,
               CFCParamList *param_list, CFCDocuComment *docucomment,
               int is_final, int is_abstract) {
    // Validate macro_sym, derive micro_sym.
    if (!S_validate_macro_sym(macro_sym)) {
        CFCUtil_die("Invalid macro_sym: '%s'",
                    macro_sym ? macro_sym : "[NULL]");
    }
    char *micro_sym = CFCUtil_strdup(macro_sym);
    size_t i;
    for (i = 0; micro_sym[i] != '\0'; i++) {
        micro_sym[i] = tolower(micro_sym[i]);
    }

    // Super-init and clean up derived micro_sym.
    CFCFunction_init((CFCFunction*)self, parcel, exposure, class_name,
                     class_cnick, micro_sym, return_type, param_list,
                     docucomment, false);
    FREEMEM(micro_sym);

    // Verify that the first element in the arg list is a self.
    CFCVariable **args = CFCParamList_get_variables(param_list);
    if (!args[0]) { CFCUtil_die("Missing 'self' argument"); }
    CFCType *type = CFCVariable_get_type(args[0]);
    const char *specifier = CFCType_get_specifier(type);
    const char *prefix    = CFCMethod_get_prefix(self);
    const char *last_colon = strrchr(class_name, ':');
    const char *struct_sym = last_colon ? last_colon + 1 : class_name;
    char *wanted = (char*)MALLOCATE(strlen(prefix) + strlen(struct_sym) + 1);
    sprintf(wanted, "%s%s", prefix, struct_sym);
    int mismatch = strcmp(wanted, specifier);
    FREEMEM(wanted);
    if (mismatch) {
        CFCUtil_die("First arg type doesn't match class: '%s' '%s",
                    class_name, specifier);
    }

    self->macro_sym     = CFCUtil_strdup(macro_sym);
    self->short_typedef = NULL;
    self->full_typedef  = NULL;
    self->is_final      = is_final;
    self->is_abstract   = is_abstract;

    // Derive more symbols.
    const char *full_func_sym = CFCMethod_implementing_func_sym(self);
    size_t amount = strlen(full_func_sym) + sizeof("_OVERRIDE") + 1;
    self->full_callback_sym = (char*)MALLOCATE(amount);
    self->full_override_sym = (char*)MALLOCATE(amount);
    sprintf(self->full_callback_sym, "%s_CALLBACK", full_func_sym);
    sprintf(self->full_override_sym, "%s_OVERRIDE", full_func_sym);

    // Assume that this method is novel until we discover when applying
    // inheritance that it was overridden.
    self->is_novel = 1;

    // Cache typedefs.
    S_update_typedefs(self, CFCSymbol_short_sym((CFCSymbol*)self));

    return self;
}

void
CFCMethod_destroy(CFCMethod *self) {
    FREEMEM(self->macro_sym);
    FREEMEM(self->short_typedef);
    FREEMEM(self->full_typedef);
    FREEMEM(self->full_callback_sym);
    FREEMEM(self->full_override_sym);
    CFCFunction_destroy((CFCFunction*)self);
}

int
CFCMethod_compatible(CFCMethod *self, CFCMethod *other) {
    if (!other) { return false; }
    if (strcmp(self->macro_sym, other->macro_sym)) { return false; }
    int my_public = CFCMethod_public(self);
    int other_public = CFCMethod_public(other);
    if (!!my_public != !!other_public) { return false; }

    // Check arguments and initial values.
    CFCParamList *my_param_list    = self->function.param_list;
    CFCParamList *other_param_list = other->function.param_list;
    CFCVariable **my_args    = CFCParamList_get_variables(my_param_list);
    CFCVariable **other_args = CFCParamList_get_variables(other_param_list);
    const char  **my_vals    = CFCParamList_get_initial_values(my_param_list);
    const char  **other_vals = CFCParamList_get_initial_values(other_param_list);
    size_t i;
    for (i = 1; ; i++) {  // start at 1, skipping self
        if (!!my_args[i] != !!other_args[i]) { return false; }
        if (!!my_vals[i] != !!other_vals[i]) { return false; }
        if (my_vals[i]) {
            if (strcmp(my_vals[i], other_vals[i])) { return false; }
        }
        if (my_args[i]) {
            if (!CFCVariable_equals(my_args[i], other_args[i])) {
                return false;
            }
        }
        else {
            break;
        }
    }

    // Check return types.
    CFCType *type       = CFCMethod_get_return_type(self);
    CFCType *other_type = CFCMethod_get_return_type(other);
    if (CFCType_is_object(type)) {
        // Weak validation to allow covariant object return types.
        if (!CFCType_is_object(other_type)) { return false; }
        if (!CFCType_similar(type, other_type)) { return false; }
    }
    else {
        if (!CFCType_equals(type, other_type)) { return false; }
    }

    return true;
}

void
CFCMethod_override(CFCMethod *self, CFCMethod *orig) {
    // Check that the override attempt is legal.
    if (CFCMethod_final(orig)) {
        const char *orig_class = CFCMethod_get_class_name(orig);
        const char *my_class   = CFCMethod_get_class_name(self);
        CFCUtil_die("Attempt to override final method '%s' from '%s' by '%s'",
                    orig->macro_sym, orig_class, my_class);
    }
    if (!CFCMethod_compatible(self, orig)) {
        const char *func      = CFCMethod_implementing_func_sym(self);
        const char *orig_func = CFCMethod_implementing_func_sym(orig);
        CFCUtil_die("Non-matching signatures for %s and %s", func, orig_func);
    }

    // Mark the Method as no longer novel.
    self->is_novel = false;
}

CFCMethod*
CFCMethod_finalize(CFCMethod *self) {
    CFCParcel  *parcel      = CFCMethod_get_parcel(self);
    const char *exposure    = CFCMethod_get_exposure(self);
    const char *class_name  = CFCMethod_get_class_name(self);
    const char *class_cnick = CFCMethod_get_class_cnick(self);
    CFCMethod  *finalized
        = CFCMethod_new(parcel, exposure, class_name, class_cnick,
                        self->macro_sym, self->function.return_type,
                        self->function.param_list,
                        self->function.docucomment, true,
                        self->is_abstract);
    finalized->is_novel = self->is_final; // Is this right?
    S_update_typedefs(finalized, CFCSymbol_short_sym((CFCSymbol*)self));
    return finalized;
}

size_t
CFCMethod_short_method_sym(CFCMethod *self, const char *invoker, char *buf,
                           size_t buf_size) {
    CFCUTIL_NULL_CHECK(invoker);
    size_t needed = strlen(invoker) + 1 + strlen(self->macro_sym) + 1;
    if (buf_size >= needed) {
        sprintf(buf, "%s_%s", invoker, self->macro_sym);
    }
    return needed;
}

size_t
CFCMethod_full_method_sym(CFCMethod *self, const char *invoker, char *buf,
                          size_t buf_size) {
    CFCUTIL_NULL_CHECK(invoker);
    const char *Prefix = CFCMethod_get_Prefix(self);
    size_t needed = strlen(Prefix)
                    + strlen(invoker)
                    + 1
                    + strlen(self->macro_sym)
                    + 1;
    if (buf_size >= needed) {
        sprintf(buf, "%s%s_%s", Prefix, invoker, self->macro_sym);
    }
    return needed;
}

size_t
CFCMethod_full_offset_sym(CFCMethod *self, const char *invoker, char *buf,
                          size_t buf_size) {
    CFCUTIL_NULL_CHECK(invoker);
    size_t needed = CFCMethod_full_method_sym(self, invoker, NULL, 0)
                    + strlen("_OFFSET");
    if (buf_size >= needed) {
        CFCMethod_full_method_sym(self, invoker, buf, buf_size);
        strcat(buf, "_OFFSET");
    }
    return needed;
}

const char*
CFCMethod_get_macro_sym(CFCMethod *self) {
    return self->macro_sym;
}

const char*
CFCMethod_micro_sym(CFCMethod *self) {
    return CFCSymbol_micro_sym((CFCSymbol*)self);
}

static void
S_update_typedefs(CFCMethod *self, const char *short_sym) {
    FREEMEM(self->short_typedef);
    FREEMEM(self->full_typedef);
    if (short_sym) {
        const char *prefix = CFCMethod_get_prefix(self);
        size_t amount = strlen(short_sym) + 3;
        self->short_typedef = (char*)MALLOCATE(amount);
        sprintf(self->short_typedef, "%s_t", short_sym);
        amount += strlen(prefix);
        self->full_typedef = (char*)MALLOCATE(amount);
        sprintf(self->full_typedef, "%s%s_t", prefix, short_sym);
    }
    else {
        self->short_typedef = NULL;
        self->full_typedef = NULL;
    }
}

const char*
CFCMethod_short_typedef(CFCMethod *self) {
    return self->short_typedef;
}

const char*
CFCMethod_full_typedef(CFCMethod *self) {
    return self->full_typedef;
}

const char*
CFCMethod_full_callback_sym(CFCMethod *self) {
    return self->full_callback_sym;
}

const char*
CFCMethod_full_override_sym(CFCMethod *self) {
    return self->full_override_sym;
}

int
CFCMethod_final(CFCMethod *self) {
    return self->is_final;
}

int
CFCMethod_abstract(CFCMethod *self) {
    return self->is_abstract;
}

int
CFCMethod_novel(CFCMethod *self) {
    return self->is_novel;
}

CFCType*
CFCMethod_self_type(CFCMethod *self) {
    CFCVariable **vars = CFCParamList_get_variables(self->function.param_list);
    return CFCVariable_get_type(vars[0]);
}

CFCParcel*
CFCMethod_get_parcel(CFCMethod *self) {
    return CFCSymbol_get_parcel((CFCSymbol*)self);
}

const char*
CFCMethod_get_prefix(CFCMethod *self) {
    return CFCSymbol_get_prefix((CFCSymbol*)self);
}

const char*
CFCMethod_get_Prefix(CFCMethod *self) {
    return CFCSymbol_get_Prefix((CFCSymbol*)self);
}

const char*
CFCMethod_get_exposure(CFCMethod *self) {
    return CFCSymbol_get_exposure((CFCSymbol*)self);
}

const char*
CFCMethod_get_class_name(CFCMethod *self) {
    return CFCSymbol_get_class_name((CFCSymbol*)self);
}

const char*
CFCMethod_get_class_cnick(CFCMethod *self) {
    return CFCSymbol_get_class_cnick((CFCSymbol*)self);
}

int
CFCMethod_public(CFCMethod *self) {
    return CFCSymbol_public((CFCSymbol*)self);
}

CFCType*
CFCMethod_get_return_type(CFCMethod *self) {
    return CFCFunction_get_return_type((CFCFunction*)self);
}

CFCParamList*
CFCMethod_get_param_list(CFCMethod *self) {
    return CFCFunction_get_param_list((CFCFunction*)self);
}

const char*
CFCMethod_implementing_func_sym(CFCMethod *self) {
    return CFCFunction_full_func_sym((CFCFunction*)self);
}

const char*
CFCMethod_short_implementing_func_sym(CFCMethod *self) {
    return CFCFunction_short_func_sym((CFCFunction*)self);
}

