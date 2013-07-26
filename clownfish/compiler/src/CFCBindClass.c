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
#include "CFCParcel.h"
#include "CFCType.h"
#include "CFCVariable.h"
#include "CFCUtil.h"

struct CFCBindClass {
    CFCBase base;
    CFCClass *client;
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

static const CFCMeta CFCBINDCLASS_META = {
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

    const char *PREFIX = CFCClass_get_PREFIX(client);
    self->short_names_macro = CFCUtil_sprintf("%sUSE_SHORT_NAMES", PREFIX);

    return self;
}

void
CFCBindClass_destroy(CFCBindClass *self) {
    FREEMEM(self->short_names_macro);
    CFCBase_decref((CFCBase*)self->client);
    CFCBase_destroy((CFCBase*)self);
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
    char *content = CFCUtil_sprintf(pattern, inert_var_defs, inert_func_decs,
                                    short_names);

    FREEMEM(inert_var_defs);
    FREEMEM(inert_func_decs);
    FREEMEM(short_names);
    return content;
}

static char*
S_ivars_func(CFCBindClass *self) {
    CFCClass *client = self->client;
    const char *full_type    = CFCClass_full_struct_sym(client);
    const char *full_func    = CFCClass_full_ivars_func(client);
    const char *short_func   = CFCClass_short_ivars_func(client);
    const char *full_struct  = CFCClass_full_ivars_struct(client);
    const char *short_struct = CFCClass_short_ivars_struct(client);
    const char *full_offset  = CFCClass_full_ivars_offset(client);
    const char *PREFIX       = CFCClass_get_PREFIX(client);
    char pattern[] =
        "extern size_t %s;\n"
        "typedef struct %s %s;\n"
        "static CHY_INLINE %s*\n"
        "%s(%s *self) {\n"
        "   char *ptr = (char*)self + %s;\n"
        "   return (%s*)ptr;\n"
        "}\n"
        "#ifdef %sUSE_SHORT_NAMES\n"
        "  #define %s %s\n"
        "  #define %s %s\n"
        "#endif\n";
    char *content = CFCUtil_sprintf(pattern,
                                    full_offset,
                                    full_struct, full_struct,
                                    full_struct,
                                    full_func, full_type,
                                    full_offset,
                                    full_struct,
                                    PREFIX,
                                    short_struct, full_struct,
                                    short_func, full_func);
    return content;
}

static char*
S_to_c_header_dynamic(CFCBindClass *self) {
    const char *privacy_symbol  = CFCClass_privacy_symbol(self->client);
    const char *vt_var          = CFCClass_full_vtable_var(self->client);
    const char *PREFIX          = CFCClass_get_PREFIX(self->client);
    char *ivars                 = S_ivars_func(self);
    char *struct_def            = S_struct_definition(self);
    char *parent_include        = S_parent_include(self);
    char *sub_declarations      = S_sub_declarations(self);
    char *inert_var_defs        = S_inert_var_declarations(self);
    char *method_typedefs       = S_method_typedefs(self);
    char *method_defs           = S_method_defs(self);
    char *short_names           = S_short_names(self);

    char pattern[] =
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
        "extern %sVISIBLE cfish_VTable *%s;\n"
        "\n"
        "/* Define \"short names\" for this class's symbols.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n";
    char *content
        = CFCUtil_sprintf(pattern, parent_include, privacy_symbol, ivars,
                          struct_def, privacy_symbol, inert_var_defs,
                          sub_declarations, method_typedefs, method_defs,
                          PREFIX, vt_var, short_names);

    FREEMEM(ivars);
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
    const char *class_name = CFCClass_get_class_name(client);

    if (CFCClass_inert(client)) {
        return CFCUtil_strdup("");
    }

    const char *ivars_offset = CFCClass_full_ivars_offset(client);

    const char *vt_var    = CFCClass_full_vtable_var(client);

    CFCMethod **methods  = CFCClass_methods(client);

    char *offsets           = CFCUtil_strdup("");
    char *method_defs       = CFCUtil_strdup("");
    char *novel_ms_var      = CFCUtil_strdup("");
    char *overridden_ms_var = CFCUtil_strdup("");
    char *inherited_ms_var  = CFCUtil_strdup("");

    for (int meth_num = 0; methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = methods[meth_num];

        // Define method offset variable.
        char *full_offset_sym = CFCMethod_full_offset_sym(method, client);
        offsets = CFCUtil_cat(offsets, "size_t ", full_offset_sym, ";\n",
                              NULL);
        FREEMEM(full_offset_sym);

        const char *meth_class_name = CFCMethod_get_class_name(method);
        int is_fresh = strcmp(class_name, meth_class_name) == 0;

        // Create a default implementation for abstract methods.
        if (is_fresh && CFCMethod_abstract(method)) {
            char *method_def = CFCBindMeth_abstract_method_def(method);
            method_defs = CFCUtil_cat(method_defs, method_def, "\n", NULL);
            FREEMEM(method_def);
        }

        if (is_fresh && CFCMethod_novel(method)) {
            if (novel_ms_var[0] == '\0') {
                // Start an array of cfish_NovelMethSpec structs.  Since C89
                // doesn't allow us to initialize a pointer to an anonymous
                // array inside a global struct, we have to give it a real
                // symbol and then store a pointer to that symbol inside the
                // VTableSpec struct.
                novel_ms_var
                    = CFCUtil_cat(novel_ms_var,
                                  "static const cfish_NovelMethSpec ",
                                  vt_var, "_NOVEL_METHS[] = {\n", NULL);
            }
            else {
                novel_ms_var = CFCUtil_cat(novel_ms_var, ",\n", NULL);
            }
            char *ms_def = CFCBindMeth_novel_spec_def(method);
            novel_ms_var = CFCUtil_cat(novel_ms_var, ms_def, NULL);
            FREEMEM(ms_def);
        }
        else if (is_fresh) {
            if (overridden_ms_var[0] == '\0') {
                // Start an array of cfish_OverriddenMethSpec structs.
                overridden_ms_var
                    = CFCUtil_cat(overridden_ms_var,
                                  "static const cfish_OverriddenMethSpec ",
                                  vt_var, "_OVERRIDDEN_METHS[] = {\n", NULL);
            }
            else {
                overridden_ms_var
                    = CFCUtil_cat(overridden_ms_var, ",\n", NULL);
            }
            char *ms_def = CFCBindMeth_overridden_spec_def(method, client);
            overridden_ms_var = CFCUtil_cat(overridden_ms_var, ms_def, NULL);
            FREEMEM(ms_def);
        }
        else {
            if (inherited_ms_var[0] == '\0') {
                // Start an array of cfish_InheritedMethSpec structs.
                inherited_ms_var
                    = CFCUtil_cat(inherited_ms_var,
                                  "static const cfish_InheritedMethSpec ",
                                  vt_var, "_INHERITED_METHS[] = {\n", NULL);
            }
            else {
                inherited_ms_var = CFCUtil_cat(inherited_ms_var, ",\n", NULL);
            }
            char *ms_def = CFCBindMeth_inherited_spec_def(method, client);
            inherited_ms_var = CFCUtil_cat(inherited_ms_var, ms_def, NULL);
            FREEMEM(ms_def);
        }
    }

    // Close MethSpec array definitions.
    if (novel_ms_var[0] != '\0') {
        novel_ms_var = CFCUtil_cat(novel_ms_var, "\n};\n\n", NULL);
    }
    if (overridden_ms_var[0] != '\0') {
        overridden_ms_var = CFCUtil_cat(overridden_ms_var, "\n};\n\n", NULL);
    }
    if (inherited_ms_var[0] != '\0') {
        inherited_ms_var = CFCUtil_cat(inherited_ms_var, "\n};\n\n", NULL);
    }

    const char pattern[] =
        "/* Offset from the top of the object at which the IVARS struct\n"
        " * can be found.\n"
        " */\n"
        "\n"
        "size_t %s;\n"
        "\n"
        "/* Offsets for method pointers, measured in bytes, from the top\n"
        " * of this class's vtable.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define abstract methods of this class.\n"
        " */\n"
        "\n"
        "%s\n"
        "\n"
        "/* Define the MethSpec structs used during VTable initialization.\n"
        " */\n"
        "\n"
        "%s"
        "%s"
        "%s"
        "/* Define this class's VTable.\n"
        " */\n"
        "\n"
        "cfish_VTable *%s;\n"
        "\n";
    char *code
        = CFCUtil_sprintf(pattern, ivars_offset, offsets, method_defs,
                          novel_ms_var, overridden_ms_var, inherited_ms_var,
                          vt_var);

    FREEMEM(offsets);
    FREEMEM(method_defs);
    FREEMEM(novel_ms_var);
    FREEMEM(overridden_ms_var);
    FREEMEM(inherited_ms_var);
    return code;
}

// Create the definition for the instantiable object struct.
static char*
S_struct_definition(CFCBindClass *self) {
    CFCClass *const client = self->client;
    const char *struct_sym;
    const char *prefix = CFCClass_get_prefix(client);
    if (strcmp(prefix, "cfish_") == 0) {
        struct_sym = CFCClass_full_struct_sym(client);
    }
    else {
        struct_sym = CFCClass_full_ivars_struct(client);
    }

    // Count the number of member variables declared in ancestor classes
    // outside this package so that we can skip over them.
    int num_non_package_members = 0;
    CFCParcel *parcel = CFCClass_get_parcel(client);
    CFCClass *ancestor = CFCClass_get_parent(client);
    while (ancestor && CFCClass_get_parcel(ancestor) == parcel) {
        ancestor = CFCClass_get_parent(ancestor);
    }
    if (ancestor) {
        num_non_package_members = CFCClass_num_member_vars(ancestor);
    }

    // Add all member variables declared by classes in this package.
    CFCVariable **member_vars = CFCClass_member_vars(client);
    char *member_decs = CFCUtil_strdup("");
    for (int i = num_non_package_members; member_vars[i] != NULL; i++) {
        const char *member_dec = CFCVariable_local_declaration(member_vars[i]);
        size_t needed = strlen(member_decs) + strlen(member_dec) + 10;
        member_decs = (char*)REALLOCATE(member_decs, needed);
        strcat(member_decs, "\n    ");
        strcat(member_decs, member_dec);
    }

    char pattern[] = "struct %s {\n    CFISH_OBJ_HEAD%s\n};\n";
    char *struct_def = CFCUtil_sprintf(pattern, struct_sym, member_decs);

    FREEMEM(member_decs);
    return struct_def;
}

// Return C definition of the class's VTableSpec.
char*
CFCBindClass_spec_def(CFCBindClass *self) {
    CFCClass *client = self->client;

    CFCClass   *parent       = CFCClass_get_parent(client);
    const char *class_name   = CFCClass_get_class_name(client);
    const char *vt_var       = CFCClass_full_vtable_var(client);
    const char *struct_sym   = CFCClass_full_struct_sym(client);
    const char *ivars_struct = CFCClass_full_ivars_struct(client);
    const char *prefix       = CFCClass_get_prefix(client);

    // Create a pointer to the parent class's vtable.
    char *parent_ref;
    if (parent) {
        parent_ref = CFCUtil_sprintf("&%s", CFCClass_full_vtable_var(parent));
    }
    else {
        // No parent, e.g. Obj or inert classes.
        parent_ref = CFCUtil_strdup("NULL");
    }

    int num_novel      = 0;
    int num_overridden = 0;
    int num_inherited  = 0;
    CFCMethod **methods = CFCClass_methods(client);

    for (int meth_num = 0; methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = methods[meth_num];
        const char *meth_class_name = CFCMethod_get_class_name(method);

        if (strcmp(class_name, meth_class_name) == 0) {
            if (CFCMethod_novel(method)) {
                ++num_novel;
            }
            else {
                ++num_overridden;
            }
        }
        else {
            ++num_inherited;
        }
    }

    char *novel_ms_var      = num_novel
                              ? CFCUtil_sprintf("%s_NOVEL_METHS", vt_var)
                              : CFCUtil_strdup("NULL");
    char *overridden_ms_var = num_overridden
                              ? CFCUtil_sprintf("%s_OVERRIDDEN_METHS", vt_var)
                              : CFCUtil_strdup("NULL");
    char *inherited_ms_var  = num_inherited
                              ? CFCUtil_sprintf("%s_INHERITED_METHS", vt_var)
                              : CFCUtil_strdup("NULL");

    const char *ivars_or_not = strcmp(prefix, "cfish_") == 0
                               ? struct_sym : ivars_struct;
    const char *ivars_offset_name = CFCClass_full_ivars_offset(client);

    char pattern[] =
        "    {\n"
        "        &%s, /* vtable */\n"
        "        %s, /* parent */\n"
        "        \"%s\", /* name */\n"
        "        sizeof(%s), /* ivars_size */\n"
        "        &%s, /* ivars_offset_ptr */\n"
        "        %d, /* num_novel */\n"
        "        %d, /* num_overridden */\n"
        "        %d, /* num_inherited */\n"
        "        %s, /* novel_meth_specs */\n"
        "        %s, /* overridden_meth_specs */\n"
        "        %s /* inherited_meth_specs */\n"
        "    }";
    char *code
        = CFCUtil_sprintf(pattern, vt_var, parent_ref, class_name,
                          ivars_or_not, ivars_offset_name, num_novel,
                          num_overridden, num_inherited, novel_ms_var,
                          overridden_ms_var, inherited_ms_var);

    FREEMEM(parent_ref);
    FREEMEM(novel_ms_var);
    FREEMEM(overridden_ms_var);
    FREEMEM(inherited_ms_var);
    return code;
}

// Declare host callbacks.
char*
CFCBindClass_callback_decs(CFCBindClass *self) {
    CFCClass   *client        = self->client;
    CFCMethod **fresh_methods = CFCClass_fresh_methods(client);
    char       *cb_decs       = CFCUtil_strdup("");

    for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = fresh_methods[meth_num];

        // Declare callback.
        if (CFCMethod_novel(method) && !CFCMethod_final(method)) {
            char *cb_dec = CFCBindMeth_callback_dec(method);
            cb_decs = CFCUtil_cat(cb_decs, cb_dec, "\n", NULL);
            FREEMEM(cb_dec);
        }
    }

    FREEMEM(fresh_methods);

    return cb_decs;
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
    const char *PREFIX = CFCClass_get_PREFIX(self->client);
    CFCFunction **functions = CFCClass_functions(self->client);
    CFCMethod** fresh_methods = CFCClass_fresh_methods(self->client);
    char *declarations = CFCUtil_strdup("");
    for (int i = 0; functions[i] != NULL; i++) {
        CFCFunction *func = functions[i];
        char *dec = CFCBindFunc_func_declaration(func);
        if (!CFCFunction_inline(func)) {
            declarations = CFCUtil_cat(declarations, PREFIX, "VISIBLE ", NULL);
        }
        declarations = CFCUtil_cat(declarations, dec, "\n\n", NULL);
        FREEMEM(dec);
    }
    for (int i = 0; fresh_methods[i] != NULL; i++) {
        CFCMethod *method = fresh_methods[i];
        char *dec = CFCBindFunc_func_declaration((CFCFunction*)method);
        if (CFCMethod_final(method)) {
            declarations = CFCUtil_cat(declarations, PREFIX, "VISIBLE ", NULL);
        }
        declarations = CFCUtil_cat(declarations, dec, "\n\n", NULL);
        FREEMEM(dec);
    }
    FREEMEM(fresh_methods);
    return declarations;
}

// Declare class (a.k.a. "inert") variables.
static char*
S_inert_var_declarations(CFCBindClass *self) {
    const char *PREFIX = CFCClass_get_PREFIX(self->client);
    CFCVariable **inert_vars = CFCClass_inert_vars(self->client);
    char *declarations = CFCUtil_strdup("");
    for (int i = 0; inert_vars[i] != NULL; i++) {
        const char *global_c = CFCVariable_global_c(inert_vars[i]);
        declarations = CFCUtil_cat(declarations, "extern ", PREFIX, "VISIBLE ",
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
            static const char pattern[] = "  #define %s %s\n";

            // Method invocation symbols.
            char *short_sym  = CFCMethod_short_method_sym(meth, client);
            char *full_sym   = CFCMethod_full_method_sym(meth, client);
            char *define_sym = CFCUtil_sprintf(pattern, short_sym, full_sym);
            short_names = CFCUtil_cat(short_names, define_sym, NULL);
            FREEMEM(short_sym);
            FREEMEM(full_sym);
            FREEMEM(define_sym);

            // Method typedefs.
            char *short_typedef  = CFCMethod_short_typedef(meth, client);
            char *full_typedef   = CFCMethod_full_typedef(meth, client);
            char *define_typedef = CFCUtil_sprintf(pattern, short_typedef,
                                                   full_typedef);
            short_names = CFCUtil_cat(short_names, define_typedef, NULL);
            FREEMEM(short_typedef);
            FREEMEM(full_typedef);
            FREEMEM(define_typedef);
        }
    }
    short_names = CFCUtil_cat(short_names, "#endif /* ",
                              self->short_names_macro, " */\n", NULL);

    return short_names;
}

