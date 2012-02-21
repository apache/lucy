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
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerlClass.h"
#include "CFCUtil.h"
#include "CFCClass.h"
#include "CFCMethod.h"
#include "CFCParcel.h"
#include "CFCParamList.h"
#include "CFCFunction.h"
#include "CFCDocuComment.h"
#include "CFCSymbol.h"
#include "CFCVariable.h"
#include "CFCType.h"
#include "CFCPerlPod.h"
#include "CFCPerlMethod.h"
#include "CFCPerlConstructor.h"
#include "CFCPerlTypeMap.h"

struct CFCPerlClass {
    CFCBase base;
    CFCParcel *parcel;
    char *class_name;
    CFCClass *client;
    char *xs_code;
    CFCPerlPod *pod_spec;
    char **meth_aliases;
    char **meth_names;
    size_t num_methods;
    char **excluded;
    size_t num_excluded;
    char **cons_aliases;
    char **cons_inits;
    size_t num_cons;
    int    exclude_cons;
    char **class_aliases;
    size_t num_class_aliases;
};

static CFCPerlClass **registry = NULL;
static size_t registry_size = 0;
static size_t registry_cap  = 0;

const static CFCMeta CFCPERLCLASS_META = {
    "Clownfish::CFC::Binding::Perl::Class",
    sizeof(CFCPerlClass),
    (CFCBase_destroy_t)CFCPerlClass_destroy
};

CFCPerlClass*
CFCPerlClass_new(CFCParcel *parcel, const char *class_name) {
    CFCPerlClass *self = (CFCPerlClass*)CFCBase_allocate(&CFCPERLCLASS_META);
    return CFCPerlClass_init(self, parcel, class_name);
}

CFCPerlClass*
CFCPerlClass_init(CFCPerlClass *self, CFCParcel *parcel,
                  const char *class_name) {
    CFCUTIL_NULL_CHECK(parcel);
    CFCUTIL_NULL_CHECK(class_name);
    self->parcel = (CFCParcel*)CFCBase_incref((CFCBase*)parcel);
    self->class_name = CFCUtil_strdup(class_name);
    // Client may be NULL, since fetch_singleton() does not always succeed.
    self->client = CFCClass_fetch_singleton(parcel, class_name); 
    self->pod_spec          = NULL;
    self->xs_code           = NULL;
    self->meth_aliases      = NULL;
    self->meth_names        = NULL;
    self->num_methods       = 0;
    self->excluded          = NULL;
    self->num_excluded      = 0;
    self->cons_aliases      = NULL;
    self->cons_inits        = NULL;
    self->num_cons          = 0;
    self->exclude_cons      = 0;
    self->class_aliases     = (char**)CALLOCATE(1, sizeof(char*));
    self->num_class_aliases = 0;
    return self;
}

void
CFCPerlClass_destroy(CFCPerlClass *self) {
    CFCBase_decref((CFCBase*)self->parcel);
    CFCBase_decref((CFCBase*)self->client);
    CFCBase_decref((CFCBase*)self->pod_spec);
    FREEMEM(self->class_name);
    FREEMEM(self->xs_code);
    for (size_t i = 0; i < self->num_methods; i++) {
        FREEMEM(self->meth_aliases[i]);
        FREEMEM(self->meth_names[i]);
    }
    FREEMEM(self->meth_aliases);
    FREEMEM(self->meth_names);
    for (size_t i = 0; i < self->num_excluded; i++) {
        FREEMEM(self->excluded[i]);
    }
    FREEMEM(self->excluded);
    for (size_t i = 0; i < self->num_cons; i++) {
        FREEMEM(self->cons_aliases[i]);
        FREEMEM(self->cons_inits[i]);
    }
    FREEMEM(self->cons_aliases);
    FREEMEM(self->cons_inits);
    for (size_t i = 0; i < self->num_class_aliases; i++) {
        FREEMEM(self->class_aliases[i]);
    }
    FREEMEM(self->class_aliases);
    CFCBase_destroy((CFCBase*)self);
}

static int
S_compare_cfcperlclass(const void *va, const void *vb) {
    CFCPerlClass *a = *(CFCPerlClass**)va;
    CFCPerlClass *b = *(CFCPerlClass**)vb;
    return strcmp(a->class_name, b->class_name);
}

void
CFCPerlClass_add_to_registry(CFCPerlClass *self) {
    if (registry_size == registry_cap) {
        size_t new_cap = registry_cap + 10;
        registry = (CFCPerlClass**)REALLOCATE(registry,
                                              (new_cap + 1) * sizeof(CFCPerlClass*));
        for (size_t i = registry_cap; i <= new_cap; i++) {
            registry[i] = NULL;
        }
        registry_cap = new_cap;
    }
    CFCPerlClass *existing = CFCPerlClass_singleton(self->class_name);
    if (existing) {
        CFCUtil_die("Class '%s' already registered", self->class_name);
    }
    registry[registry_size] = (CFCPerlClass*)CFCBase_incref((CFCBase*)self);
    registry_size++;
    qsort(registry, registry_size, sizeof(CFCPerlClass*),
          S_compare_cfcperlclass);
}

CFCPerlClass*
CFCPerlClass_singleton(const char *class_name) {
    CFCUTIL_NULL_CHECK(class_name);
    for (size_t i = 0; i < registry_size; i++) {
        CFCPerlClass *existing = registry[i];
        if (strcmp(class_name, existing->class_name) == 0) {
            return existing;
        }
    }
    return NULL;
}

CFCPerlClass**
CFCPerlClass_registry() {
    return registry;
}

void
CFCPerlClass_clear_registry(void) {
    for (size_t i = 0; i < registry_size; i++) {
        CFCBase_decref((CFCBase*)registry[i]);
    }
    FREEMEM(registry);
    registry_size = 0;
    registry_cap  = 0;
    registry      = NULL;
}

void
CFCPerlClass_bind_method(CFCPerlClass *self, const char *alias,
                         const char *method) {
    size_t size = (self->num_methods + 1) * sizeof(char*);
    self->meth_aliases = (char**)REALLOCATE(self->meth_aliases, size);
    self->meth_names   = (char**)REALLOCATE(self->meth_names,   size);
    self->meth_aliases[self->num_methods] = (char*)CFCUtil_strdup(alias);
    self->meth_names[self->num_methods]   = (char*)CFCUtil_strdup(method);
    self->num_methods++;
    if (!self->client) {
        CFCUtil_die("Can't bind_method %s -- can't find client for %s",
                    alias, self->class_name);
    }
}

void
CFCPerlClass_exclude_method(CFCPerlClass *self, const char *method) {
    size_t size = (self->num_excluded + 1) * sizeof(char*);
    self->excluded = (char**)REALLOCATE(self->excluded, size);
    self->excluded[self->num_excluded] = (char*)CFCUtil_strdup(method);
    self->num_excluded++;
}

void
CFCPerlClass_bind_constructor(CFCPerlClass *self, const char *alias,
                              const char *initializer) {
    alias       = alias       ? alias       : "new";
    initializer = initializer ? initializer : "init";
    size_t size = (self->num_cons + 1) * sizeof(char*);
    self->cons_aliases = (char**)REALLOCATE(self->cons_aliases, size);
    self->cons_inits   = (char**)REALLOCATE(self->cons_inits,   size);
    self->cons_aliases[self->num_cons] = (char*)CFCUtil_strdup(alias);
    self->cons_inits[self->num_cons]   = (char*)CFCUtil_strdup(initializer);
    self->num_cons++;
    if (!self->client) {
        CFCUtil_die("Can't bind_constructor %s -- can't find client for %s",
                    alias, self->class_name);
    }
}

void
CFCPerlClass_exclude_constructor(CFCPerlClass *self) {
    self->exclude_cons = 1;
}

static int
S_can_be_bound(CFCParamList *param_list, CFCType *return_type) {
    int success = 1;
    CFCVariable **arg_vars = CFCParamList_get_variables(param_list);

    for (size_t i = 0; arg_vars[i] != NULL; i++) {
        CFCType *type = CFCVariable_get_type(arg_vars[i]);
        char *conversion = CFCPerlTypeMap_from_perl(type, "foo");
        if (conversion) { FREEMEM(conversion); }
        else            { success = 0; }
    }
    if (!CFCType_is_void(return_type)) {
        char *conversion = CFCPerlTypeMap_to_perl(return_type, "foo");
        if (conversion) { FREEMEM(conversion); }
        else            { success = 0; }
    }

    return success;
}

CFCPerlMethod**
CFCPerlClass_method_bindings(CFCPerlClass *self) {
    CFCClass       *client     = self->client;
    const char     *class_name = self->class_name;
    size_t          num_bound  = 0;
    CFCMethod     **fresh_methods = CFCClass_fresh_methods(client);
    CFCClass      **descendants   = CFCClass_tree_to_ladder(client);
    CFCPerlMethod **bound 
        = (CFCPerlMethod**)CALLOCATE(1, sizeof(CFCPerlMethod*));
 
     // Iterate over the class's fresh methods.
    for (size_t i = 0; fresh_methods[i] != NULL; i++) {
        CFCMethod  *method    = fresh_methods[i];
        const char *alias     = CFCMethod_micro_sym(method);
        const char *meth_name = CFCMethod_get_macro_sym(method);

        // Only deal with methods when they are novel (i.e. first declared).
        if (!CFCMethod_novel(method)) { continue; }

        // Skip private methods.
        if (CFCSymbol_private((CFCSymbol*)method)) { continue; }

        // Skip methods which have been explicitly excluded.
        int is_excluded = 0;
        for (size_t j = 0; j < self->num_excluded; j++) {
            if (strcmp(self->excluded[j], meth_name) == 0) {
                is_excluded = 1;
                break;
            }
        }
        if (is_excluded) { continue; }

        // Skip methods with types which cannot be mapped automatically.
        CFCParamList *param_list  = CFCMethod_get_param_list(method);
        CFCType      *return_type = CFCMethod_get_return_type(method);
        if (!S_can_be_bound(param_list, return_type)) {
            continue;
        }

        // See if the user wants the method to have a specific alias.
        for (size_t j = 0; j < self->num_methods; j++) {
            const char *maybe = self->meth_names[j];
            if (strcmp(maybe, meth_name) == 0) {
                if (self->meth_aliases[j]) {
                    alias = self->meth_aliases[j];
                }
            }
        }

        /* Create an XSub binding for each override.  Each of these directly
         * calls the implementing function, rather than invokes the method on
         * the object using VTable method dispatch.  Doing things this way
         * allows SUPER:: invocations from Perl-space to work properly.
         */
        for (size_t j = 0; descendants[j] != NULL; j++) {
            CFCClass *descendant = descendants[j];
            CFCMethod *real_method
                = CFCClass_fresh_method(descendant, meth_name);
            if (!real_method) { continue; }

            // Create the binding, add it to the array.
            CFCPerlMethod *meth_binding = CFCPerlMethod_new(real_method, alias);
            size_t size = (num_bound + 2) * sizeof(CFCPerlMethod*);
            bound = (CFCPerlMethod**)REALLOCATE(bound, size);
            bound[num_bound] = meth_binding;
            num_bound++;
            bound[num_bound] = NULL;
        }

    }

    FREEMEM(fresh_methods);
    FREEMEM(descendants);

    return bound;
}

static const char NEW[] = "new";

CFCPerlConstructor**
CFCPerlClass_constructor_bindings(CFCPerlClass *self) {
    CFCClass     *client    = self->client;
    CFCFunction **functions = CFCClass_functions(self->client);
    size_t        num_bound = 0;
    CFCPerlConstructor **bound 
        = (CFCPerlConstructor**)CALLOCATE(1, sizeof(CFCPerlConstructor*));

    // Iterate over the list of possible initialization functions.
    for (size_t i = 0; functions[i] != NULL; i++) {
        CFCFunction  *function    = functions[i];
        const char   *micro_sym   = CFCFunction_micro_sym(function);
        CFCParamList *param_list  = CFCFunction_get_param_list(function);
        CFCType      *return_type = CFCFunction_get_return_type(function);
        const char   *alias       = NULL;

        // Find user-specified alias.
        for (size_t j = 0; j < self->num_cons; j++) {
            if (strcmp(micro_sym, self->cons_inits[j]) == 0) {
                alias = self->cons_aliases[j];
                if (!S_can_be_bound(param_list, return_type)) {
                    CFCUtil_die("Can't bind %s as %s -- types can't be mapped",
                                micro_sym, alias);
                }
                break;
            }
        }

        // Automatically bind init() to new() when possible.
        if (!alias
            && !self->exclude_cons
            && strcmp(micro_sym, "init") == 0
            && S_can_be_bound(param_list, return_type)
           ) {
            int saw_new = 0;
            for (size_t j = 0; j < self->num_cons; j++) {
                if (strcmp(self->cons_aliases[j], "new") == 0) {
                    saw_new = 1;
                }
            }
            if (!saw_new) {
                alias = NEW;
            }
        }

        if (!alias) {
            continue;
        }

        // Create the binding, add it to the array.
        CFCPerlConstructor *cons_binding
            = CFCPerlConstructor_new(client, alias, micro_sym);
        size_t size = (num_bound + 2) * sizeof(CFCPerlConstructor*);
        bound = (CFCPerlConstructor**)REALLOCATE(bound, size);
        bound[num_bound] = cons_binding;
        num_bound++;
        bound[num_bound] = NULL;
    }

    return bound;
}

char*
CFCPerlClass_create_pod(CFCPerlClass *self) {
    CFCPerlPod *pod_spec   = self->pod_spec;
    const char *class_name = self->class_name;
    CFCClass   *client     = self->client;
    if (!pod_spec) {
        return NULL;
    }
    if (!client) {
        CFCUtil_die("No client for %s", class_name);
    }
    CFCDocuComment *docucom = CFCClass_get_docucomment(client);
    if (!docucom) {
        CFCUtil_die("No DocuComment for %s", class_name);
    }

    // Get the class's brief description.
    const char *raw_brief = CFCDocuComment_get_brief(docucom);
    char *brief = CFCPerlPod_perlify_doc_text(pod_spec, raw_brief);

    // Get the class's long description.
    const char *raw_description = CFCPerlPod_get_description(pod_spec);
    if (!raw_description || !strlen(raw_description)) {
        raw_description = CFCDocuComment_get_long(docucom);
    }
    char *description = CFCPerlPod_perlify_doc_text(pod_spec, raw_description);

    // Create SYNOPSIS.
    const char *raw_synopsis = CFCPerlPod_get_synopsis(pod_spec);
    char *synopsis = CFCUtil_strdup("");
    if (raw_synopsis && strlen(raw_synopsis)) {
        synopsis = CFCUtil_cat(synopsis, "=head1 SYNOPSIS\n\n", raw_synopsis,
                               "\n", NULL);
    }

    // Create CONSTRUCTORS.
    char *constructor_pod = CFCPerlPod_constructors_pod(pod_spec, client);

    // Create METHODS, possibly including an ABSTRACT METHODS section.
    char *methods_pod = CFCPerlPod_methods_pod(pod_spec, client);

    // Build an INHERITANCE section describing class ancestry.
    char *inheritance = CFCUtil_strdup("");
    if (CFCClass_get_parent(client)) {
        inheritance = CFCUtil_cat(inheritance, "=head1 INHERITANCE\n\n",
                                  class_name, NULL);
        CFCClass *ancestor = client;
        while (NULL != (ancestor = CFCClass_get_parent(ancestor))) {
            const char *ancestor_klass = CFCClass_get_class_name(ancestor);
            if (CFCPerlClass_singleton(ancestor_klass)) {
                inheritance = CFCUtil_cat(inheritance, " isa L<",
                                          ancestor_klass, ">", NULL);
            }
            else {
                inheritance = CFCUtil_cat(inheritance, " isa ",
                                          ancestor_klass, NULL);
            }
        }
        inheritance = CFCUtil_cat(inheritance, ".\n", NULL);
    }

    // Put it all together.
    const char pattern[] = 
    "# Auto-generated file -- DO NOT EDIT!!!!!\n"
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
    "=head1 NAME\n"
    "\n"
    "%s - %s\n"
    "\n"
    "%s\n"
    "\n"
    "=head1 DESCRIPTION\n"
    "\n"
    "%s\n"
    "\n"
    "%s\n"
    "\n"
    "%s\n"
    "\n"
    "%s\n"
    "\n"
    "=cut\n"
    "\n";

    size_t size = sizeof(pattern)
                  + strlen(class_name)
                  + strlen(brief)
                  + strlen(synopsis)
                  + strlen(description)
                  + strlen(constructor_pod)
                  + strlen(methods_pod)
                  + strlen(inheritance)
                  + 20;

    char *pod = (char*)MALLOCATE(size);
    sprintf(pod, pattern, class_name, brief, synopsis, description,
            constructor_pod, methods_pod, inheritance);

    FREEMEM(brief);
    FREEMEM(synopsis);
    FREEMEM(description);
    FREEMEM(constructor_pod);
    FREEMEM(methods_pod);
    FREEMEM(inheritance);

    return pod;
}

CFCClass*
CFCPerlClass_get_client(CFCPerlClass *self) {
    return self->client;
}

const char*
CFCPerlClass_get_class_name(CFCPerlClass *self) {
    return self->class_name;
}

void
CFCPerlClass_append_xs(CFCPerlClass *self, const char *xs) {
    if (!self->xs_code) {
        self->xs_code = CFCUtil_strdup("");
    }
    self->xs_code = CFCUtil_cat(self->xs_code, xs, NULL);
}

const char*
CFCPerlClass_get_xs_code(CFCPerlClass *self) {
    return self->xs_code;
}

void
CFCPerlClass_set_pod_spec(CFCPerlClass *self, CFCPerlPod *pod_spec) {
    CFCPerlPod *old_pod_spec = self->pod_spec;
    self->pod_spec = (CFCPerlPod*)CFCBase_incref((CFCBase*)pod_spec);
    CFCBase_decref((CFCBase*)old_pod_spec);
}

CFCPerlPod*
CFCPerlClass_get_pod_spec(CFCPerlClass *self) {
    return self->pod_spec;
}

void
CFCPerlClass_add_class_alias(CFCPerlClass *self, const char *alias) {
    for (size_t i = 0; i < self->num_class_aliases; i++) {
        if (strcmp(alias, self->class_aliases[i]) == 0) {
            CFCUtil_die("Alias '%s' already added for class '%s'", alias,
                        self->class_name);
        }
    }
    size_t size = (self->num_class_aliases + 2) * sizeof(char*);
    self->class_aliases = (char**)REALLOCATE(self->class_aliases, size);
    self->class_aliases[self->num_class_aliases] = CFCUtil_strdup(alias);
    self->num_class_aliases++;
    self->class_aliases[self->num_class_aliases] = NULL;
}

char**
CFCPerlClass_get_class_aliases(CFCPerlClass *self) {
    return self->class_aliases;
}

