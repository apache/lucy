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

#include "charmony.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerl.h"
#include "CFCParcel.h"
#include "CFCClass.h"
#include "CFCMethod.h"
#include "CFCHierarchy.h"
#include "CFCUtil.h"
#include "CFCPerlClass.h"
#include "CFCPerlSub.h"
#include "CFCPerlConstructor.h"
#include "CFCPerlMethod.h"
#include "CFCPerlTypeMap.h"
#include "CFCBindCore.h"

struct CFCPerl {
    CFCBase base;
    CFCHierarchy *hierarchy;
    char *lib_dir;
    char *boot_class;
    char *header;
    char *footer;
    char *xs_path;
    char *boot_func;
};

// Modify a string in place, swapping out "::" for the supplied character.
static void
S_replace_double_colons(char *text, char replacement);

static void
S_write_callbacks_c(CFCPerl *self);

static const CFCMeta CFCPERL_META = {
    "Clownfish::CFC::Binding::Perl",
    sizeof(CFCPerl),
    (CFCBase_destroy_t)CFCPerl_destroy
};

CFCPerl*
CFCPerl_new(CFCHierarchy *hierarchy, const char *lib_dir,
            const char *boot_class, const char *header, const char *footer) {
    CFCPerl *self = (CFCPerl*)CFCBase_allocate(&CFCPERL_META);
    return CFCPerl_init(self, hierarchy, lib_dir, boot_class, header, footer);
}

CFCPerl*
CFCPerl_init(CFCPerl *self, CFCHierarchy *hierarchy, const char *lib_dir,
             const char *boot_class, const char *header, const char *footer) {
    CFCUTIL_NULL_CHECK(hierarchy);
    CFCUTIL_NULL_CHECK(lib_dir);
    CFCUTIL_NULL_CHECK(boot_class);
    CFCUTIL_NULL_CHECK(header);
    CFCUTIL_NULL_CHECK(footer);
    self->hierarchy  = (CFCHierarchy*)CFCBase_incref((CFCBase*)hierarchy);
    self->lib_dir    = CFCUtil_strdup(lib_dir);
    self->boot_class = CFCUtil_strdup(boot_class);
    self->header     = CFCUtil_strdup(header);
    self->footer     = CFCUtil_strdup(footer);

    // Derive path to generated .xs file.
    self->xs_path = CFCUtil_sprintf("%s" CHY_DIR_SEP "%s.xs", lib_dir,
                                    boot_class);
    S_replace_double_colons(self->xs_path, CHY_DIR_SEP_CHAR);

    // Derive the name of the bootstrap function.
    self->boot_func = CFCUtil_sprintf("cfish_%s_bootstrap", boot_class);
    for (int i = 0; self->boot_func[i] != 0; i++) {
        if (!isalnum(self->boot_func[i])) {
            self->boot_func[i] = '_';
        }
    }

    return self;
}

void
CFCPerl_destroy(CFCPerl *self) {
    CFCBase_decref((CFCBase*)self->hierarchy);
    FREEMEM(self->lib_dir);
    FREEMEM(self->boot_class);
    FREEMEM(self->header);
    FREEMEM(self->footer);
    FREEMEM(self->xs_path);
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
        char *pod_path = CFCUtil_sprintf("%s" CHY_DIR_SEP "%s.pod",
                                         self->lib_dir, class_name);
        S_replace_double_colons(pod_path, CHY_DIR_SEP_CHAR);

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
    char *guard = CFCUtil_sprintf("%s_BOOT", self->boot_class);
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
    char *content
        = CFCUtil_sprintf(pattern, self->header, guard, guard, self->boot_func,
                          guard, self->footer);

    const char *inc_dest = CFCHierarchy_get_include_dest(self->hierarchy);
    char *boot_h_path = CFCUtil_sprintf("%s" CHY_DIR_SEP "boot.h", inc_dest);
    CFCUtil_write_file(boot_h_path, content, strlen(content));
    FREEMEM(boot_h_path);

    FREEMEM(content);
    FREEMEM(guard);
}

static void
S_write_boot_c(CFCPerl *self) {
    CFCClass  **ordered   = CFCHierarchy_ordered_classes(self->hierarchy);
    CFCParcel **parcels   = CFCParcel_all_parcels();
    char *pound_includes  = CFCUtil_strdup("");
    char *bootstrap_code  = CFCUtil_strdup("");
    char *alias_adds      = CFCUtil_strdup("");
    char *isa_pushes      = CFCUtil_strdup("");

    for (size_t i = 0; parcels[i]; ++i) {
        if (!CFCParcel_included(parcels[i])) {
            const char *prefix = CFCParcel_get_prefix(parcels[i]);
            bootstrap_code = CFCUtil_cat(bootstrap_code, "    ", prefix,
                                         "bootstrap_parcel();\n", NULL);
        }
    }

    for (size_t i = 0; ordered[i] != NULL; i++) {
        CFCClass *klass = ordered[i];
        if (CFCClass_included(klass)) { continue; }

        const char *class_name = CFCClass_get_class_name(klass);
        const char *include_h  = CFCClass_include_h(klass);
        pound_includes = CFCUtil_cat(pound_includes, "#include \"",
                                     include_h, "\"\n", NULL);

        if (CFCClass_inert(klass)) { continue; }

        // Add aliases for selected KinoSearch classes which allow old indexes
        // to be read.
        CFCPerlClass *class_binding = CFCPerlClass_singleton(class_name);
        if (class_binding) {
            const char *vtable_var = CFCClass_full_vtable_var(klass);
            const char **aliases
                = CFCPerlClass_get_class_aliases(class_binding);
            for (size_t j = 0; aliases[j] != NULL; j++) {
                const char *alias = aliases[j];
                size_t alias_len  = strlen(alias);
                const char pattern[] =
                    "    cfish_VTable_add_alias_to_registry("
                    "%s, \"%s\", %u);\n";
                char *alias_add
                    = CFCUtil_sprintf(pattern, vtable_var, alias,
                                      (unsigned)alias_len);
                alias_adds = CFCUtil_cat(alias_adds, alias_add, NULL);
                FREEMEM(alias_add);
            }

            char *metadata_code
                = CFCPerlClass_method_metadata_code(class_binding);
            alias_adds = CFCUtil_cat(alias_adds, metadata_code, NULL);
            FREEMEM(metadata_code);
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
        "#include \"cfish_parcel.h\"\n"
        "#include \"EXTERN.h\"\n"
        "#include \"perl.h\"\n"
        "#include \"XSUB.h\"\n"
        "#include \"boot.h\"\n"
        "#include \"Clownfish/String.h\"\n"
        "#include \"Clownfish/VTable.h\"\n"
        "%s\n"
        "\n"
        "void\n"
        "%s() {\n"
        "%s"
        "\n"
        "%s"
        "\n"
        "    AV *isa;\n"
        "%s"
        "}\n"
        "\n"
        "%s\n"
        "\n";
    char *content
        = CFCUtil_sprintf(pattern, self->header, pound_includes,
                          self->boot_func, bootstrap_code, alias_adds,
                          isa_pushes, self->footer);

    const char *src_dest = CFCHierarchy_get_source_dest(self->hierarchy);
    char *boot_c_path = CFCUtil_sprintf("%s" CHY_DIR_SEP "boot.c", src_dest);
    CFCUtil_write_file(boot_c_path, content, strlen(content));
    FREEMEM(boot_c_path);

    FREEMEM(content);
    FREEMEM(isa_pushes);
    FREEMEM(alias_adds);
    FREEMEM(bootstrap_code);
    FREEMEM(pound_includes);
    FREEMEM(parcels);
    FREEMEM(ordered);
}

void
CFCPerl_write_hostdefs(CFCPerl *self) {
    const char pattern[] =
        "%s\n"
        "\n"
        "#ifndef H_CFISH_HOSTDEFS\n"
        "#define H_CFISH_HOSTDEFS 1\n"
        "\n"
        "/* Refcount / host object */\n"
        "typedef union {\n"
        "    size_t  count;\n"
        "    void   *host_obj;\n"
        "} cfish_ref_t;\n"
        "\n"
        "#define CFISH_OBJ_HEAD\\\n"
        "   cfish_ref_t ref;\n"
        "\n"
        "#endif /* H_CFISH_HOSTDEFS */\n"
        "\n"
        "%s\n";
    char *content
        = CFCUtil_sprintf(pattern, self->header, self->footer);

    // Unlink then write file.
    const char *inc_dest = CFCHierarchy_get_include_dest(self->hierarchy);
    char *filepath = CFCUtil_sprintf("%s" CHY_DIR_SEP "cfish_hostdefs.h",
                                     inc_dest);
    remove(filepath);
    CFCUtil_write_file(filepath, content, strlen(content));
    FREEMEM(filepath);

    FREEMEM(content);
}

void
CFCPerl_write_boot(CFCPerl *self) {
    S_write_boot_h(self);
    S_write_boot_c(self);
}

static char*
S_xs_file_contents(CFCPerl *self, const char *generated_xs,
                   const char *xs_init, const char *hand_rolled_xs) {
    const char pattern[] =
        "%s"
        "\n"
        "#include \"XSBind.h\"\n"
        "#include \"boot.h\"\n"
        "\n"
        "%s\n"
        "\n"
        "MODULE = %s   PACKAGE = %s\n"
        "\n"
        "void\n"
        "_init_autobindings()\n"
        "PPCODE:\n"
        "{\n"
        "    const char* file = __FILE__;\n"
        "    CFISH_UNUSED_VAR(cv);\n"
        "    CFISH_UNUSED_VAR(items); %s\n"
        "}\n"
        "\n"
        "%s\n"
        "\n"
        "%s";
    char *contents
        = CFCUtil_sprintf(pattern, self->header, generated_xs,
                          self->boot_class, self->boot_class, xs_init,
                          hand_rolled_xs, self->footer);

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

void
CFCPerl_write_bindings(CFCPerl *self) {
    CFCClass **ordered = CFCHierarchy_ordered_classes(self->hierarchy);
    CFCPerlClass **registry = CFCPerlClass_registry();
    char *hand_rolled_xs   = CFCUtil_strdup("");
    char *generated_xs     = CFCUtil_strdup("");
    char *xs_init          = CFCUtil_strdup("");

    // Pound-includes for generated headers.
    for (size_t i = 0; ordered[i] != NULL; i++) {
        CFCClass *klass = ordered[i];
        // TODO: Don't include headers for parcels the source parcels don't
        // depend on.
        const char *include_h = CFCClass_include_h(klass);
        generated_xs = CFCUtil_cat(generated_xs, "#include \"", include_h,
                                   "\"\n", NULL);
    }
    generated_xs = CFCUtil_cat(generated_xs, "\n", NULL);

    for (size_t i = 0; ordered[i] != NULL; i++) {
        CFCClass *klass = ordered[i];
        if (CFCClass_included(klass)) { continue; }

        // Constructors.
        CFCPerlConstructor **constructors
            = CFCPerlClass_constructor_bindings(klass);
        for (size_t j = 0; constructors[j] != NULL; j++) {
            CFCPerlSub *xsub = (CFCPerlSub*)constructors[j];

            // Add the XSUB function definition.
            char *xsub_def = CFCPerlConstructor_xsub_def(constructors[j]);
            generated_xs = CFCUtil_cat(generated_xs, xsub_def, "\n",
                                       NULL);
            FREEMEM(xsub_def);

            // Add XSUB initialization at boot.
            xs_init = S_add_xs_init(xs_init, xsub);
        }
        FREEMEM(constructors);

        // Methods.
        CFCPerlMethod **methods = CFCPerlClass_method_bindings(klass);
        for (size_t j = 0; methods[j] != NULL; j++) {
            CFCPerlSub *xsub = (CFCPerlSub*)methods[j];

            // Add the XSUB function definition.
            char *xsub_def = CFCPerlMethod_xsub_def(methods[j]);
            generated_xs = CFCUtil_cat(generated_xs, xsub_def, "\n",
                                       NULL);
            FREEMEM(xsub_def);

            // Add XSUB initialization at boot.
            xs_init = S_add_xs_init(xs_init, xsub);
        }
        FREEMEM(methods);
    }

    // Hand-rolled XS.
    for (size_t i = 0; registry[i] != NULL; i++) {
        const char *xs = CFCPerlClass_get_xs_code(registry[i]);
        hand_rolled_xs = CFCUtil_cat(hand_rolled_xs, xs, "\n", NULL);
    }

    // Write out if there have been any changes.
    char *xs_file_contents
        = S_xs_file_contents(self, generated_xs, xs_init, hand_rolled_xs);
    CFCUtil_write_if_changed(self->xs_path, xs_file_contents,
                             strlen(xs_file_contents));

    FREEMEM(xs_file_contents);
    FREEMEM(hand_rolled_xs);
    FREEMEM(xs_init);
    FREEMEM(generated_xs);
    FREEMEM(ordered);
}

void
CFCPerl_write_callbacks(CFCPerl *self) {
    CFCBindCore *core_binding
        = CFCBindCore_new(self->hierarchy, self->header, self->footer);
    CFCBindCore_write_callbacks_h(core_binding);
    CFCBase_decref((CFCBase*)core_binding);

    S_write_callbacks_c(self);
}

static void
S_write_callbacks_c(CFCPerl *self) {
    CFCClass **ordered = CFCHierarchy_ordered_classes(self->hierarchy);
    static const char pattern[] =
        "%s"
        "\n"
        "#include \"XSBind.h\"\n"
        "#include \"callbacks.h\"\n"
        "\n"
        "static void\n"
        "S_finish_callback_void(const char *meth_name) {\n"
        "    int count = call_method(meth_name, G_VOID | G_DISCARD);\n"
        "    if (count != 0) {\n"
        "        CFISH_THROW(CFISH_ERR, \"Bad callback to '%%s': %%i32\",\n"
        "                    meth_name, (int32_t)count);\n"
        "    }\n"
        "    FREETMPS;\n"
        "    LEAVE;\n"
        "}\n"
        "\n"
        "static CFISH_INLINE SV*\n"
        "SI_do_callback_sv(const char *meth_name) {\n"
        "    int count = call_method(meth_name, G_SCALAR);\n"
        "    if (count != 1) {\n"
        "        CFISH_THROW(CFISH_ERR, \"Bad callback to '%%s': %%i32\",\n"
        "                    meth_name, (int32_t)count);\n"
        "    }\n"
        "    dSP;\n"
        "    SV *return_sv = POPs;\n"
        "    PUTBACK;\n"
        "    return return_sv;\n"
        "}\n"
        "\n"
        "static int64_t\n"
        "S_finish_callback_i64(const char *meth_name) {\n"
        "    SV *return_sv = SI_do_callback_sv(meth_name);\n"
        "    int64_t retval;\n"
        "    if (sizeof(IV) == 8) {\n"
        "        retval = (int64_t)SvIV(return_sv);\n"
        "    }\n"
        "    else {\n"
        "        if (SvIOK(return_sv)) {\n"
        "            // It's already no more than 32 bits, so don't convert.\n"
        "            retval = SvIV(return_sv);\n"
        "        }\n"
        "        else {\n"
        "            // Maybe lossy.\n"
        "            double temp = SvNV(return_sv);\n"
        "            retval = (int64_t)temp;\n"
        "        }\n"
        "    }\n"
        "    FREETMPS;\n"
        "    LEAVE;\n"
        "    return retval;\n"
        "}\n"
        "\n"
        "static double\n"
        "S_finish_callback_f64(const char *meth_name) {\n"
        "    SV *return_sv = SI_do_callback_sv(meth_name);\n"
        "    double retval = SvNV(return_sv);\n"
        "    FREETMPS;\n"
        "    LEAVE;\n"
        "    return retval;\n"
        "}\n"
        "\n"
        "static cfish_Obj*\n"
        "S_finish_callback_obj(void *vself, const char *meth_name,\n"
        "                      int nullable) {\n"
        "    SV *return_sv = SI_do_callback_sv(meth_name);\n"
        "    cfish_Obj *retval = XSBind_perl_to_cfish(return_sv);\n"
        "    FREETMPS;\n"
        "    LEAVE;\n"
        "    if (!nullable && !retval) {\n"
        "        CFISH_THROW(CFISH_ERR, \"%%o#%%s cannot return NULL\",\n"
        "                    CFISH_Obj_Get_Class_Name((cfish_Obj*)vself),\n"
        "                    meth_name);\n"
        "    }\n"
        "    return retval;\n"
        "}\n"
        "\n";
    char *content = CFCUtil_sprintf(pattern, self->header);

    for (size_t i = 0; ordered[i] != NULL; i++) {
        CFCClass *klass = ordered[i];
        if (CFCClass_inert(klass)) { continue; }

        CFCMethod **fresh_methods = CFCClass_fresh_methods(klass);
        for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
            CFCMethod *method = fresh_methods[meth_num];

            // Define callback.
            if (CFCMethod_novel(method) && !CFCMethod_final(method)) {
                char *cb_def = CFCPerlMethod_callback_def(method);
                content = CFCUtil_cat(content, cb_def, "\n", NULL);
                FREEMEM(cb_def);
            }
        }
        FREEMEM(fresh_methods);
    }

    content = CFCUtil_cat(content, self->footer, NULL);

    // Write if changed.
    const char *src_dest = CFCHierarchy_get_source_dest(self->hierarchy);
    char *filepath = CFCUtil_sprintf("%s" CHY_DIR_SEP "callbacks.c",
                                     src_dest);
    CFCUtil_write_if_changed(filepath, content, strlen(content));

    FREEMEM(filepath);
    FREEMEM(content);
    FREEMEM(ordered);
}

void
CFCPerl_write_xs_typemap(CFCPerl *self) {
    CFCPerlTypeMap_write_xs_typemap(self->hierarchy);
}

