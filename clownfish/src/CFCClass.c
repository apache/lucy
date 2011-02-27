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
#include "CFCClass.h"
#include "CFCFunction.h"
#include "CFCMethod.h"
#include "CFCParcel.h"
#include "CFCDocuComment.h"
#include "CFCUtil.h"
#include "CFCVariable.h"

typedef struct CFCClassAttribute {
    char *name;
    char *value;
} CFCClassAttribute;

struct CFCClass {
    CFCSymbol symbol;
    int tree_grown;
    CFCDocuComment *docucomment;
    struct CFCClass *parent;
    struct CFCClass **children;
    size_t num_kids;
    CFCFunction **functions;
    size_t num_functions;
    CFCMethod **methods;
    size_t num_methods;
    CFCVariable **member_vars;
    size_t num_member_vars;
    CFCVariable **inert_vars;
    size_t num_inert_vars;
    CFCClassAttribute **attributes;
    size_t num_attributes;
    char *autocode;
    char *source_class;
    char *parent_class_name;
    int is_final;
    int is_inert;
    char *struct_sym;
    char *full_struct_sym;
    char *short_vtable_var;
    char *full_vtable_var;
    char *full_vtable_type;
    char *include_h;
};

CFCClass*
CFCClass_new(struct CFCParcel *parcel, const char *exposure, 
              const char *class_name, const char *class_cnick, 
              const char *micro_sym, CFCDocuComment *docucomment, 
              const char *source_class, const char *parent_class_name, 
              int is_final, int is_inert)
{
    CFCClass *self = (CFCClass*)CFCBase_allocate(sizeof(CFCClass),
        "Clownfish::Class");
    return CFCClass_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym, docucomment, source_class, parent_class_name, is_final, 
        is_inert);
}

CFCClass*
CFCClass_init(CFCClass *self, struct CFCParcel *parcel, 
               const char *exposure, const char *class_name, 
               const char *class_cnick, const char *micro_sym, 
               CFCDocuComment *docucomment, const char *source_class, 
               const char *parent_class_name, int is_final, int is_inert)
{
    CFCSymbol_init((CFCSymbol*)self, parcel, exposure, class_name, 
        class_cnick, micro_sym);
    self->parent     = NULL;
    self->tree_grown = false;
    self->autocode   = (char*)CALLOCATE(1, sizeof(char));
    self->children        = (CFCClass**)CALLOCATE(1, sizeof(CFCClass*));
    self->num_kids        = 0;
    self->functions       = (CFCFunction**)CALLOCATE(1, sizeof(CFCFunction*));
    self->num_functions   = 0;
    self->methods         = (CFCMethod**)CALLOCATE(1, sizeof(CFCMethod*));
    self->num_methods     = 0;
    self->member_vars     = (CFCVariable**)CALLOCATE(1, sizeof(CFCVariable*));
    self->num_member_vars = 0;
    self->inert_vars      = (CFCVariable**)CALLOCATE(1, sizeof(CFCVariable*));
    self->num_inert_vars  = 0;
    self->attributes      = (CFCClassAttribute**)CALLOCATE(1, sizeof(CFCClassAttribute*));
    self->num_attributes  = 0;
    self->parent_class_name = CFCUtil_strdup(parent_class_name);
    self->docucomment 
        = (CFCDocuComment*)CFCBase_incref((CFCBase*)docucomment);

    // Assume that Foo::Bar should be found in Foo/Bar.h.
    self->source_class = source_class 
                       ? CFCUtil_strdup(source_class)
                       : CFCUtil_strdup(class_name);

    // Cache several derived symbols.
    const char *last_colon = strrchr(class_name, ':');
    self->struct_sym = last_colon 
                     ? CFCUtil_strdup(last_colon + 1)
                     : CFCUtil_strdup(class_name);
    const char *prefix = CFCSymbol_get_prefix((CFCSymbol*)self);
    size_t prefix_len = strlen(prefix);
    size_t struct_sym_len = strlen(self->struct_sym);
    self->short_vtable_var = (char*)MALLOCATE(struct_sym_len + 1);
    self->full_struct_sym  = (char*)MALLOCATE(prefix_len + struct_sym_len + 1);
    self->full_vtable_var  = (char*)MALLOCATE(prefix_len + struct_sym_len + 1);
    self->full_vtable_type = (char*)MALLOCATE(prefix_len + struct_sym_len + 3 + 1);
    size_t i;
    for (i = 0; i < struct_sym_len; i++) {
        self->short_vtable_var[i] = toupper(self->struct_sym[i]);
    }
    self->short_vtable_var[struct_sym_len] = '\0';
    int check = sprintf(self->full_struct_sym, "%s%s", prefix,
        self->struct_sym);
    if (check < 0) { croak("sprintf failed"); }
    for (i = 0; self->full_struct_sym[i] != '\0'; i++) {
        self->full_vtable_var[i] = toupper(self->full_struct_sym[i]);
    }
    self->full_vtable_var[i] = '\0';
    check = sprintf(self->full_vtable_type, "%s_VT", self->full_vtable_var);
    if (check < 0) { croak("sprintf failed"); }

    // Cache the relative path to the autogenerated C header file.
    size_t source_class_len = strlen(self->source_class);
    self->include_h = (char*)MALLOCATE(source_class_len + 3);
    int j;
    for (i = 0, j = 0; i < source_class_len; i++) {
        if (self->source_class[i] == ':') {
            self->include_h[j++] = '/';
            i++;
        }
        else {
            self->include_h[j++] = self->source_class[i];
        }
    }
    self->include_h[j] = '\0';
    strcat(self->include_h, ".h");

    self->is_final = !!is_final;
    self->is_inert = !!is_inert;

    return self;
}

void
CFCClass_destroy(CFCClass *self)
{
    CFCBase_decref((CFCBase*)self->docucomment);
    CFCBase_decref((CFCBase*)self->parent);
    size_t i;
    for (i = 0; self->children[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->children[i]);
    }
    for (i = 0; self->functions[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->functions[i]);
    }
    for (i = 0; self->methods[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->methods[i]);
    }
    for (i = 0; self->member_vars[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->member_vars[i]);
    }
    for (i = 0; self->inert_vars[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->inert_vars[i]);
    }
    for (i = 0; self->attributes[i] != NULL; i++) {
        CFCClassAttribute *attribute = self->attributes[i];
        FREEMEM(attribute->name);
        FREEMEM(attribute->value);
        FREEMEM(attribute);
    }
    FREEMEM(self->children);
    FREEMEM(self->functions);
    FREEMEM(self->methods);
    FREEMEM(self->member_vars);
    FREEMEM(self->inert_vars);
    FREEMEM(self->attributes);
    FREEMEM(self->autocode);
    FREEMEM(self->source_class);
    FREEMEM(self->parent_class_name);
    FREEMEM(self->struct_sym);
    FREEMEM(self->short_vtable_var);
    FREEMEM(self->full_struct_sym);
    FREEMEM(self->full_vtable_var);
    FREEMEM(self->full_vtable_type);
    CFCSymbol_destroy((CFCSymbol*)self);
}

void
CFCClass_add_child(CFCClass *self, CFCClass *child)
{
    CFCUTIL_NULL_CHECK(child);
    if (self->tree_grown) { croak("Can't call add_child after grow_tree"); }
    self->num_kids++;
    size_t size = (self->num_kids + 1) * sizeof(CFCClass*);
    self->children = (CFCClass**)REALLOCATE(self->children, size);
    self->children[self->num_kids - 1] 
        = (CFCClass*)CFCBase_incref((CFCBase*)child);
    self->children[self->num_kids] = NULL;
}

void
CFCClass_add_function(CFCClass *self, CFCFunction *func)
{
    CFCUTIL_NULL_CHECK(func);
    if (self->tree_grown) { 
        croak("Can't call add_function after grow_tree"); 
    }
    self->num_functions++;
    size_t size = (self->num_functions + 1) * sizeof(CFCFunction*);
    self->functions = (CFCFunction**)REALLOCATE(self->functions, size);
    self->functions[self->num_functions - 1] 
        = (CFCFunction*)CFCBase_incref((CFCBase*)func);
    self->functions[self->num_functions] = NULL;
}

void
CFCClass_add_method(CFCClass *self, CFCMethod *method)
{
    CFCUTIL_NULL_CHECK(method);
    if (self->tree_grown) { 
        croak("Can't call add_method after grow_tree"); 
    }
    if (self->is_inert) {
        croak("Can't add_method to an inert class");
    }
    self->num_methods++;
    size_t size = (self->num_methods + 1) * sizeof(CFCMethod*);
    self->methods = (CFCMethod**)REALLOCATE(self->methods, size);
    self->methods[self->num_methods - 1] 
        = (CFCMethod*)CFCBase_incref((CFCBase*)method);
    self->methods[self->num_methods] = NULL;
}

void
CFCClass_zap_methods(CFCClass *self)
{
    size_t i;
    for (i = 0; self->methods[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->methods[i]);
    }
    self->methods[0] = NULL;
    self->num_methods = 0;
}

void
CFCClass_add_member_var(CFCClass *self, CFCVariable *var)
{
    CFCUTIL_NULL_CHECK(var);
    if (self->tree_grown) { 
        croak("Can't call add_member_var after grow_tree"); 
    }
    self->num_member_vars++;
    size_t size = (self->num_member_vars + 1) * sizeof(CFCVariable*);
    self->member_vars = (CFCVariable**)REALLOCATE(self->member_vars, size);
    self->member_vars[self->num_member_vars - 1] 
        = (CFCVariable*)CFCBase_incref((CFCBase*)var);
    self->member_vars[self->num_member_vars] = NULL;
}

void
CFCClass_add_inert_var(CFCClass *self, CFCVariable *var)
{
    CFCUTIL_NULL_CHECK(var);
    if (self->tree_grown) { 
        croak("Can't call add_inert_var after grow_tree"); 
    }
    self->num_inert_vars++;
    size_t size = (self->num_inert_vars + 1) * sizeof(CFCVariable*);
    self->inert_vars = (CFCVariable**)REALLOCATE(self->inert_vars, size);
    self->inert_vars[self->num_inert_vars - 1] 
        = (CFCVariable*)CFCBase_incref((CFCBase*)var);
    self->inert_vars[self->num_inert_vars] = NULL;
}

void
CFCClass_add_attribute(CFCClass *self, const char *name, const char *value)
{
    if (!name || !strlen(name)) { croak("'name' is required"); }
    if (CFCClass_has_attribute(self, name)) { 
        croak("Attribute '%s' already registered");
    }
    CFCClassAttribute *attribute 
        = (CFCClassAttribute*)MALLOCATE(sizeof(CFCClassAttribute));
    attribute->name  = CFCUtil_strdup(name);
    attribute->value = CFCUtil_strdup(value);
    self->num_attributes++;
    size_t size = (self->num_attributes + 1) * sizeof(CFCClassAttribute*);
    self->attributes = (CFCClassAttribute**)REALLOCATE(self->attributes, size);
    self->attributes[self->num_attributes - 1] = attribute;
    self->attributes[self->num_attributes] = NULL;

}

int
CFCClass_has_attribute(CFCClass *self, const char *name)
{
    CFCUTIL_NULL_CHECK(name);
    size_t i;
    for (i = 0; i < self->num_attributes; i++) {
        if (strcmp(name, self->attributes[i]->name) == 0) {
            return true;
        }
    }
    return false;
}

static CFCFunction*
S_find_func(CFCFunction **funcs, const char *sym)
{
    const size_t MAX_LEN = 128;
    char lcsym[MAX_LEN + 1];
    size_t sym_len = strlen(sym);
    if (sym_len > MAX_LEN) { croak("sym too long: '%s'", sym); }
    size_t i;
    for (i = 0; i <= sym_len; i++) {
        lcsym[i] = tolower(sym[i]);
    }
    for (i = 0; funcs[i] != NULL; i++) {
        CFCFunction *func = funcs[i];
        const char *func_micro_sym = CFCSymbol_micro_sym((CFCSymbol*)func);
        if (strcmp(lcsym, func_micro_sym) == 0) {
            return func;
        }
    }
    return NULL;
}

CFCFunction*
CFCClass_function(CFCClass *self, const char *sym)
{
    return S_find_func(self->functions, sym);
}

CFCMethod*
CFCClass_method(CFCClass *self, const char *sym)
{
    return (CFCMethod*)S_find_func((CFCFunction**)self->methods, sym);
}

CFCMethod*
CFCClass_novel_method(CFCClass *self, const char *sym)
{
    CFCMethod *method = CFCClass_method(self, sym);
    if (method) {
        const char *cnick = CFCClass_get_cnick(self);
        const char *meth_cnick 
            = CFCSymbol_get_class_cnick((CFCSymbol*)method);
        if (strcmp(cnick, meth_cnick) == 0) {
            return method;
        }
    }
    return NULL;
}

// Pass down member vars to from parent to children.
void
CFCClass_bequeath_member_vars(CFCClass *self)
{
    size_t i;
    for (i = 0; self->children[i] != NULL; i++) {
        CFCClass *child = self->children[i];
        size_t num_vars = self->num_member_vars + child->num_member_vars;
        size_t size = (num_vars + 1) * sizeof(CFCVariable*);
        child->member_vars 
            = (CFCVariable**)REALLOCATE(child->member_vars, size);
        memmove(child->member_vars + self->num_member_vars,
            child->member_vars, 
            child->num_member_vars * sizeof(CFCVariable*));
        memcpy(child->member_vars, self->member_vars, 
            self->num_member_vars * sizeof(CFCVariable*));
        size_t j;
        for (j = 0; self->member_vars[j] != NULL; j++) {
            CFCBase_incref((CFCBase*)child->member_vars[j]);
        }
        child->num_member_vars = num_vars;
        child->member_vars[num_vars] = NULL;
        CFCClass_bequeath_member_vars(child);
    }
}

void
CFCClass_bequeath_methods(CFCClass *self)
{
    size_t child_num;
    for (child_num = 0; self->children[child_num] != NULL; child_num++) {
        CFCClass *child = self->children[child_num];

        // Create array of methods, preserving exact order so vtables match up.
        size_t num_methods = 0;
        size_t max_methods = self->num_methods + child->num_methods;
        CFCMethod **methods = (CFCMethod**)MALLOCATE(
            (max_methods + 1) * sizeof(CFCMethod*));

        // Gather methods which child inherits or overrides.
        size_t i;
        for (i = 0; i < self->num_methods; i++) {
            CFCMethod *method = self->methods[i];
            const char *micro_sym = CFCSymbol_micro_sym((CFCSymbol*)method);
            CFCMethod *child_method = CFCClass_method(child, micro_sym);
            if (child_method) {
                CFCMethod_override(child_method, method);
                methods[num_methods++] = child_method;
            }
            else {
                methods[num_methods++] = method;
            }
        }

        // Append novel child methods to array.  Child methods which were just
        // marked via CFCMethod_override() a moment ago are skipped.
        for (i = 0; i < child->num_methods; i++) {
            CFCMethod *method = child->methods[i];
            if (CFCMethod_novel(method)) {
                methods[num_methods++] = method;
            }
        }
        methods[num_methods] = NULL;
        
        // Manage refcounts and assign new array.  Transform to final methods
        // if child class is a final class.
        if (child->is_final) {
            for (i = 0; i < num_methods; i++) {
                methods[i] = CFCMethod_finalize(methods[i]);
            }
        }
        else {
            for (i = 0; i < num_methods; i++) {
                CFCBase_incref((CFCBase*)methods[i]);
            }
        }
        for (i = 0; i < child->num_methods; i++) {
            CFCBase_decref((CFCBase*)child->methods[i]);
        }
        FREEMEM(child->methods);
        child->methods     = methods;
        child->num_methods = num_methods;

        // Pass it all down to the next generation.
        CFCClass_bequeath_methods(child);
        CFCClass_set_tree_grown(child, true);
    }
}

// Let the children know who their parent class is.
void
CFCClass_establish_ancestry(CFCClass *self)
{
    size_t i;
    for (i = 0; i < self->num_kids; i++) {
        CFCClass *child = self->children[i];
        // This is a circular reference and thus a memory leak, but we don't
        // care, because we have to have everything in memory at once anyway.
        CFCClass_set_parent(child, self);
        CFCClass_establish_ancestry(child);
    }
}

static size_t
S_family_tree_size(CFCClass *self)
{
    size_t count = 1; // self
    size_t i;
    for (i = 0; i < self->num_kids; i++) {
        count += S_family_tree_size(self->children[i]);
    }
    return count;
}

// Return value is valid only so long as object persists (elements are not
// refcounted).
CFCClass**
CFCClass_tree_to_ladder(CFCClass *self)
{
    size_t ladder_len = S_family_tree_size(self);
    CFCClass **ladder = MALLOCATE((ladder_len + 1) * sizeof(CFCClass*));
    ladder[ladder_len] = NULL;
    size_t step = 0;
    ladder[step++] = self;
    size_t i;
    for (i = 0; i < self->num_kids; i++) {
        CFCClass *child = self->children[i];
        CFCClass **child_ladder = CFCClass_tree_to_ladder(child);
        size_t j;
        for (j = 0; child_ladder[j] != NULL; j++) {
            ladder[step++] = child_ladder[j];
        }
        FREEMEM(child_ladder);
    }
    return ladder;
}

static CFCSymbol**
S_novel_syms(CFCClass *self, CFCSymbol **syms)
{
    const char *cnick = CFCSymbol_get_class_cnick((CFCSymbol*)self);
    size_t count = 0;
    while (syms[count] != NULL) { count++; }
    size_t amount = (count + 1) * sizeof(CFCSymbol*);
    CFCSymbol **novel = (CFCSymbol**)MALLOCATE(amount);
    size_t num_novel = 0;
    size_t i;
    for (i = 0; i < count; i++) {
        CFCSymbol *sym = syms[i];
        const char *sym_cnick = CFCSymbol_get_class_cnick((CFCSymbol*)sym);
        if (strcmp(sym_cnick, cnick) == 0) {
            novel[num_novel++] = sym;
        }
    }
    novel[num_novel] = NULL;
    return novel;
}

CFCMethod**
CFCClass_novel_methods(CFCClass *self)
{
    return (CFCMethod**)S_novel_syms(self, (CFCSymbol**)self->methods);
}

CFCVariable**
CFCClass_novel_member_vars(CFCClass *self)
{
    return (CFCVariable**)S_novel_syms(self, (CFCSymbol**)self->member_vars);
}

CFCClass**
CFCClass_children(CFCClass *self)
{
    return self->children;
}

CFCFunction**
CFCClass_functions(CFCClass *self)
{
    return self->functions;
}

CFCMethod**
CFCClass_methods(CFCClass *self)
{
    return self->methods;
}

CFCVariable**
CFCClass_member_vars(CFCClass *self)
{
    return self->member_vars;
}

CFCVariable**
CFCClass_inert_vars(CFCClass *self)
{
    return self->inert_vars;
}

const char*
CFCClass_get_cnick(CFCClass *self)
{
    return CFCSymbol_get_class_cnick((CFCSymbol*)self);
}

void
CFCClass_set_tree_grown(CFCClass *self, int tree_grown)
{
    self->tree_grown = !!tree_grown;
}

int
CFCClass_tree_grown(CFCClass *self)
{
    return self->tree_grown;
}

void
CFCClass_set_parent(CFCClass *self, CFCClass *parent)
{
    CFCBase_decref((CFCBase*)self->parent);
    self->parent = (CFCClass*)CFCBase_incref((CFCBase*)parent);
}

CFCClass*
CFCClass_get_parent(CFCClass *self)
{
    return self->parent;
}

void
CFCClass_append_autocode(CFCClass *self, const char *autocode)
{
    size_t size = strlen(self->autocode) + strlen(autocode) + 1;
    self->autocode = (char*)REALLOCATE(self->autocode, size);
    strcat(self->autocode, autocode);
}

const char*
CFCClass_get_autocode(CFCClass *self)
{
    return self->autocode;
}

const char*
CFCClass_get_source_class(CFCClass *self)
{
    return self->source_class;
}

const char*
CFCClass_get_parent_class_name(CFCClass *self)
{
    return self->parent_class_name;
}

int
CFCClass_final(CFCClass *self)
{
    return self->is_final;
}

int
CFCClass_inert(CFCClass *self)
{
    return self->is_inert;
}

const char*
CFCClass_get_struct_sym(CFCClass *self)
{
    return self->struct_sym;
}

const char*
CFCClass_full_struct_sym(CFCClass *self)
{
    return self->full_struct_sym;
}

const char*
CFCClass_short_vtable_var(CFCClass *self)
{
    return self->short_vtable_var;
}

const char*
CFCClass_full_vtable_var(CFCClass *self)
{
    return self->full_vtable_var;
}

const char*
CFCClass_full_vtable_type(CFCClass *self)
{
    return self->full_vtable_type;
}

const char*
CFCClass_include_h(CFCClass *self)
{
    return self->include_h;
}

struct CFCDocuComment*
CFCClass_get_docucomment(CFCClass *self)
{
    return self->docucomment;
}

