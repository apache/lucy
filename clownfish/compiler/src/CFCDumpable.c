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

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCDumpable.h"
#include "CFCClass.h"
#include "CFCFunction.h"
#include "CFCMethod.h"
#include "CFCParamList.h"
#include "CFCParcel.h"
#include "CFCSymbol.h"
#include "CFCType.h"
#include "CFCVariable.h"
#include "CFCUtil.h"

// Add an autogenerated Dump method to the CFCClass.
static void
S_add_dump_method(CFCClass *klass);

// Add an autogenerated Load method to the CFCClass.
static void
S_add_load_method(CFCClass *klass);

// Create a Clownfish::CFC::Model::Method object for either Dump() or Load().
static CFCMethod*
S_make_method_obj(CFCClass *klass, const char *method_name);

// Generate code for dumping a single member var.
static void
S_process_dump_member(CFCClass *klass, CFCVariable *member, char *buf,
                      size_t buf_size);

// Generate code for loading a single member var.
static void
S_process_load_member(CFCClass *klass, CFCVariable *member, char *buf,
                      size_t buf_size);

struct CFCDumpable {
    CFCBase base;
};

const static CFCMeta CFCDUMPABLE_META = {
    "Clownfish::CFC::Dumpable",
    sizeof(CFCDumpable),
    (CFCBase_destroy_t)CFCDumpable_destroy
};

CFCDumpable*
CFCDumpable_new(void) {
    CFCDumpable *self = (CFCDumpable*)CFCBase_allocate(&CFCDUMPABLE_META);
    return CFCDumpable_init(self);
}

CFCDumpable*
CFCDumpable_init(CFCDumpable *self) {
    return self;
}

void
CFCDumpable_destroy(CFCDumpable *self) {
    CFCBase_destroy((CFCBase*)self);
}

void
CFCDumpable_add_dumpables(CFCDumpable *self, CFCClass *klass) {
    (void)self;

    if (!CFCClass_has_attribute(klass, "dumpable")) {
        CFCUtil_die("Class %s isn't dumpable", CFCClass_get_class_name(klass));
    }

    // Inherit Dump/Load from parent if no fresh member vars.
    CFCClass *parent = CFCClass_get_parent(klass);
    if (parent && CFCClass_has_attribute(parent, "dumpable")) {
        CFCVariable **fresh = CFCClass_fresh_member_vars(klass);
        int needs_autogenerated_dumpables = fresh[0] != NULL ? true : false;
        FREEMEM(fresh);
        if (!needs_autogenerated_dumpables) { return; }
    }

    if (!CFCClass_fresh_method(klass, "Dump")) {
        S_add_dump_method(klass);
    }
    if (!CFCClass_fresh_method(klass, "Load")) {
        S_add_load_method(klass);
    }
}

static CFCMethod*
S_make_method_obj(CFCClass *klass, const char *method_name) {
    const char *klass_struct_sym = CFCClass_get_struct_sym(klass);
    const char *klass_name   = CFCClass_get_class_name(klass);
    const char *klass_cnick  = CFCClass_get_cnick(klass);
    CFCParcel  *klass_parcel = CFCClass_get_parcel(klass);
    CFCParcel  *cf_parcel    = CFCParcel_clownfish_parcel();

    CFCType *return_type
        = CFCType_new_object(CFCTYPE_INCREMENTED, cf_parcel, "Obj", 1);
    CFCType *self_type = CFCType_new_object(0, klass_parcel, klass_struct_sym, 1);
    CFCVariable *self_var = CFCVariable_new(NULL, NULL, NULL, NULL, "self",
                                            self_type, false);
    CFCParamList *param_list = NULL;

    if (strcmp(method_name, "Dump") == 0) {
        param_list = CFCParamList_new(false);
        CFCParamList_add_param(param_list, self_var, NULL);
    }
    else if (strcmp(method_name, "Load") == 0) {
        CFCType *dump_type = CFCType_new_object(0, cf_parcel, "Obj", 1);
        CFCVariable *dump_var = CFCVariable_new(NULL, NULL, NULL, NULL, "dump",
                                                dump_type, false);
        param_list = CFCParamList_new(false);
        CFCParamList_add_param(param_list, self_var, NULL);
        CFCParamList_add_param(param_list, dump_var, NULL);
        CFCBase_decref((CFCBase*)dump_var);
        CFCBase_decref((CFCBase*)dump_type);
    }
    else {
        CFCUtil_die("Unexpected method_name: '%s'", method_name);
    }

    CFCMethod *method = CFCMethod_new(klass_parcel, "public", klass_name,
                                      klass_cnick, method_name, return_type,
                                      param_list, NULL, false, false);

    CFCBase_decref((CFCBase*)param_list);
    CFCBase_decref((CFCBase*)self_type);
    CFCBase_decref((CFCBase*)self_var);
    CFCBase_decref((CFCBase*)return_type);

    return method;
}

static void
S_add_dump_method(CFCClass *klass) {
    CFCMethod *method = S_make_method_obj(klass, "Dump");
    CFCClass_add_method(klass, method);
    CFCBase_decref((CFCBase*)method);
    const char *full_func_sym = CFCMethod_implementing_func_sym(method);
    const char *full_struct   = CFCClass_full_struct_sym(klass);
    const char *vtable_var    = CFCClass_full_vtable_var(klass);
    CFCClass   *parent        = CFCClass_get_parent(klass);
    const size_t BUF_SIZE = 400;
    char buf[BUF_SIZE];

    char *full_typedef = CFCMethod_full_typedef(method, klass);
    char *full_meth    = CFCMethod_full_method_sym(method, klass);

    if (parent && CFCClass_has_attribute(parent, "dumpable")) {
        const char pattern[] =
            "cfish_Obj*\n"
            "%s(%s *self)\n"
            "{\n"
            "    %s super_dump = SUPER_METHOD_PTR(%s, %s);\n"
            "    cfish_Hash *dump = (cfish_Hash*)super_dump(self);\n";
        char *autocode
            = CFCUtil_sprintf(pattern, full_func_sym, full_struct,
                              full_typedef, vtable_var, full_meth);
        CFCClass_append_autocode(klass, autocode);
        FREEMEM(full_meth);
        FREEMEM(full_typedef);
        FREEMEM(autocode);

        CFCVariable **fresh = CFCClass_fresh_member_vars(klass);
        for (size_t i = 0; fresh[i] != NULL; i++) {
            S_process_dump_member(klass, fresh[i], buf, BUF_SIZE);
        }
        FREEMEM(fresh);
    }
    else {
        const char pattern[] =
            "cfish_Obj*\n"
            "%s(%s *self)\n"
            "{\n"
            "    cfish_Hash *dump = cfish_Hash_new(0);\n"
            "    Cfish_Hash_Store_Str(dump, \"_class\", 6,\n"
            "        (cfish_Obj*)Cfish_CB_Clone(Cfish_Obj_Get_Class_Name((cfish_Obj*)self)));\n";
        char *autocode = CFCUtil_sprintf(pattern, full_func_sym, full_struct);
        CFCClass_append_autocode(klass, autocode);
        FREEMEM(autocode);
        CFCVariable **members = CFCClass_member_vars(klass);
        for (size_t i = 0; members[i] != NULL; i++) {
            S_process_dump_member(klass, members[i], buf, BUF_SIZE);
        }
    }

    CFCClass_append_autocode(klass, "    return (cfish_Obj*)dump;\n}\n\n");
}

static void
S_add_load_method(CFCClass *klass) {
    CFCMethod *method = S_make_method_obj(klass, "Load");
    CFCClass_add_method(klass, method);
    CFCBase_decref((CFCBase*)method);
    const char *full_func_sym = CFCMethod_implementing_func_sym(method);
    const char *full_struct   = CFCClass_full_struct_sym(klass);
    const char *vtable_var    = CFCClass_full_vtable_var(klass);
    CFCClass   *parent        = CFCClass_get_parent(klass);
    const size_t BUF_SIZE = 400;
    char buf[BUF_SIZE];

    char *full_typedef = CFCMethod_full_typedef(method, klass);
    char *full_meth    = CFCMethod_full_method_sym(method, klass);

    if (parent && CFCClass_has_attribute(parent, "dumpable")) {
        const char pattern[] =
            "cfish_Obj*\n"
            "%s(%s *self, cfish_Obj *dump)\n"
            "{\n"
            "    cfish_Hash *source = (cfish_Hash*)CFISH_CERTIFY(dump, CFISH_HASH);\n"
            "    %s super_load = SUPER_METHOD_PTR(%s, %s);\n"
            "    %s *loaded = (%s*)super_load(self, dump);\n";
        char *autocode
            = CFCUtil_sprintf(pattern, full_func_sym, full_struct,
                              full_typedef, vtable_var, full_meth, full_struct,
                              full_struct);
        CFCClass_append_autocode(klass, autocode);
        FREEMEM(full_meth);
        FREEMEM(full_typedef);
        FREEMEM(autocode);

        CFCVariable **fresh = CFCClass_fresh_member_vars(klass);
        for (size_t i = 0; fresh[i] != NULL; i++) {
            S_process_load_member(klass, fresh[i], buf, BUF_SIZE);
        }
        FREEMEM(fresh);
    }
    else {
        const char pattern[] =
            "cfish_Obj*\n"
            "%s(%s *self, cfish_Obj *dump)\n"
            "{\n"
            "    cfish_Hash *source = (cfish_Hash*)CFISH_CERTIFY(dump, CFISH_HASH);\n"
            "    cfish_CharBuf *class_name = (cfish_CharBuf*)CFISH_CERTIFY(\n"
            "        Cfish_Hash_Fetch_Str(source, \"_class\", 6), CFISH_CHARBUF);\n"
            "    cfish_VTable *vtable = cfish_VTable_singleton(class_name, NULL);\n"
            "    %s *loaded = (%s*)Cfish_VTable_Make_Obj(vtable);\n"
            "    CHY_UNUSED_VAR(self);\n";
        char *autocode
            = CFCUtil_sprintf(pattern, full_func_sym, full_struct, full_struct,
                              full_struct);
        CFCClass_append_autocode(klass, autocode);
        FREEMEM(autocode);
        CFCVariable **members = CFCClass_member_vars(klass);
        for (size_t i = 0; members[i] != NULL; i++) {
            S_process_load_member(klass, members[i], buf, BUF_SIZE);
        }
    }

    CFCClass_append_autocode(klass, "    return (cfish_Obj*)loaded;\n}\n\n");
}

static void
S_process_dump_member(CFCClass *klass, CFCVariable *member, char *buf,
                      size_t buf_size) {
    CFCUTIL_NULL_CHECK(member);
    CFCType *type = CFCVariable_get_type(member);
    const char *name = CFCVariable_micro_sym(member);
    unsigned name_len = (unsigned)strlen(name);
    const char *specifier = CFCType_get_specifier(type);

    // Skip the VTable and the refcount/host-object.
    if (strcmp(specifier, "lucy_VTable") == 0
        || strcmp(specifier, "lucy_ref_t") == 0
       ) {
        return;
    }

    if (CFCType_is_integer(type) || CFCType_is_floating(type)) {
        char int_pattern[] =
            "    Cfish_Hash_Store_Str(dump, \"%s\", %u, (cfish_Obj*)cfish_CB_newf(\"%%i64\", (int64_t)self->%s));\n";
        char float_pattern[] =
            "    Cfish_Hash_Store_Str(dump, \"%s\", %u, (cfish_Obj*)cfish_CB_newf(\"%%f64\", (double)self->%s));\n";
        char bool_pattern[] =
            "    Cfish_Hash_Store_Str(dump, \"%s\", %u, (cfish_Obj*)cfish_Bool_singleton(self->%s));\n";
        const char *pattern;
        if (strcmp(specifier, "bool") == 0) {
            pattern = bool_pattern;
        }
        else if (CFCType_is_integer(type)) {
            pattern = int_pattern;
        }
        else {
            pattern = float_pattern;
        }
        size_t needed = strlen(pattern) + name_len * 2 + 20;
        if (buf_size < needed) {
            CFCUtil_die("Buffer not big enough (%lu < %lu)",
                        (unsigned long)buf_size, (unsigned long)needed);
        }
        sprintf(buf, pattern, name, name_len, name);
    }
    else if (CFCType_is_object(type)) {
        char pattern[] =
            "    if (self->%s) {\n"
            "        Cfish_Hash_Store_Str(dump, \"%s\", %u, Cfish_Obj_Dump((cfish_Obj*)self->%s));\n"
            "    }\n";

        size_t needed = strlen(pattern) + name_len * 3 + 20;
        if (buf_size < needed) {
            CFCUtil_die("Buffer not big enough (%lu < %lu)",
                        (unsigned long)buf_size, (unsigned long)needed);
        }
        sprintf(buf, pattern, name, name, name_len, name);
    }
    else {
        CFCUtil_die("Don't know how to dump a %s",
                    CFCType_get_specifier(type));
    }

    CFCClass_append_autocode(klass, buf);
}

static void
S_process_load_member(CFCClass *klass, CFCVariable *member, char *buf,
                      size_t buf_size) {
    CFCUTIL_NULL_CHECK(member);
    CFCType *type = CFCVariable_get_type(member);
    const char *type_str = CFCType_to_c(type);
    const char *name = CFCVariable_micro_sym(member);
    unsigned name_len = (unsigned)strlen(name);
    char extraction[200];
    const char *specifier = CFCType_get_specifier(type);

    // Skip the VTable and the refcount/host-object.
    if (strcmp(specifier, "lucy_VTable") == 0
        || strcmp(specifier, "lucy_ref_t") == 0
       ) {
        return;
    }

    if (2 * strlen(type_str) + 100 > sizeof(extraction)) { // play it safe
        CFCUtil_die("type_str too long: '%s'", type_str);
    }
    if (CFCType_is_integer(type)) {
        if (strcmp(specifier, "bool") == 0) {
            sprintf(extraction, "Cfish_Obj_To_Bool(var)");
        }
        else {
            sprintf(extraction, "(%s)Cfish_Obj_To_I64(var)", type_str);
        }
    }
    else if (CFCType_is_floating(type)) {
        sprintf(extraction, "(%s)Cfish_Obj_To_F64(var)", type_str);
    }
    else if (CFCType_is_object(type)) {
        const char *vtable_var = CFCType_get_vtable_var(type);
        sprintf(extraction,
                "(%s*)CFISH_CERTIFY(Cfish_Obj_Load(var, var), %s)",
                specifier, vtable_var);
    }
    else {
        CFCUtil_die("Don't know how to load %s", specifier);
    }

    const char *pattern =
        "    {\n"
        "        cfish_Obj *var = Cfish_Hash_Fetch_Str(source, \"%s\", %u);\n"
        "        if (var) { loaded->%s = %s; }\n"
        "    }\n";
    size_t needed = sizeof(pattern)
                    + (name_len * 2)
                    + strlen(extraction)
                    + 20;
    if (buf_size < needed) {
        CFCUtil_die("Buffer not big enough (%lu < %lu)",
                    (unsigned long)buf_size, (unsigned long)needed);
    }
    sprintf(buf, pattern, name, name_len, name, extraction);

    CFCClass_append_autocode(klass, buf);
}

