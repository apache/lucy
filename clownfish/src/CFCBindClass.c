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

#define CFC_NEED_BASE_STRUCT_DEF

#include <stdio.h>
#include <string.h>
#include "CFCBindClass.h"
#include "CFCBase.h"
#include "CFCClass.h"
#include "CFCFunction.h"
#include "CFCMethod.h"
#include "CFCParamList.h"
#include "CFCType.h"
#include "CFCVariable.h"
#include "CFCUtil.h"

struct CFCBindClass {
    CFCBase base;
    CFCClass *client;
    char *full_callbacks_var;
    char *full_name_var;
    char *short_names_macro;
};

CFCBindClass*
CFCBindClass_new(CFCClass *client) {
    CFCBindClass *self 
        = (CFCBindClass*)CFCBase_allocate(sizeof(CFCBindClass),
                                          "Clownfish::Binding::Core::Class");
    return CFCBindClass_init(self, client);
}

CFCBindClass*
CFCBindClass_init(CFCBindClass *self, CFCClass *client) {
    CFCUTIL_NULL_CHECK(client);
    self->client = (CFCClass*)CFCBase_incref((CFCBase*)client);

    const char *full_vtable_var = CFCClass_full_vtable_var(client);
    const char *PREFIX = CFCClass_get_PREFIX(client);
    self->full_callbacks_var = (char*)MALLOCATE(strlen(full_vtable_var) + 20);
    self->full_name_var      = (char*)MALLOCATE(strlen(full_vtable_var) + 20);
    self->short_names_macro  = (char*)MALLOCATE(strlen(PREFIX) + 20);
    sprintf(self->full_callbacks_var, "%s_CALLBACKS", full_vtable_var);
    sprintf(self->full_name_var, "%s_CLASS_NAME", full_vtable_var);
    sprintf(self->short_names_macro, "%sUSE_SHORT_NAMES", PREFIX);

    return self;
}

void
CFCBindClass_destroy(CFCBindClass *self)
{
    FREEMEM(self->full_callbacks_var);
    FREEMEM(self->full_name_var);
    FREEMEM(self->short_names_macro);
    CFCBase_decref((CFCBase*)self->client);
    CFCBase_destroy((CFCBase*)self);
}

// Create the definition for the instantiable object struct.
char*
CFCBindClass_struct_definition(CFCBindClass *self) {
    const char *struct_sym = CFCClass_full_struct_sym(self->client);
    CFCVariable **member_vars = CFCClass_member_vars(self->client);
    char *member_decs = CFCUtil_strdup("");
    
    for (int i = 0; member_vars[i] != NULL; i++) {
        const char *member_dec = CFCVariable_local_declaration(member_vars[i]);
        size_t needed = strlen(member_decs) + strlen(member_dec) + 10;
        member_decs = (char*)REALLOCATE(member_decs, needed);
        strcat(member_decs, "\n    ");
        strcat(member_decs, member_dec);
    }

    char pattern[] = "struct %s {%s\n};\n";
    size_t size = sizeof(pattern)
                  + strlen(struct_sym)
                  + strlen(member_decs)
                  + 10;
    char *struct_def = (char*)MALLOCATE(size);
    sprintf(struct_def, pattern, struct_sym, member_decs);

    FREEMEM(member_decs);
    return struct_def;
}

/* C code defining the ZombieCharBuf which contains the class name for this
 * class. */
char*
CFCBindClass_name_var_definition(CFCBindClass *self) {
    const char *class_name  = CFCClass_get_class_name(self->client);
    unsigned class_name_len = (unsigned)strlen(class_name);

    const char pattern[] = 
        "cfish_ZombieCharBuf %s = {\n"
        "    CFISH_ZOMBIECHARBUF,\n"
        "    {1}, /* ref.count */\n"
        "    \"%s\",\n"
        "    %u,\n"
        "    0\n"
        "};\n\n";
    size_t size = sizeof(pattern)
                  + strlen(self->full_name_var)
                  + class_name_len
                  + 15 // class_name_len
                  + 20;
    char *name_var_def = (char*)MALLOCATE(size);
    sprintf(name_var_def, pattern, self->full_name_var, class_name,
            class_name_len);

    return name_var_def;
}

// Return C code defining the class's VTable.
char*
CFCBindClass_vtable_definition(CFCBindClass *self) {
    CFCClass    *client     = self->client;
    CFCClass    *parent     = CFCClass_get_parent(client);
    CFCMethod  **methods    = CFCClass_methods(client);
    const char  *vt_type    = CFCClass_full_vtable_type(client);
    const char  *vt_var     = CFCClass_full_vtable_var(client);
    const char  *struct_sym = CFCClass_full_struct_sym(client);
    
    // Create a pointer to the parent class's vtable.
    const char *parent_ref = parent
                             ? CFCClass_full_vtable_var(parent)
                             : "NULL"; // No parent, e.g. Obj or inert classes.

    // Spec functions which implement the methods, casting to quiet compiler.
    char *method_str = CFCUtil_strdup("");
    int num_methods = 0;
    for (; methods[num_methods] != NULL; num_methods++) {
        CFCMethod *method = methods[num_methods];
        const char *implementing_sym = CFCMethod_implementing_func_sym(method);
        size_t size = strlen(method_str) + strlen(implementing_sym) + 50;
        method_str = (char*)REALLOCATE(method_str, size);
        if (strlen(method_str)) {
            strcat(method_str, ",\n");
        }
        strcat(method_str, "        (cfish_method_t)");
        strcat(method_str, implementing_sym);
    }

    char pattern[] =
        "\n"
        "%s %s_vt = {\n"
        "    CFISH_VTABLE, /* vtable vtable */\n"
        "    {1}, /* ref.count */\n"
        "    %s, /* parent */\n"
        "    (cfish_CharBuf*)&%s,\n"
        "    0, /* flags */\n"
        "    NULL, /* \"void *x\" member reserved for future use */\n"
        "    sizeof(%s), /* obj_alloc_size */\n"
        "    offsetof(cfish_VTable, methods)\n"
        "        + %d * sizeof(cfish_method_t), /* vt_alloc_size */\n"
        "    &%s,  /* callbacks */\n"
        "    {\n"
        "%s\n"
        "    }\n"
        "};\n\n";

    size_t size = sizeof(pattern)
                  + strlen(vt_type)
                  + strlen(vt_var)
                  + strlen(parent_ref)
                  + strlen(self->full_name_var)
                  + strlen(struct_sym)
                  + 10 /* num_methods */
                  + strlen(self->full_callbacks_var)
                  + strlen(method_str)
                  + 40;
    char *vtable_def = (char*)MALLOCATE(size);
    sprintf(vtable_def, pattern, vt_type, vt_var, parent_ref,
            self->full_name_var, struct_sym, num_methods,
            self->full_callbacks_var, method_str);

    FREEMEM(method_str);
    return vtable_def;
}

CFCClass*
CFCBindClass_get_client(CFCBindClass *self) {
    return self->client;
}

const char*
CFCBindClass_full_callbacks_var(CFCBindClass *self)
{
    return self->full_callbacks_var;
}

const char*
CFCBindClass_full_name_var(CFCBindClass *self)
{
    return self->full_name_var;
}

const char*
CFCBindClass_short_names_macro(CFCBindClass *self)
{
    return self->short_names_macro;
}

