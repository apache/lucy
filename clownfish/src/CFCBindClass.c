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
#include "CFCBindFunction.h"
#include "CFCBindMethod.h"
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

// Generate C header for an inert class.
static char*
S_to_c_header_inert(CFCBindClass *self);

// Generate C header for a dynamic class.
static char*
S_to_c_header_dynamic(CFCBindClass *self);

// Create the definition for the instantiable object struct.
static char*
S_struct_definition(CFCBindClass *self);

/* C code defining the ZombieCharBuf which contains the class name for this
 * class.
 */
static char*
S_name_var_definition(CFCBindClass *self);

// Return C code defining the class's VTable.
static char*
S_vtable_definition(CFCBindClass *self);

// Declare cfish_Callback objects.
static char*
S_callback_declarations(CFCBindClass *self);

// Declare typedefs for fresh methods, to ease casting.
static char*
S_method_typedefs(CFCBindClass *self);

// If class inherits from something, include the parent class's header.
static char*
S_parent_include(CFCBindClass *self);

// Add a C function definition for each method and each function.
static char*
S_sub_declarations(CFCBindClass *self);

// Declare class (a.k.a. "inert") variables.
static char*
S_inert_var_declarations(CFCBindClass *self);

// Define method invocation inline functions.
static char*
S_method_defs(CFCBindClass *self);

// Define the virtual table singleton object.
static char*
S_vt_singleton_def(CFCBindClass *self);

// Define short names for all of the symbols associated with this class.
static char*
S_short_names(CFCBindClass *self);

const static CFCMeta CFCBINDCLASS_META = {
    "Clownfish::CFC::Binding::Core::Class",
    sizeof(CFCBindClass),
    (CFCBase_destroy_t)CFCBindClass_destroy
};

CFCBindClass*
CFCBindClass_new(CFCClass *client) {
    CFCBindClass *self = (CFCBindClass*)CFCBase_allocate(&CFCBINDCLASS_META);
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
CFCBindClass_destroy(CFCBindClass *self) {
    FREEMEM(self->full_callbacks_var);
    FREEMEM(self->full_name_var);
    FREEMEM(self->short_names_macro);
    CFCBase_decref((CFCBase*)self->client);
    CFCBase_destroy((CFCBase*)self);
}

static int
S_method_is_fresh(CFCMethod *method, CFCMethod **fresh_methods) {
    for (int i = 0; fresh_methods[i] != NULL; i++) {
        if (method == fresh_methods[i]) { return 1; }
    }
    return 0;
}

char*
CFCBindClass_to_c_header(CFCBindClass *self) {
    if (CFCClass_inert(self->client)) {
        // Inert classes only output inert functions and vars.
        return S_to_c_header_inert(self);
    }
    else {
        return S_to_c_header_dynamic(self);
    }
}

static char*
S_to_c_header_inert(CFCBindClass *self) {
    char *inert_func_decs = S_sub_declarations(self);
    char *inert_var_defs  = S_inert_var_declarations(self);
    char *short_names     = S_short_names(self);

    char pattern[] =
        "#include \"charmony.h\"\n"
        "#include \"parcel.h\"\n"
        "\n"
        "/* Declare this class's inert variables.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Declare this class's inert functions.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define \"short names\" for this class's symbols.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n";

    size_t size = sizeof(pattern)
                  + strlen(inert_var_defs)
                  + strlen(inert_func_decs)
                  + strlen(short_names)
                  + 50;
    char *content = (char*)MALLOCATE(size);
    sprintf(content, pattern, inert_var_defs, inert_func_decs, short_names);

    FREEMEM(inert_var_defs);
    FREEMEM(inert_func_decs);
    FREEMEM(short_names);
    return content;
}

static char*
S_to_c_header_dynamic(CFCBindClass *self) {
    const char *privacy_symbol  = CFCClass_privacy_symbol(self->client);
    char *struct_def            = S_struct_definition(self);
    char *parent_include        = S_parent_include(self);
    char *sub_declarations      = S_sub_declarations(self);
    char *inert_var_defs        = S_inert_var_declarations(self);
    char *method_typedefs       = S_method_typedefs(self);
    char *method_defs           = S_method_defs(self);
    char *vt_singleton_def      = S_vt_singleton_def(self);
    char *callback_declarations = S_callback_declarations(self);
    char *short_names           = S_short_names(self);

    char pattern[] =
        "#include \"charmony.h\"\n"
        "#include \"parcel.h\"\n"
        "\n"
        "/* Include the header for this class's parent. \n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define the struct layout for instances of this class.\n"
        " */\n"
        "\n"
        "#ifdef %s\n"
        "%s\n"
        "#endif /* %s */\n"
        "\n"
        "/* Declare this class's inert variables.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Declare both this class's inert functions and the C functions which\n"
        " * implement this class's dynamic methods.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Declare the cfish_Callback objects which provide the introspection\n"
        " * information needed to perform method overriding at runtime.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define typedefs for each dynamic method, allowing us to cast generic\n"
        " * pointers to the appropriate function pointer type more cleanly.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define the inline functions which implement this class's virtual methods.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define the VTable singleton for this class.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define \"short names\" for this class's symbols.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n";

    size_t size = sizeof(pattern)
                  + strlen(parent_include)
                  + strlen(privacy_symbol)
                  + strlen(struct_def)
                  + strlen(privacy_symbol)
                  + strlen(inert_var_defs)
                  + strlen(sub_declarations)
                  + strlen(callback_declarations)
                  + strlen(method_typedefs)
                  + strlen(method_defs)
                  + strlen(vt_singleton_def)
                  + strlen(short_names)
                  + 100;

    char *content = (char*)MALLOCATE(size);
    sprintf(content, pattern, parent_include, privacy_symbol, struct_def,
            privacy_symbol, inert_var_defs, sub_declarations,
            callback_declarations, method_typedefs, method_defs,
            vt_singleton_def, short_names);

    FREEMEM(struct_def);
    FREEMEM(parent_include);
    FREEMEM(sub_declarations);
    FREEMEM(inert_var_defs);
    FREEMEM(method_typedefs);
    FREEMEM(method_defs);
    FREEMEM(vt_singleton_def);
    FREEMEM(callback_declarations);
    FREEMEM(short_names);
    return content;
}

char*
CFCBindClass_to_c(CFCBindClass *self) {
    CFCClass *client = self->client;

    if (CFCClass_inert(client)) {
        return CFCUtil_strdup(CFCClass_get_autocode(client));
    }

    const char *include_h = CFCClass_include_h(client);
    const char *autocode  = CFCClass_get_autocode(client);
    const char *vt_type   = CFCClass_full_vtable_type(client);
    const char *cnick     = CFCClass_get_cnick(client);
    char *class_name_def  = S_name_var_definition(self);
    char *vtable_def      = S_vtable_definition(self);

    CFCMethod **methods  = CFCClass_methods(client);
    CFCMethod **fresh_methods = CFCClass_fresh_methods(client);

    char *offsets    = CFCUtil_strdup("");
    char *cb_funcs   = CFCUtil_strdup("");
    char *cb_objects = CFCUtil_strdup("");

    /* Start a NULL-terminated array of cfish_Callback vars.  Since C89
     * doesn't allow us to initialize a pointer to an anonymous array inside a
     * global struct, we have to give it a real symbol and then store a pointer
     * to that symbol inside the VTable struct. */
    char *cb_var = CFCUtil_strdup("");
    cb_var = CFCUtil_cat(cb_var, "cfish_Callback *", self->full_callbacks_var,
                         "[] = {\n    ", NULL);

    for (int meth_num = 0; methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = methods[meth_num];
        int method_is_fresh = S_method_is_fresh(method, fresh_methods);
        size_t off_sym_size 
            = CFCMethod_full_offset_sym(method, cnick, NULL, 0);
        char *full_offset_sym = (char*)MALLOCATE(off_sym_size);
        CFCMethod_full_offset_sym(method, cnick, full_offset_sym,
                                  off_sym_size);
        char meth_num_str[20];
        sprintf(meth_num_str, "%d", meth_num);

        // Create offset in bytes for the method from the top of the VTable
        // object.
        char *offset_str = CFCUtil_cat(CFCUtil_strdup(""), "(offsetof(", vt_type,
                                       ", methods) + ", meth_num_str,
                                       " * sizeof(cfish_method_t))", NULL);
        offsets = CFCUtil_cat(offsets, "size_t ", full_offset_sym, " = ",
                              offset_str, ";\n", NULL);
        FREEMEM(full_offset_sym);

        // Create a default implementation for abstract methods.
        if (method_is_fresh && CFCMethod_abstract(method)) {
            char *method_def = CFCBindMeth_abstract_method_def(method);
            cb_funcs = CFCUtil_cat(cb_funcs, method_def, "\n", NULL);
            FREEMEM(method_def);
        }

        // Define callbacks for methods that can be overridden via the
        // host.
        if (CFCMethod_public(method) || CFCMethod_abstract(method)) {
            if (method_is_fresh && CFCMethod_novel(method)) {
                char *cb_def = CFCBindMeth_callback_def(method);
                char *cb_obj_def
                    = CFCBindMeth_callback_obj_def(method, offset_str);
                cb_funcs = CFCUtil_cat(cb_funcs, cb_def, "\n", NULL);
                cb_objects = CFCUtil_cat(cb_objects, cb_obj_def, NULL);
                FREEMEM(cb_def);
                FREEMEM(cb_obj_def);
            }
            CFCMethod *novel
                = CFCClass_find_novel_method(client,
                                             CFCMethod_micro_sym(method));
            const char *full_cb_sym = CFCMethod_full_callback_sym(novel);
            cb_var = CFCUtil_cat(cb_var, "&", full_cb_sym, ",\n    ", NULL);
        }

        FREEMEM(offset_str);
    }

    // Close callbacks variable definition.
    cb_var =  CFCUtil_cat(cb_var, "NULL\n};\n", NULL);

    const char pattern[] =
        "#include \"%s\"\n"
        "\n"
        "/* Offsets for method pointers, measured in bytes, from the top\n"
        " * of this class's vtable.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define functions which implement host callbacks for the methods\n"
        " * of this class which can be overridden via the host.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define the cfish_Callback objects which provide introspection\n"
        " * information and allow runtime overriding of dynamic methods.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Assemble all the cfish_Callback objects for this class so that\n"
        " * they can be searched when performing method overriding.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define the variable which holds this class's class name.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define this class's VTable.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Include auxilary automatically generated code for this class.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n";
    size_t size = sizeof(pattern)
                  + strlen(include_h)
                  + strlen(offsets)
                  + strlen(cb_funcs)
                  + strlen(cb_objects)
                  + strlen(cb_var)
                  + strlen(class_name_def)
                  + strlen(vtable_def)
                  + strlen(autocode)
                  + 100;
    char *code = (char*)MALLOCATE(size);
    sprintf(code, pattern, include_h, offsets, cb_funcs, cb_objects, cb_var,
            class_name_def, vtable_def, autocode);

    FREEMEM(fresh_methods);
    FREEMEM(offsets);
    FREEMEM(cb_funcs);
    FREEMEM(cb_objects);
    FREEMEM(cb_var);
    FREEMEM(class_name_def);
    FREEMEM(vtable_def);
    return code;
}

// Create the definition for the instantiable object struct.
static char*
S_struct_definition(CFCBindClass *self) {
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

static char*
S_name_var_definition(CFCBindClass *self) {
    const char *class_name  = CFCClass_get_class_name(self->client);
    unsigned class_name_len = (unsigned)strlen(class_name);

    const char pattern[] =
        "static cfish_ZombieCharBuf %s = {\n"
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
static char*
S_vtable_definition(CFCBindClass *self) {
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

// Declare cfish_Callback objects.
static char*
S_callback_declarations(CFCBindClass *self) {
    CFCMethod** fresh_methods = CFCClass_fresh_methods(self->client);
    char *declarations = CFCUtil_strdup("");
    for (int i = 0; fresh_methods[i] != NULL; i++) {
        CFCMethod *method = fresh_methods[i];
        if (CFCMethod_public(method) || CFCMethod_abstract(method)) {
            char *callback = CFCBindMeth_callback_dec(method);
            declarations = CFCUtil_cat(declarations, callback, NULL);
            FREEMEM(callback);
        }
    }
    FREEMEM(fresh_methods);
    return declarations;
}

// Declare typedefs for every fresh method implementation, to ease casting.
static char*
S_method_typedefs(CFCBindClass *self) {
    CFCMethod** fresh_methods = CFCClass_fresh_methods(self->client);
    char *typedefs = CFCUtil_strdup("");
    for (int i = 0; fresh_methods[i] != NULL; i++) {
        CFCMethod *method = fresh_methods[i];
        char *typedef_str = CFCBindMeth_typdef_dec(method);
        typedefs = CFCUtil_cat(typedefs, typedef_str, "\n", NULL);
        FREEMEM(typedef_str);
    }
    FREEMEM(fresh_methods);
    return typedefs;
}

// If class inherits from something, include the parent class's header.
static char*
S_parent_include(CFCBindClass *self) {
    char *parent_include = CFCUtil_strdup("");
    CFCClass *parent = CFCClass_get_parent(self->client);
    if (parent) {
        parent_include = CFCUtil_cat(parent_include, "#include \"",
                                     CFCClass_include_h(parent), "\"", NULL);
    }
    return parent_include;
}

// Add a C function definition for each method and each function.
static char*
S_sub_declarations(CFCBindClass *self) {
    CFCFunction **functions = CFCClass_functions(self->client);
    CFCMethod** fresh_methods = CFCClass_fresh_methods(self->client);
    char *declarations = CFCUtil_strdup("");
    for (int i = 0; functions[i] != NULL; i++) {
        CFCFunction *func = functions[i];
        char *dec = CFCBindFunc_func_declaration(func);
        declarations = CFCUtil_cat(declarations, dec, "\n\n", NULL);
        FREEMEM(dec);
    }
    for (int i = 0; fresh_methods[i] != NULL; i++) {
        CFCMethod *method = fresh_methods[i];
        char *dec = CFCBindFunc_func_declaration((CFCFunction*)method);
        declarations = CFCUtil_cat(declarations, dec, "\n\n", NULL);
        FREEMEM(dec);
    }
    FREEMEM(fresh_methods);
    return declarations;
}

// Declare class (a.k.a. "inert") variables.
static char*
S_inert_var_declarations(CFCBindClass *self) {
    CFCVariable **inert_vars = CFCClass_inert_vars(self->client);
    char *declarations = CFCUtil_strdup("");
    for (int i = 0; inert_vars[i] != NULL; i++) {
        const char *global_c = CFCVariable_global_c(inert_vars[i]);
        declarations = CFCUtil_cat(declarations, "extern ", global_c, ";\n",
                                   NULL);
    }
    return declarations;
}

// Define method invocation inline functions.
static char*
S_method_defs(CFCBindClass *self) {
    CFCMethod **methods = CFCClass_methods(self->client);
    char *method_defs = CFCUtil_strdup("");
    for (int i = 0; methods[i] != NULL; i++) {
        CFCMethod *method = methods[i];
        char *def = CFCBindMeth_method_def(method, self->client);
        method_defs = CFCUtil_cat(method_defs, def, "\n", NULL);
        FREEMEM(def);
    }
    return method_defs;
}


// Define the virtual table singleton object.
static char*
S_vt_singleton_def(CFCBindClass *self) {
    CFCClass   *client      = self->client;
    const char *vt_type     = CFCClass_full_vtable_type(client);
    const char *vt_var      = CFCClass_full_vtable_var(client);
    const char *vt_hidden   = CFCClass_full_vtable_hidden(client);
    size_t      num_methods = CFCClass_num_methods(client);

    const char pattern[] =
        "typedef struct %s {\n"
        "    cfish_VTable *vtable;\n"
        "    cfish_ref_t ref;\n"
        "    cfish_VTable *parent;\n"
        "    cfish_CharBuf *name;\n"
        "    uint32_t flags;\n"
        "    void *x;\n"
        "    size_t obj_alloc_size;\n"
        "    size_t vt_alloc_size;\n"
        "    void *callbacks;\n"
        "    cfish_method_t methods[%lu];\n"
        "} %s;\n"
        "extern struct %s %s;\n"
        "#define %s ((cfish_VTable*)&%s)\n";

    size_t size = sizeof(pattern)
                  + strlen(vt_type)
                  + 40 // num_methods
                  + strlen(vt_type)
                  + strlen(vt_type)
                  + strlen(vt_hidden)
                  + strlen(vt_var)
                  + strlen(vt_hidden)
                  + 30;
    char *def = (char*)MALLOCATE(size);
    sprintf(def, pattern, vt_type, (unsigned long)num_methods, vt_type,
            vt_type, vt_hidden, vt_var, vt_hidden);

    return def;
}

// Define short names for all of the symbols associated with this class.
static char*
S_short_names(CFCBindClass *self) {
    CFCClass *client = self->client;
    char *short_names = CFCUtil_strdup("");
    short_names = CFCUtil_cat(short_names, "#ifdef ", self->short_names_macro,
                              "\n", NULL);

    if (!CFCClass_inert(client)) {
        const char *short_struct = CFCClass_get_struct_sym(client);
        const char *full_struct  = CFCClass_full_struct_sym(client);
        const char *short_vt_var = CFCClass_short_vtable_var(client);
        const char *full_vt_var  = CFCClass_full_vtable_var(client);
        short_names = CFCUtil_cat(short_names, "  #define ",
                                  short_struct, " ", full_struct, "\n",
                                  "  #define ", short_vt_var, " ",
                                  full_vt_var, "\n", NULL);
    }

    CFCFunction **functions = CFCClass_functions(client);
    for (int i = 0; functions[i] != NULL; i++) {
        CFCFunction *func = functions[i];
        short_names = CFCUtil_cat(short_names, "  #define ",
                                  CFCFunction_short_func_sym(func), " ",
                                  CFCFunction_full_func_sym(func), "\n",
                                  NULL);
    }

    CFCVariable **inert_vars = CFCClass_inert_vars(client);
    for (int i = 0; inert_vars[i] != NULL; i++) {
        CFCVariable *var = inert_vars[i];
        short_names = CFCUtil_cat(short_names, "  #define ",
                                  CFCVariable_short_sym(var), " ",
                                  CFCVariable_full_sym(var), "\n", NULL);
    }

    if (!CFCClass_inert(client)) {
        CFCMethod **fresh_methods = CFCClass_fresh_methods(client);
        for (int i = 0; fresh_methods[i] != NULL; i++) {
            CFCMethod *meth = fresh_methods[i];
            const char *short_typedef = CFCMethod_short_typedef(meth);
            const char *full_typedef  = CFCMethod_full_typedef(meth);
            short_names = CFCUtil_cat(short_names, "  #define ",
                                      short_typedef, " ", full_typedef, "\n",
                                      NULL);
            const char *short_func = CFCMethod_short_implementing_func_sym(meth);
            const char *full_func  = CFCMethod_implementing_func_sym(meth);
            short_names = CFCUtil_cat(short_names, "  #define ", short_func,
                                      " ", full_func, "\n", NULL);
        }
        FREEMEM(fresh_methods);

        CFCMethod  **methods = CFCClass_methods(client);
        const char  *cnick   = CFCClass_get_cnick(client);
        for (int i = 0; methods[i] != NULL; i++) {
            CFCMethod *meth = methods[i];
            size_t size = CFCMethod_short_method_sym(meth, cnick, NULL, 0);
            char *short_sym = (char*)MALLOCATE(size);
            CFCMethod_short_method_sym(meth, cnick, short_sym, size);
            size = CFCMethod_full_method_sym(meth, cnick, NULL, 0);
            char *full_sym = (char*)MALLOCATE(size);
            CFCMethod_full_method_sym(meth, cnick, full_sym, size);
            short_names = CFCUtil_cat(short_names, "  #define ", short_sym,
                                      " ", full_sym, "\n", NULL);
            FREEMEM(short_sym);
            FREEMEM(full_sym);
        }
    }
    short_names = CFCUtil_cat(short_names, "#endif /* ",
                              self->short_names_macro, " */\n", NULL);

    return short_names;
}

