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
#include <ctype.h>
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerl.h"
#include "CFCParcel.h"
#include "CFCClass.h"
#include "CFCHierarchy.h"
#include "CFCUtil.h"
#include "CFCPerlClass.h"
#include "CFCPerlSub.h"
#include "CFCPerlConstructor.h"
#include "CFCPerlMethod.h"
#include "CFCPerlTypeMap.h"

struct CFCPerl {
    CFCBase base;
    CFCParcel *parcel;
    CFCHierarchy *hierarchy;
    char *lib_dir;
    char *boot_class;
    char *header;
    char *footer;
    char *xs_path;
    char *pm_path;
    char *boot_h_file;
    char *boot_c_file;
    char *boot_h_path;
    char *boot_c_path;
    char *boot_func;
};

// Modify a string in place, swapping out "::" for the supplied character.
static void
S_replace_double_colons(char *text, char replacement);

const static CFCMeta CFCPERL_META = {
    "Clownfish::CFC::Binding::Perl",
    sizeof(CFCPerl),
    (CFCBase_destroy_t)CFCPerl_destroy
};

CFCPerl*
CFCPerl_new(CFCParcel *parcel, CFCHierarchy *hierarchy, const char *lib_dir,
            const char *boot_class, const char *header, const char *footer) {
    CFCPerl *self = (CFCPerl*)CFCBase_allocate(&CFCPERL_META);
    return CFCPerl_init(self, parcel, hierarchy, lib_dir, boot_class, header,
                        footer);
}

CFCPerl*
CFCPerl_init(CFCPerl *self, CFCParcel *parcel, CFCHierarchy *hierarchy,
             const char *lib_dir, const char *boot_class, const char *header,
             const char *footer) {
    CFCUTIL_NULL_CHECK(parcel);
    CFCUTIL_NULL_CHECK(hierarchy);
    CFCUTIL_NULL_CHECK(lib_dir);
    CFCUTIL_NULL_CHECK(boot_class);
    CFCUTIL_NULL_CHECK(header);
    CFCUTIL_NULL_CHECK(footer);
    self->parcel     = (CFCParcel*)CFCBase_incref((CFCBase*)parcel);
    self->hierarchy  = (CFCHierarchy*)CFCBase_incref((CFCBase*)hierarchy);
    self->lib_dir    = CFCUtil_strdup(lib_dir);
    self->boot_class = CFCUtil_strdup(boot_class);
    self->header     = CFCUtil_strdup(header);
    self->footer     = CFCUtil_strdup(footer);

    // Derive path to generated .xs file.
    self->xs_path = CFCUtil_cat(CFCUtil_strdup(""), lib_dir, CFCUTIL_PATH_SEP,
                                boot_class, ".xs", NULL);
    S_replace_double_colons(self->xs_path, CFCUTIL_PATH_SEP_CHAR);

    // Derive path to generated .pm file.
    self->pm_path = CFCUtil_cat(CFCUtil_strdup(""), lib_dir, CFCUTIL_PATH_SEP,
                                boot_class, CFCUTIL_PATH_SEP,
                                "Autobinding.pm", NULL);
    S_replace_double_colons(self->pm_path, CFCUTIL_PATH_SEP_CHAR);

    // Derive the name of the files containing bootstrapping code.
    const char *prefix   = CFCParcel_get_prefix(parcel);
    const char *inc_dest = CFCHierarchy_get_include_dest(hierarchy);
    const char *src_dest = CFCHierarchy_get_source_dest(hierarchy);
    self->boot_h_file = CFCUtil_cat(CFCUtil_strdup(""), prefix, "boot.h",
                                    NULL);
    self->boot_c_file = CFCUtil_cat(CFCUtil_strdup(""), prefix, "boot.c",
                                    NULL);
    self->boot_h_path = CFCUtil_cat(CFCUtil_strdup(""), inc_dest,
                                    CFCUTIL_PATH_SEP, self->boot_h_file,
                                    NULL);
    self->boot_c_path = CFCUtil_cat(CFCUtil_strdup(""), src_dest,
                                    CFCUTIL_PATH_SEP, self->boot_c_file,
                                    NULL);

    // Derive the name of the bootstrap function.
    self->boot_func
        = CFCUtil_cat(CFCUtil_strdup(""), CFCParcel_get_prefix(parcel),
                      boot_class, "_bootstrap", NULL);
    for (int i = 0; self->boot_func[i] != 0; i++) {
        if (!isalnum(self->boot_func[i])) {
            self->boot_func[i] = '_';
        }
    }

    return self;
}

void
CFCPerl_destroy(CFCPerl *self) {
    CFCBase_decref((CFCBase*)self->parcel);
    CFCBase_decref((CFCBase*)self->hierarchy);
    FREEMEM(self->lib_dir);
    FREEMEM(self->boot_class);
    FREEMEM(self->header);
    FREEMEM(self->footer);
    FREEMEM(self->xs_path);
    FREEMEM(self->pm_path);
    FREEMEM(self->boot_h_file);
    FREEMEM(self->boot_c_file);
    FREEMEM(self->boot_h_path);
    FREEMEM(self->boot_c_path);
    FREEMEM(self->boot_func);
    CFCBase_destroy((CFCBase*)self);
}

static void
S_replace_double_colons(char *text, char replacement) {
    size_t pos = 0;
    for (char *ptr = text; *ptr != '\0'; ptr++) {
        if (strncmp(ptr, "::", 2) == 0) {
            text[pos++] = replacement;
            ptr++;
        }
        else {
            text[pos++] = *ptr;
        }
    }
    text[pos] = '\0';
}

char**
CFCPerl_write_pod(CFCPerl *self) {
    CFCPerlClass **registry  = CFCPerlClass_registry();
    size_t num_registered = 0;
    while (registry[num_registered] != NULL) { num_registered++; }
    char     **pod_paths = (char**)CALLOCATE(num_registered + 1, sizeof(char*));
    char     **pods      = (char**)CALLOCATE(num_registered + 1, sizeof(char*));
    size_t     count     = 0;

    // Generate POD, but don't write.  That way, if there's an error while
    // generating pod, we leak memory but don't clutter up the file system.
    for (size_t i = 0; i < num_registered; i++) {
        const char *class_name = CFCPerlClass_get_class_name(registry[i]);
        char *pod = CFCPerlClass_create_pod(registry[i]);
        if (!pod) { continue; }
        char *pod_path
            = CFCUtil_cat(CFCUtil_strdup(""), self->lib_dir, CFCUTIL_PATH_SEP,
                          class_name, ".pod", NULL);
        S_replace_double_colons(pod_path, CFCUTIL_PATH_SEP_CHAR);

        pods[count] = pod;
        pod_paths[count] = pod_path;
        count++;
    }

    // Write out any POD files that have changed.
    size_t num_written = 0;
    for (size_t i = 0; i < count; i++) {
        char *pod      = pods[i];
        char *pod_path = pod_paths[i];
        if (CFCUtil_write_if_changed(pod_path, pod, strlen(pod))) {
            pod_paths[num_written] = pod_path;
            num_written++;
        }
        else {
            FREEMEM(pod_path);
        }
        FREEMEM(pod);
    }
    pod_paths[num_written] = NULL;

    return pod_paths;
}

static void
S_write_boot_h(CFCPerl *self) {
    char *guard = CFCUtil_cat(CFCUtil_strdup(""), self->boot_class,
                              "_BOOT", NULL);
    S_replace_double_colons(guard, '_');
    for (char *ptr = guard; *ptr != '\0'; ptr++) {
        if (isalpha(*ptr)) {
            *ptr = toupper(*ptr);
        }
    }

    const char pattern[] = 
        "%s\n"
        "\n"
        "#ifndef %s\n"
        "#define %s 1\n"
        "\n"
        "void\n"
        "%s();\n"
        "\n"
        "#endif /* %s */\n"
        "\n"
        "%s\n";

    size_t size = sizeof(pattern)
                  + strlen(self->header)
                  + strlen(guard)
                  + strlen(guard)
                  + strlen(self->boot_func)
                  + strlen(guard)
                  + strlen(self->footer)
                  + 20;
    char *content = (char*)MALLOCATE(size);
    sprintf(content, pattern, self->header, guard, guard, self->boot_func,
            guard, self->footer);
    CFCUtil_write_file(self->boot_h_path, content, strlen(content));

    FREEMEM(content);
    FREEMEM(guard);
}

static void
S_write_boot_c(CFCPerl *self) {
    CFCClass **ordered   = CFCHierarchy_ordered_classes(self->hierarchy);
    char *pound_includes = CFCUtil_strdup("");
    char *registrations  = CFCUtil_strdup("");
    char *isa_pushes     = CFCUtil_strdup("");

    for (size_t i = 0; ordered[i] != NULL; i++) {
        CFCClass *klass = ordered[i];
        const char *class_name = CFCClass_get_class_name(klass);
        const char *include_h  = CFCClass_include_h(klass);
        pound_includes = CFCUtil_cat(pound_includes, "#include \"",
                                     include_h, "\"\n", NULL);

        if (CFCClass_inert(klass) || CFCClass_included(klass)) { continue; }

        // Ignore return value from VTable_add_to_registry, since it's OK if
        // multiple threads contend for adding these permanent VTables and some
        // fail.
        registrations
            = CFCUtil_cat(registrations, "    cfish_VTable_add_to_registry(",
                          CFCClass_full_vtable_var(klass), ");\n", NULL);

        // Add aliases for selected KinoSearch classes which allow old indexes
        // to be read.
        CFCPerlClass *class_binding = CFCPerlClass_singleton(class_name);
        if (class_binding) {
            const char *vtable_var = CFCClass_full_vtable_var(klass);
            char **aliases
                = CFCPerlClass_get_class_aliases(class_binding);
            for (size_t j = 0; aliases[j] != NULL; j++) {
                const char *alias = aliases[j];
                size_t alias_len  = strlen(alias);
                const char pattern[] =
                    "%s"
                    "    Cfish_ZCB_Assign_Str(alias, \"%s\", %u);\n"
                    "    cfish_VTable_add_alias_to_registry(%s,\n"
                    "        (cfish_CharBuf*)alias);\n";

                size_t new_size = sizeof(pattern)
                                  + strlen(registrations)
                                  + alias_len
                                  + 20    // stringified alias_len
                                  + strlen(vtable_var)
                                  + 50;
                char *new_registrations = (char*)MALLOCATE(new_size);
                sprintf(new_registrations, pattern, registrations, alias,
                        (unsigned)alias_len, vtable_var);
                FREEMEM(registrations);
                registrations = new_registrations;
            }
        }

        CFCClass *parent = CFCClass_get_parent(klass);
        if (parent) {
            const char *parent_class_name = CFCClass_get_class_name(parent);
            isa_pushes
                = CFCUtil_cat(isa_pushes, "    isa = get_av(\"",
                              class_name, "::ISA\", 1);\n", NULL);
            isa_pushes
                = CFCUtil_cat(isa_pushes, "    av_push(isa, newSVpv(\"", 
                              parent_class_name, "\", 0));\n", NULL);
        }
    }

    const char pattern[] =
        "%s\n"
        "\n"
        "#include \"EXTERN.h\"\n"
        "#include \"perl.h\"\n"
        "#include \"XSUB.h\"\n"
        "#include \"%s\"\n"
        "#include \"parcel.h\"\n"
        "%s\n"
        "\n"
        "void\n"
        "%s() {\n"
        "    AV *isa;\n"
        "    cfish_ZombieCharBuf *alias = CFISH_ZCB_WRAP_STR(\"\", 0);\n"
        "%s\n"
        "%s\n"
        "}\n"
        "\n"
        "%s\n"
        "\n";

    size_t size = sizeof(pattern)
                  + strlen(self->header)
                  + strlen(self->boot_h_file)
                  + strlen(pound_includes)
                  + strlen(self->boot_func)
                  + strlen(registrations)
                  + strlen(isa_pushes)
                  + strlen(self->footer)
                  + 100;
    char *content = (char*)MALLOCATE(size);
    sprintf(content, pattern, self->header, self->boot_h_file, pound_includes,
            self->boot_func, registrations, isa_pushes, self->footer);
    CFCUtil_write_file(self->boot_c_path, content, strlen(content));

    FREEMEM(content);
    FREEMEM(isa_pushes);
    FREEMEM(registrations);
    FREEMEM(pound_includes);
    FREEMEM(ordered);
}

void
CFCPerl_write_boot(CFCPerl *self) {
    S_write_boot_h(self);
    S_write_boot_c(self);
}

static char*
S_pm_file_contents(CFCPerl *self, const char *params_hash_defs) {
    const char pattern[] = 
    "# DO NOT EDIT!!!! This is an auto-generated file.\n"
    "\n"
    "# Licensed to the Apache Software Foundation (ASF) under one or more\n"
    "# contributor license agreements.  See the NOTICE file distributed with\n"
    "# this work for additional information regarding copyright ownership.\n"
    "# The ASF licenses this file to You under the Apache License, Version 2.0\n"
    "# (the \"License\"); you may not use this file except in compliance with\n"
    "# the License.  You may obtain a copy of the License at\n"
    "#\n"
    "#     http://www.apache.org/licenses/LICENSE-2.0\n"
    "#\n"
    "# Unless required by applicable law or agreed to in writing, software\n"
    "# distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    "# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    "# See the License for the specific language governing permissions and\n"
    "# limitations under the License.\n"
    "\n"
    "use strict;\n"
    "use warnings;\n"
    "\n"
    "package %s::Autobinding;\n"
    "\n"
    "init_autobindings();\n"
    "\n"
    "%s\n"
    "\n"
    "1;\n"
    "\n";
    size_t size = sizeof(pattern)
                  + strlen(self->boot_class)
                  + strlen(params_hash_defs)
                  + 20;
    char *contents = (char*)MALLOCATE(size);
    sprintf(contents, pattern, self->boot_class, params_hash_defs);
    return contents;
}

static char*
S_xs_file_contents(CFCPerl *self, const char *generated_xs,
                   const char *xs_init, const char *hand_rolled_xs) {
    const char pattern[] = 
    "/* DO NOT EDIT!!!! This is an auto-generated file. */\n"
    "\n"
    "/* Licensed to the Apache Software Foundation (ASF) under one or more\n"
    " * contributor license agreements.  See the NOTICE file distributed with\n"
    " * this work for additional information regarding copyright ownership.\n"
    " * The ASF licenses this file to You under the Apache License, Version 2.0\n"
    " * (the \"License\"); you may not use this file except in compliance with\n"
    " * the License.  You may obtain a copy of the License at\n"
    " *\n"
    " *     http://www.apache.org/licenses/LICENSE-2.0\n"
    " *\n"
    " * Unless required by applicable law or agreed to in writing, software\n"
    " * distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    " * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    " * See the License for the specific language governing permissions and\n"
    " * limitations under the License.\n"
    " */\n"
    "\n"
    "#include \"XSBind.h\"\n"
    "#include \"parcel.h\"\n"
    "#include \"%s\"\n"
    "\n"
    "#include \"Lucy/Object/Host.h\"\n"
    "#include \"Lucy/Util/Memory.h\"\n"
    "#include \"Lucy/Util/StringHelper.h\"\n"
    "\n"
    "%s\n"
    "\n"
    "MODULE = %s   PACKAGE = %s::Autobinding\n"
    "\n"
    "void\n"
    "init_autobindings()\n"
    "PPCODE:\n"
    "{\n"
    "    char* file = __FILE__;\n"
    "    CHY_UNUSED_VAR(cv);\n"
    "    CHY_UNUSED_VAR(items); %s\n"
    "}\n"
    "\n"
    "%s\n"
    "\n";

    size_t size = sizeof(pattern)
                  + strlen(self->boot_h_file)
                  + strlen(generated_xs)
                  + strlen(self->boot_class) * 2
                  + strlen(xs_init)
                  + strlen(hand_rolled_xs)
                  + 30;
    char *contents = (char*)MALLOCATE(size);
    sprintf(contents, pattern, self->boot_h_file, generated_xs,
            self->boot_class, self->boot_class, xs_init, hand_rolled_xs);

    return contents;
}

static char*
S_add_xs_init(char *xs_init, CFCPerlSub *xsub) {
    const char *c_name = CFCPerlSub_c_name(xsub);
    const char *perl_name = CFCPerlSub_perl_name(xsub);
    if (strlen(xs_init)) {
        xs_init = CFCUtil_cat(xs_init, "\n    ", NULL);
    }
    xs_init = CFCUtil_cat(xs_init, "newXS(\"", perl_name, "\", ", c_name,
                          ", file);", NULL);
    return xs_init;
}

// Params hashes for arg checking of XSUBs that take labeled params.
static char*
S_add_params_hash_def(char *params_hash_defs, CFCPerlSub *xsub) {
    char *def = CFCPerlSub_params_hash_def(xsub);
    if (def) {
        if (strlen(params_hash_defs)) {
            params_hash_defs = CFCUtil_cat(params_hash_defs, "\n", NULL);
        }
        params_hash_defs = CFCUtil_cat(params_hash_defs, def, NULL);
    }
    return params_hash_defs;
}

void
CFCPerl_write_bindings(CFCPerl *self) {
    CFCClass **ordered = CFCHierarchy_ordered_classes(self->hierarchy);
    CFCPerlClass **registry = CFCPerlClass_registry();
    char *hand_rolled_xs   = CFCUtil_strdup("");
    char *generated_xs     = CFCUtil_strdup("");
    char *xs_init          = CFCUtil_strdup("");
    char *params_hash_defs = CFCUtil_strdup("");

    // Pound-includes for generated headers.
    for (size_t i = 0; ordered[i] != NULL; i++) {
        const char *include_h = CFCClass_include_h(ordered[i]);
        generated_xs = CFCUtil_cat(generated_xs, "#include \"", include_h,
                                   "\"\n", NULL);
    }
    generated_xs = CFCUtil_cat(generated_xs, "\n", NULL);

    for (size_t i = 0; registry[i] != NULL; i++) {
        CFCPerlClass *class_binding = registry[i];
        CFCClass *client = CFCPerlClass_get_client(class_binding);
        if (!client) { continue; }

        // Constructors.
        CFCPerlConstructor **constructors
            = CFCPerlClass_constructor_bindings(class_binding);
        for (size_t j = 0; constructors[j] != NULL; j++) {
            CFCPerlSub *xsub = (CFCPerlSub*)constructors[j];

            // Add the XSUB function definition.
            char *xsub_def = CFCPerlConstructor_xsub_def(constructors[j]);
            generated_xs = CFCUtil_cat(generated_xs, xsub_def, "\n",
                                       NULL);
            FREEMEM(xsub_def);

            // Add params hash def, XSUB initialization at boot.
            params_hash_defs = S_add_params_hash_def(params_hash_defs, xsub);
            xs_init = S_add_xs_init(xs_init, xsub);
        }
        FREEMEM(constructors);

        // Methods.
        CFCPerlMethod **methods
            = CFCPerlClass_method_bindings(class_binding);
        for (size_t j = 0; methods[j] != NULL; j++) {
            CFCPerlSub *xsub = (CFCPerlSub*)methods[j];

            // Add the XSUB function definition.
            char *xsub_def = CFCPerlMethod_xsub_def(methods[j]);
            generated_xs = CFCUtil_cat(generated_xs, xsub_def, "\n",
                                       NULL);
            FREEMEM(xsub_def);

            // Add params hash def, XSUB initialization at boot.
            params_hash_defs = S_add_params_hash_def(params_hash_defs, xsub);
            xs_init = S_add_xs_init(xs_init, xsub);
        }
        FREEMEM(methods);
    }

    // Hand-rolled XS.
    for (size_t i = 0; registry[i] != NULL; i++) {
        CFCPerlClass *class_binding = registry[i];
        const char *xs = CFCPerlClass_get_xs_code(registry[i]);
        hand_rolled_xs = CFCUtil_cat(hand_rolled_xs, xs, "\n", NULL);
    }

    // Write out if there have been any changes.
    char *xs_file_contents
        = S_xs_file_contents(self, generated_xs, xs_init, hand_rolled_xs);
    char *pm_file_contents = S_pm_file_contents(self, params_hash_defs);
    CFCUtil_write_if_changed(self->xs_path, xs_file_contents,
                             strlen(xs_file_contents));
    CFCUtil_write_if_changed(self->pm_path, pm_file_contents,
                             strlen(pm_file_contents));

    FREEMEM(pm_file_contents);
    FREEMEM(xs_file_contents);
    FREEMEM(params_hash_defs);
    FREEMEM(hand_rolled_xs);
    FREEMEM(xs_init);
    FREEMEM(generated_xs);
    FREEMEM(ordered);
}

void
CFCPerl_write_xs_typemap(CFCPerl *self) {
    CFCPerlTypeMap_write_xs_typemap(self->hierarchy);
}

