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
#include "CFCParcel.h"
#include "CFCUtil.h"

struct CFCClass {
    CFCSymbol symbol;
    int tree_grown;
    struct CFCClass *parent;
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
};

CFCClass*
CFCClass_new(struct CFCParcel *parcel, const char *exposure, 
              const char *class_name, const char *class_cnick, 
              const char *micro_sym, const char *source_class,
              const char *parent_class_name, int is_final, int is_inert)
{
    CFCClass *self = (CFCClass*)CFCBase_allocate(sizeof(CFCClass),
        "Clownfish::Class");
    if (!self) { croak("malloc failed"); }
    return CFCClass_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym, source_class, parent_class_name, is_final, is_inert);
}

CFCClass*
CFCClass_init(CFCClass *self, struct CFCParcel *parcel, 
               const char *exposure, const char *class_name, 
               const char *class_cnick, const char *micro_sym,
               const char *source_class, const char *parent_class_name, 
               int is_final, int is_inert)
{
    CFCSymbol_init((CFCSymbol*)self, parcel, exposure, class_name, 
        class_cnick, micro_sym);
    self->parent     = NULL;
    self->tree_grown = false;
    self->autocode   = (char*)calloc(1, sizeof(char));
    self->parent_class_name = CFCUtil_strdup(parent_class_name);

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
    self->short_vtable_var = (char*)malloc(struct_sym_len + 1);
    self->full_struct_sym  = (char*)malloc(prefix_len + struct_sym_len + 1);
    self->full_vtable_var  = (char*)malloc(prefix_len + struct_sym_len + 1);
    self->full_vtable_type = (char*)malloc(prefix_len + struct_sym_len + 3 + 1);
    if (   !self->full_struct_sym || !self->short_vtable_var 
        || !self->full_vtable_var || !self->full_vtable_type
    ) {
        croak("malloc failed");
    }
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

    self->is_final = !!is_final;
    self->is_inert = !!is_inert;

    return self;
}

void
CFCClass_destroy(CFCClass *self)
{
    CFCBase_decref((CFCBase*)self->parent);
    free(self->autocode);
    free(self->source_class);
    free(self->parent_class_name);
    free(self->struct_sym);
    free(self->short_vtable_var);
    free(self->full_struct_sym);
    free(self->full_vtable_var);
    free(self->full_vtable_type);
    CFCSymbol_destroy((CFCSymbol*)self);
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
    self->autocode = (char*)realloc(self->autocode, size);
    if (!self->autocode) { croak("realloc failed"); }
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

