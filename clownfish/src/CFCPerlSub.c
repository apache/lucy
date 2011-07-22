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
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerlSub.h"
#include "CFCUtil.h"
#include "CFCParamList.h"
#include "CFCVariable.h"

struct CFCPerlSub {
    CFCBase base;
    CFCParamList *param_list;
    char *class_name;
    char *alias;
    int use_labeled_params;
    char *perl_name;
    char *c_name;
};

CFCPerlSub*
CFCPerlSub_new(const char *klass, CFCParamList *param_list,
               const char *class_name, const char *alias,
               int use_labeled_params) {
    CFCPerlSub *self
        = (CFCPerlSub*)CFCBase_allocate(sizeof(CFCPerlSub), klass);
    return CFCPerlSub_init(self, param_list, class_name, alias,
                           use_labeled_params);
}

CFCPerlSub*
CFCPerlSub_init(CFCPerlSub *self, CFCParamList *param_list,
                const char *class_name, const char *alias,
                int use_labeled_params) {
    CFCUTIL_NULL_CHECK(param_list);
    CFCUTIL_NULL_CHECK(class_name);
    CFCUTIL_NULL_CHECK(alias);
    self->param_list  = (CFCParamList*)CFCBase_incref((CFCBase*)param_list);
    self->class_name  = CFCUtil_strdup(class_name);
    self->alias       = CFCUtil_strdup(alias);
    self->use_labeled_params = use_labeled_params;
    self->perl_name = CFCUtil_cat(CFCUtil_strdup(class_name), "::", alias,
                                  NULL);

    size_t c_name_len = strlen(self->perl_name) + sizeof("XS_") + 1;
    self->c_name = (char*)MALLOCATE(c_name_len);
    int j = 3;
    memcpy(self->c_name, "XS_", j);
    for (int i = 0, max = strlen(self->perl_name); i < max; i++) {
        char c = self->perl_name[i];
        if (c == ':') {
            while (self->perl_name[i + 1] == ':') { i++; }
            self->c_name[j++] = '_';
        }
        else {
            self->c_name[j++] = c;
        }
    }
    self->c_name[j] = 0; // NULL-terminate.

    return self;
}

void
CFCPerlSub_destroy(CFCPerlSub *self) {
    CFCBase_decref((CFCBase*)self->param_list);
    FREEMEM(self->class_name);
    FREEMEM(self->alias);
    FREEMEM(self->perl_name);
    FREEMEM(self->c_name);
    CFCBase_destroy((CFCBase*)self);
}

char*
CFCPerlSub_params_hash_def(CFCPerlSub *self)
{
    if (!self->use_labeled_params) {
        return NULL;
    }

    char *def = CFCUtil_strdup("");
    def = CFCUtil_cat(def, "%", self->perl_name, "_PARAMS = (", NULL);

    CFCVariable **arg_vars = CFCParamList_get_variables(self->param_list);
    const char **vals = CFCParamList_get_initial_values(self->param_list);

    // No labeled params means an empty params hash def.
    if (!arg_vars[1]) {
        def = CFCUtil_cat(def, ");\n", NULL);
        return def;
    }

    for (int i = 1; arg_vars[i] != NULL; i++) {
        CFCVariable *var = arg_vars[i];
        const char *micro_sym = CFCVariable_micro_sym(var);
        const char *val = vals[i];
        val = val == NULL
              ? "undef"
              : strcmp(val, "NULL") == 0
              ? "undef"
              : strcmp(val, "true") == 0
              ? "1"
              : strcmp(val, "false") == 0
              ? "0"
              : val;
        def = CFCUtil_cat(def, "\n    ", micro_sym, " => ", val, ",", NULL);
    }
    def = CFCUtil_cat(def, "\n);\n", NULL);

    return def;
}

CFCParamList*
CFCPerlSub_get_param_list(CFCPerlSub *self) {
    return self->param_list;
}

const char*
CFCPerlSub_get_class_name(CFCPerlSub *self) {
    return self->class_name;
}

const char*
CFCPerlSub_get_alias(CFCPerlSub *self) {
    return self->alias;
}

int
CFCPerlSub_use_labeled_params(CFCPerlSub *self) {
    return self->use_labeled_params;
}

const char*
CFCPerlSub_perl_name(CFCPerlSub *self) {
    return self->perl_name;
}

const char*
CFCPerlSub_c_name(CFCPerlSub *self) {
    return self->c_name;
}

const char*
CFCPerlSub_c_name_list(CFCPerlSub *self) {
    return CFCParamList_name_list(self->param_list);
}

