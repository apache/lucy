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

typedef struct CFCClass CFCClass;
struct CFCParcel;
struct CFCDocuComment;
struct CFCFunction;
struct CFCVariable;

CFCClass*
CFCClass_new(struct CFCParcel *parcel, const char *exposure, 
              const char *class_name, const char *class_cnick, 
              const char *micro_sym, struct CFCDocuComment *docucomment,
              const char *source_class, const char *parent_class_name, 
              int is_final, int is_inert);

CFCClass*
CFCClass_init(CFCClass *self, struct CFCParcel *parcel, 
               const char *exposure, const char *class_name, 
               const char *class_cnick, const char *micro_sym,
               struct CFCDocuComment *docucomment, 
               const char *source_class, const char *parent_class_name, 
               int is_final, int is_inert);

void
CFCClass_destroy(CFCClass *self);

void
CFCClass_add_child(CFCClass *self, CFCClass *child);

void
CFCClass_add_function(CFCClass *self, struct CFCFunction *func);

void
CFCClass_add_member_var(CFCClass *self, struct CFCVariable *var);

void
CFCClass_add_inert_var(CFCClass *self, struct CFCVariable *var);

struct CFCFunction*
CFCClass_function(CFCClass *self, const char *sym);

/** Pass down member vars to from parent to children.
 */
void
CFCClass_bequeath_member_vars(CFCClass *self);

CFCClass**
CFCClass_children(CFCClass *self);

struct CFCFunction**
CFCClass_functions(CFCClass *self);

struct CFCVariable**
CFCClass_member_vars(CFCClass *self);

struct CFCVariable**
CFCClass_inert_vars(CFCClass *self);

const char*
CFCClass_get_cnick(CFCClass *self);

void
CFCClass_set_tree_grown(CFCClass *self, int tree_grown);

int
CFCClass_tree_grown(CFCClass *self);

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

#endif /* H_CFCCLASS */

