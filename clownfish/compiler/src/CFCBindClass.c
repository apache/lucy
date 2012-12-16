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
    char *method_specs_var;
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

// Return C code defining the class's VTable.
static char*
S_vtable_definition(CFCBindClass *self);

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
    self->method_specs_var  = (char*)MALLOCATE(strlen(full_vtable_var) + 20);
    self->short_names_macro = (char*)MALLOCATE(strlen(PREFIX) + 20);
    sprintf(self->method_specs_var, "%s_METHODS", full_vtable_var);
    sprintf(self->short_names_macro, "%sUSE_SHORT_NAMES", PREFIX);

    return self;
}

void
CFCBindClass_destroy(CFCBindClass *self) {
    FREEMEM(self->method_specs_var);
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
    const char *vt_var          = CFCClass_full_vtable_var(self->client);
    char *struct_def            = S_struct_definition(self);
    char *parent_include        = S_parent_include(self);
    char *sub_declarations      = S_sub_declarations(self);
    char *inert_var_defs        = S_inert_var_declarations(self);
    char *method_typedefs       = S_method_typedefs(self);
    char *method_defs           = S_method_defs(self);
    char *short_names           = S_short_names(self);

    const char *visibility = CFCClass_included(self->client)
                             ? "CHY_IMPORT" : "CHY_EXPORT";

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
        "/* Declare the VTable singleton for this class.\n"
        " */\n"
        "\n"
        "extern %s cfish_VTable *%s;\n"
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
                  + strlen(method_typedefs)
                  + strlen(method_defs)
                  + strlen(visibility)
                  + strlen(vt_var)
                  + strlen(short_names)
                  + 100;

    char *content = (char*)MALLOCATE(size);
    sprintf(content, pattern, parent_include, privacy_symbol, struct_def,
            privacy_symbol, inert_var_defs, sub_declarations, method_typedefs,
            method_defs, visibility, vt_var, short_names);

    FREEMEM(struct_def);
    FREEMEM(parent_include);
    FREEMEM(sub_declarations);
    FREEMEM(inert_var_defs);
    FREEMEM(method_typedefs);
    FREEMEM(method_defs);
    FREEMEM(short_names);
    return content;
}

char*
CFCBindClass_to_c_data(CFCBindClass *self) {
    CFCClass *client = self->client;

    if (CFCClass_inert(client)) {
        return CFCUtil_strdup(CFCClass_get_autocode(client));
    }

    const char *autocode  = CFCClass_get_autocode(client);
    const char *vt_var    = CFCClass_full_vtable_var(client);

    CFCMethod **methods  = CFCClass_methods(client);
    CFCMethod **fresh_methods = CFCClass_fresh_methods(client);

    char *offsets  = CFCUtil_strdup("");
    char *cb_funcs = CFCUtil_strdup("");
    char *ms_var   = CFCUtil_strdup("");

    for (int meth_num = 0; methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = methods[meth_num];
        size_t off_sym_size 
            = CFCMethod_full_offset_sym(method, client, NULL, 0);
        char *full_offset_sym = (char*)MALLOCATE(off_sym_size);
        CFCMethod_full_offset_sym(method, client, full_offset_sym,
                                  off_sym_size);
        char meth_num_str[20];
        sprintf(meth_num_str, "%d", meth_num);

        // Create offset in bytes for the method from the top of the VTable
        // object.
        offsets = CFCUtil_cat(offsets, "size_t ", full_offset_sym, " = ",
                              "offsetof(cfish_VTable, method_ptrs) + ",
                              meth_num_str, " * sizeof(cfish_method_t);\n",
                              NULL);
        FREEMEM(full_offset_sym);
    }

    if (fresh_methods[0] != NULL) {
        /* Start an array of cfish_MethodSpec structs.  Since C89 doesn't allow 
         * us to initialize a pointer to an anonymous array inside a global
         * struct, we have to give it a real symbol and then store a pointer to
         * that symbol inside the VTableSpec struct. */
        ms_var = CFCUtil_cat(ms_var, "static cfish_MethodSpec ",
                             self->method_specs_var, "[] = {\n", NULL);

        for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
            CFCMethod *method = fresh_methods[meth_num];

            // Create a default implementation for abstract methods.
            if (CFCMethod_abstract(method)) {
                char *method_def = CFCBindMeth_abstract_method_def(method);
                cb_funcs = CFCUtil_cat(cb_funcs, method_def, "\n", NULL);
                FREEMEM(method_def);
            }

            // Declare (but don't define) callback.
            if (CFCMethod_novel(method) && !CFCMethod_final(method)) {
                char *cb_dec = CFCBindMeth_callback_dec(method);
                cb_funcs = CFCUtil_cat(cb_funcs, cb_dec, "\n", NULL);
                FREEMEM(cb_dec);
            }

            // Define MethodSpec struct.
            if (meth_num != 0) {
                ms_var = CFCUtil_cat(ms_var, ",\n", NULL);
            }
            char *ms_def = CFCBindMeth_spec_def(method);
            ms_var = CFCUtil_cat(ms_var, ms_def, NULL);
            FREEMEM(ms_def);
        }

        // Close MethodSpec array definition.
        ms_var =  CFCUtil_cat(ms_var, "\n};\n", NULL);
    }

    const char pattern[] =
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
        "/* Define the cfish_MethodSpec structs used during VTable\n"
        " * initialization.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define this class's VTable.\n"
        " */\n"
        "\n"
        "cfish_VTable *%s;\n"
        "\n"
        "/* Include auxilary automatically generated code for this class.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n";
    size_t size = sizeof(pattern)
                  + strlen(offsets)
                  + strlen(cb_funcs)
                  + strlen(ms_var)
                  + strlen(vt_var)
                  + strlen(autocode)
                  + 100;
    char *code = (char*)MALLOCATE(size);
    sprintf(code, pattern, offsets, cb_funcs, ms_var, vt_var, autocode);

    FREEMEM(fresh_methods);
    FREEMEM(offsets);
    FREEMEM(cb_funcs);
    FREEMEM(ms_var);
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

// Return C definition of the class's VTableSpec.
char*
CFCBindClass_spec_def(CFCBindClass *self) {
    CFCClass *client = self->client;

    CFCClass    *parent     = CFCClass_get_parent(client);
    const char  *class_name = CFCClass_get_class_name(client);
    const char  *vt_var     = CFCClass_full_vtable_var(client);
    const char  *struct_sym = CFCClass_full_struct_sym(client);

    // Create a pointer to the parent class's vtable.
    char *parent_ref;
    if (parent) {
        parent_ref = CFCUtil_cat(CFCUtil_strdup("&"),
                                 CFCClass_full_vtable_var(parent), NULL);
    }
    else {
        // No parent, e.g. Obj or inert classes.
        parent_ref = CFCUtil_strdup("NULL");
    }

    int num_fresh = 0;
    int num_novel = 0;
    CFCMethod **fresh_methods = CFCClass_fresh_methods(client);
    for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = fresh_methods[meth_num];
        ++num_fresh;
        if (CFCMethod_novel(method)) { ++num_novel; }
    }
    FREEMEM(fresh_methods);
    const char *ms_var = num_fresh ? self->method_specs_var : "NULL";

    char pattern[] =
        "    {\n"
        "        &%s, /* vtable */\n"
        "        %s, /* parent */\n"
        "        \"%s\", /* name */\n"
        "        sizeof(%s), /* obj_alloc_size */\n"
        "        %d, /* num_fresh */\n"
        "        %d, /* num_novel */\n"
        "        %s /* method_specs */\n"
        "    }";

    size_t size = sizeof(pattern)
                  + strlen(vt_var)
                  + strlen(parent_ref)
                  + strlen(class_name)
                  + strlen(struct_sym)
                  + 10 // for num_fresh
                  + 10 // for num_novel
                  + strlen(ms_var)
                  + 100;
    char *code = (char*)MALLOCATE(size);
    sprintf(code, pattern, vt_var, parent_ref, class_name, struct_sym,
            num_fresh, num_novel, ms_var);

    FREEMEM(parent_ref);
    return code;
}

// Declare typedefs for every method, to ease casting.
static char*
S_method_typedefs(CFCBindClass *self) {
    CFCMethod** methods = CFCClass_methods(self->client);
    char *typedefs = CFCUtil_strdup("");
    for (int i = 0; methods[i] != NULL; i++) {
        CFCMethod *method = methods[i];
        char *typedef_str = CFCBindMeth_typedef_dec(method, self->client);
        typedefs = CFCUtil_cat(typedefs, typedef_str, "\n", NULL);
        FREEMEM(typedef_str);
    }
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
    const char *visibility = CFCClass_included(self->client)
                             ? "CHY_IMPORT " : "CHY_EXPORT ";
    CFCFunction **functions = CFCClass_functions(self->client);
    CFCMethod** fresh_methods = CFCClass_fresh_methods(self->client);
    char *declarations = CFCUtil_strdup("");
    for (int i = 0; functions[i] != NULL; i++) {
        CFCFunction *func = functions[i];
        char *dec = CFCBindFunc_func_declaration(func);
        if (!CFCFunction_inline(func)) {
            declarations = CFCUtil_cat(declarations, visibility, NULL);
        }
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
    const char *visibility = CFCClass_included(self->client)
                             ? "CHY_IMPORT " : "CHY_EXPORT ";
    CFCVariable **inert_vars = CFCClass_inert_vars(self->client);
    char *declarations = CFCUtil_strdup("");
    for (int i = 0; inert_vars[i] != NULL; i++) {
        const char *global_c = CFCVariable_global_c(inert_vars[i]);
        declarations = CFCUtil_cat(declarations, "extern ", visibility,
                                   global_c, ";\n", NULL);
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

            // Implementing functions.
            const char *short_func = CFCMethod_short_implementing_func_sym(meth);
            const char *full_func  = CFCMethod_implementing_func_sym(meth);
            short_names = CFCUtil_cat(short_names, "  #define ", short_func,
                                      " ", full_func, "\n", NULL);
        }
        FREEMEM(fresh_methods);

        CFCMethod  **methods = CFCClass_methods(client);
        for (int i = 0; methods[i] != NULL; i++) {
            CFCMethod *meth = methods[i];

            // Method invocation symbols.
            size_t size = CFCMethod_short_method_sym(meth, client, NULL, 0);
            char *short_sym = (char*)MALLOCATE(size);
            CFCMethod_short_method_sym(meth, client, short_sym, size);
            size = CFCMethod_full_method_sym(meth, client, NULL, 0);
            char *full_sym = (char*)MALLOCATE(size);
            CFCMethod_full_method_sym(meth, client, full_sym, size);
            short_names = CFCUtil_cat(short_names, "  #define ", short_sym,
                                      " ", full_sym, "\n", NULL);
            FREEMEM(short_sym);
            FREEMEM(full_sym);

            // Method typedefs.
            size = CFCMethod_short_typedef(meth, client, NULL, 0);
            char *short_typedef = (char*)MALLOCATE(size);
            CFCMethod_short_typedef(meth, client, short_typedef, size);
            size = CFCMethod_full_typedef(meth, client, NULL, 0);
            char *full_typedef = (char*)MALLOCATE(size);
            CFCMethod_full_typedef(meth, client, full_typedef, size);
            short_names = CFCUtil_cat(short_names, "  #define ",
                                      short_typedef, " ", full_typedef, "\n",
                                      NULL);
            FREEMEM(short_typedef);
            FREEMEM(full_typedef);
        }
    }
    short_names = CFCUtil_cat(short_names, "#endif /* ",
                              self->short_names_macro, " */\n", NULL);

    return short_names;
}

