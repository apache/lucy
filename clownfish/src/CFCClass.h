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

#ifndef H_CFCCLASS
#define H_CFCCLASS

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCClass CFCClass;
struct CFCParcel;
struct CFCDocuComment;
struct CFCFunction;
struct CFCMethod;
struct CFCVariable;

CFCClass*
CFCClass_create(struct CFCParcel *parcel, const char *exposure,
                const char *class_name, const char *cnick,
                const char *micro_sym, struct CFCDocuComment *docucomment,
                const char *source_class, const char *parent_class_name,
                int is_final, int is_inert);

CFCClass*
CFCClass_do_create(CFCClass *self, struct CFCParcel *parcel,
                   const char *exposure, const char *class_name,
                   const char *cnick, const char *micro_sym,
                   struct CFCDocuComment *docucomment,
                   const char *source_class, const char *parent_class_name,
                   int is_final, int is_inert);

void
CFCClass_destroy(CFCClass *self);

CFCClass*
CFCClass_fetch_singleton(struct CFCParcel *parcel, const char *class_name);

void
CFCClass_register(CFCClass *self);

void
CFCClass_clear_registry(void);

void
CFCClass_add_child(CFCClass *self, CFCClass *child);

void
CFCClass_add_function(CFCClass *self, struct CFCFunction *func);

void
CFCClass_add_method(CFCClass *self, struct CFCMethod *method);

void
CFCClass_add_member_var(CFCClass *self, struct CFCVariable *var);

void
CFCClass_add_inert_var(CFCClass *self, struct CFCVariable *var);

void
CFCClass_add_attribute(CFCClass *self, const char *name, const char *value);

int
CFCClass_has_attribute(CFCClass *self, const char *name);

struct CFCFunction*
CFCClass_function(CFCClass *self, const char *sym);

struct CFCMethod*
CFCClass_method(CFCClass *self, const char *sym);

struct CFCMethod*
CFCClass_novel_method(CFCClass *self, const char *sym);

void
CFCClass_grow_tree(CFCClass *self);

CFCClass**
CFCClass_tree_to_ladder(CFCClass *self);

struct CFCMethod**
CFCClass_novel_methods(CFCClass *self);

struct CFCVariable**
CFCClass_novel_member_vars(CFCClass *self);

CFCClass**
CFCClass_children(CFCClass *self);

struct CFCFunction**
CFCClass_functions(CFCClass *self);

struct CFCMethod**
CFCClass_methods(CFCClass *self);

struct CFCVariable**
CFCClass_member_vars(CFCClass *self);

struct CFCVariable**
CFCClass_inert_vars(CFCClass *self);

const char*
CFCClass_get_cnick(CFCClass *self);

void
CFCClass_set_parent(CFCClass *self, CFCClass *parent);

CFCClass*
CFCClass_get_parent(CFCClass *self);

void
CFCClass_append_autocode(CFCClass *self, const char *autocode);

const char*
CFCClass_get_autocode(CFCClass *self);

const char*
CFCClass_get_source_class(CFCClass *self);

const char*
CFCClass_get_parent_class_name(CFCClass *self);

int
CFCClass_final(CFCClass *self);

int
CFCClass_inert(CFCClass *self);

const char*
CFCClass_get_struct_sym(CFCClass *self);

const char*
CFCClass_full_struct_sym(CFCClass *self);

const char*
CFCClass_short_vtable_var(CFCClass *self);

const char*
CFCClass_full_vtable_var(CFCClass *self);

const char*
CFCClass_full_vtable_type(CFCClass *self);

const char*
CFCClass_include_h(CFCClass *self);

struct CFCDocuComment*
CFCClass_get_docucomment(CFCClass *self);

const char*
CFCClass_get_prefix(CFCClass *self);

const char*
CFCClass_get_Prefix(CFCClass *self);

const char*
CFCClass_get_PREFIX(CFCClass *self);

const char*
CFCClass_get_class_name(CFCClass *self);

struct CFCParcel*
CFCClass_get_parcel(CFCClass *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCCLASS */

