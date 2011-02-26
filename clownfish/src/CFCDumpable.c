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
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCDumpable.h"
#include "CFCClass.h"
#include "CFCMethod.h"
#include "CFCParamList.h"
#include "CFCParcel.h"
#include "CFCSymbol.h"
#include "CFCType.h"
#include "CFCVariable.h"
#include "CFCUtil.h"

struct CFCDumpable {
    CFCBase base;
};

CFCDumpable*
CFCDumpable_new(void)
{
    CFCDumpable *self = (CFCDumpable*)CFCBase_allocate(sizeof(CFCDumpable),
        "Clownfish::Dumpable");
    return CFCDumpable_init(self);
}

CFCDumpable*
CFCDumpable_init(CFCDumpable *self)
{
    return self;
}

void
CFCDumpable_destroy(CFCDumpable *self)
{
    CFCBase_destroy((CFCBase*)self);
}

CFCMethod*
CFCDumpable_make_method_obj(CFCDumpable *self, CFCClass *klass, 
                            const char *method_name)
{
    const char *klass_struct_sym = CFCClass_get_struct_sym(klass);
    const char *klass_name   = CFCSymbol_get_class_name((CFCSymbol*)klass);
    const char *klass_cnick  = CFCSymbol_get_class_cnick((CFCSymbol*)klass);
    CFCParcel  *klass_parcel = CFCSymbol_get_parcel((CFCSymbol*)klass);
    CFCParcel  *cf_parcel    = CFCParcel_clownfish_parcel();

    CFCType *return_type 
        = CFCType_new_object(CFCTYPE_INCREMENTED, cf_parcel, "Obj", 1);
    CFCType *self_type = CFCType_new_object(0, klass_parcel, klass_struct_sym, 1);
    CFCVariable *self_var = CFCVariable_new(klass_parcel, NULL, klass_name,
        klass_cnick, "self", self_type);
    CFCParamList *param_list = NULL;

    if (strcmp(method_name, "Dump") == 0) {
        param_list = CFCParamList_new(false);
        CFCParamList_add_param(param_list, self_var, NULL);
    }
    else if (strcmp(method_name, "Load") == 0) {
        CFCType *dump_type = CFCType_new_object(0, cf_parcel, "Obj", 1);
        CFCVariable *dump_var = CFCVariable_new(cf_parcel, NULL, NULL,
            NULL, "dump", dump_type);
        param_list = CFCParamList_new(false);
        CFCParamList_add_param(param_list, self_var, NULL);
        CFCParamList_add_param(param_list, dump_var, NULL);
        CFCBase_decref((CFCBase*)dump_var);
        CFCBase_decref((CFCBase*)dump_type);
    }
    else {
        croak("Unexpected method_name: '%s'", method_name);
    }

    CFCMethod *method = CFCMethod_new(klass_parcel, "public", klass_name,
        klass_cnick, method_name, return_type, param_list, NULL, false, 
        false);

    CFCBase_decref((CFCBase*)param_list);
    CFCBase_decref((CFCBase*)self_type);
    CFCBase_decref((CFCBase*)self_var);
    CFCBase_decref((CFCBase*)return_type);

    return method;
}

void
CFCDumpable_process_dump_member(CFCVariable *member, char *buf, 
                                size_t buf_size)
{
    CFCUTIL_NULL_CHECK(member);
    CFCType *type = CFCVariable_get_type(member);
    const char *name = CFCSymbol_micro_sym((CFCSymbol*)member);
    unsigned name_len = (unsigned)strlen(name);

    if (CFCType_is_integer(type) || CFCType_is_floating(type)) {
        char int_template[] = 
            "    Cfish_Hash_Store_Str(dump, \"%s\", %u, (cfish_Obj*)cfish_CB_newf(\"%%i64\", (int64_t)self->%s));\n";
        char float_template[] = 
            "    Cfish_Hash_Store_Str(dump, \"%s\", %u, (cfish_Obj*)cfish_CB_newf(\"%%f64\", (double)self->%s));\n";
        const char *template = CFCType_is_integer(type) 
                             ? int_template : float_template;
        size_t needed = strlen(template) + name_len * 2 + 20;
        if (buf_size < needed) {
            croak("Buffer not big enough (%lu < %lu)", 
                (unsigned long)buf_size, (unsigned long)needed);
        }
        int check = sprintf(buf, template, name, name_len, name);
        if (check < 0) { croak("sprintf failed"); }
        return;
    }
    else if (CFCType_is_object(type)) {
        char template[] = 
            "    if (self->%s) {\n"
            "        Cfish_Hash_Store_Str(dump, \"%s\", %u, Cfish_Obj_Dump((cfish_Obj*)self->%s));\n"
            "    }\n";

        size_t needed = strlen(template) + name_len * 3 + 20;
        if (buf_size < needed) {
            croak("Buffer not big enough (%lu < %lu)", 
                (unsigned long)buf_size, (unsigned long)needed);
        }
        int check = sprintf(buf, template, name, name, name_len, name);
        if (check < 0) { croak("sprintf failed"); }
        return;
    }
    else {
        croak("Don't know how to dump a %s", CFCType_get_specifier(type));
    }
}

void
CFCDumpable_process_load_member(CFCVariable *member, char *buf, 
                                size_t buf_size)
{
    CFCUTIL_NULL_CHECK(member);
    CFCType *type = CFCVariable_get_type(member);
    const char *type_str = CFCType_to_c(type);
    const char *name = CFCSymbol_micro_sym((CFCSymbol*)member);
    unsigned name_len = (unsigned)strlen(name);
    char extraction[200];
    if (strlen(type_str) + 100 > sizeof(extraction)) { // play it safe
        croak("type_str too long: '%s'", type_str);
    }
    if (CFCType_is_integer(type)) {
        int check = sprintf(extraction, "(%s)Cfish_Obj_To_I64(var)", type_str);
        if (check < 0) { croak("sprintf failed"); }
    }
    else if (CFCType_is_floating(type)) {
        int check = sprintf(extraction, "(%s)Cfish_Obj_To_F64(var)", type_str);
        if (check < 0) { croak("sprintf failed"); }
    }
    else if (CFCType_is_object(type)) {
        const char *specifier = CFCType_get_specifier(type);
        size_t specifier_len = strlen(specifier);
        char vtable_var[50];
        if (specifier_len > sizeof(vtable_var) - 2) {
            croak("specifier too long: '%s'", specifier);
        }
        size_t i;
        for (i = 0; i <= specifier_len; i++) {
            vtable_var[i] = toupper(specifier[i]);
        }
        int check = sprintf(extraction, 
             "(%s*)CFISH_CERTIFY(Cfish_Obj_Load(var, var), %s)",
             specifier, vtable_var);
        if (check < 0) { croak("sprintf failed"); }
    }
    else {
        croak("Don't know how to load %s", CFCType_get_specifier(type));
    }
    
    const char *template = 
    "    {\n"
    "        cfish_Obj *var = Cfish_Hash_Fetch_Str(source, \"%s\", %u);\n"
    "        if (var) { loaded->%s = %s; }\n"
    "    }\n";
    size_t needed = sizeof(template) 
                  + (name_len * 2) 
                  + strlen(extraction) 
                  + 20;
    if (buf_size < needed) {
        croak("Buffer not big enough (%lu < %lu)", (unsigned long)buf_size,
            (unsigned long)needed);
    }
    int check = sprintf(buf, template, name, name_len, name, extraction);
    if (check < 0) { croak("sprintf failed"); }
}


