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

#define CFC_NEED_PERLSUB_STRUCT_DEF 1
#include "CFCPerlSub.h"
#include "CFCPerlConstructor.h"
#include "CFCClass.h"
#include "CFCFunction.h"
#include "CFCParamList.h"
#include "CFCType.h"
#include "CFCVariable.h"
#include "CFCUtil.h"
#include "CFCPerlTypeMap.h"

struct CFCPerlConstructor {
    CFCPerlSub   sub;
    CFCFunction *init_func;
};

const static CFCMeta CFCPERLCONSTRUCTOR_META = {
    "Clownfish::CFC::Binding::Perl::Constructor",
    sizeof(CFCPerlConstructor),
    (CFCBase_destroy_t)CFCPerlConstructor_destroy
};

CFCPerlConstructor*
CFCPerlConstructor_new(CFCClass *klass, const char *alias,
                       const char *initializer) {
    CFCPerlConstructor *self
        = (CFCPerlConstructor*)CFCBase_allocate(&CFCPERLCONSTRUCTOR_META);
    return CFCPerlConstructor_init(self, klass, alias, initializer);
}

CFCPerlConstructor*
CFCPerlConstructor_init(CFCPerlConstructor *self, CFCClass *klass,
                        const char *alias, const char *initializer) {
    CFCUTIL_NULL_CHECK(alias);
    CFCUTIL_NULL_CHECK(klass);
    const char *class_name = CFCClass_get_class_name(klass);
    initializer = initializer ? initializer : "init";

    // Find the implementing function.
    self->init_func = NULL;
    CFCFunction **funcs = CFCClass_functions(klass);
    for (size_t i = 0; funcs[i] != NULL; i++) {
        CFCFunction *func = funcs[i];
        const char *func_name = CFCFunction_micro_sym(func);
        if (strcmp(initializer, func_name) == 0) {
            self->init_func = (CFCFunction*)CFCBase_incref((CFCBase*)func);
            break;
        }
    }
    if (!self->init_func) {
        CFCUtil_die("Missing or invalid '%s' function for '%s'",
                    initializer, class_name);
    }
    CFCParamList *param_list = CFCFunction_get_param_list(self->init_func);
    CFCPerlSub_init((CFCPerlSub*)self, param_list, class_name, alias,
                    true);
    return self;
}

void
CFCPerlConstructor_destroy(CFCPerlConstructor *self) {
    CFCBase_decref((CFCBase*)self->init_func);
    CFCPerlSub_destroy((CFCPerlSub*)self);
}

char*
CFCPerlConstructor_xsub_def(CFCPerlConstructor *self) {
    const char *c_name = self->sub.c_name;
    CFCParamList *param_list = self->sub.param_list;
    const char   *name_list  = CFCParamList_name_list(param_list);
    CFCVariable **arg_vars   = CFCParamList_get_variables(param_list);
    const char   *func_sym   = CFCFunction_full_func_sym(self->init_func);
    char *allot_params = CFCPerlSub_build_allot_params((CFCPerlSub*)self);
    CFCVariable *self_var       = arg_vars[0];
    CFCType     *self_type      = CFCVariable_get_type(self_var);
    const char  *self_type_str  = CFCType_to_c(self_type);

    // Compensate for swallowed refcounts.
    char *refcount_mods = CFCUtil_strdup("");
    for (size_t i = 0; arg_vars[i] != NULL; i++) {
        CFCVariable *var = arg_vars[i];
        CFCType *type = CFCVariable_get_type(var);
        if (CFCType_is_object(type) && CFCType_decremented(type)) {
            const char *name = CFCVariable_micro_sym(var);
            refcount_mods = CFCUtil_cat(refcount_mods, "\n    CFISH_INCREF(",
                                        name, ");", NULL);
        }
    }

    const char pattern[] =
        "XS(%s);\n"
        "XS(%s) {\n"
        "    dXSARGS;\n"
        "    CHY_UNUSED_VAR(cv);\n"
        "    if (items < 1) { CFISH_THROW(CFISH_ERR, \"Usage: %%s(class_name, ...)\",  GvNAME(CvGV(cv))); }\n"
        "    SP -= items;\n"
        "\n"
        "    %s\n"
        // Create "self" last, so that earlier exceptions while fetching
        // params don't trigger a bad invocation of DESTROY.
        "    %s self = (%s)XSBind_new_blank_obj(ST(0));%s\n"
        "\n"
        "    %s retval = %s(%s);\n"
        "    if (retval) {\n"
        "        ST(0) = (SV*)Cfish_Obj_To_Host((cfish_Obj*)retval);\n"
        "        Cfish_Obj_Dec_RefCount((cfish_Obj*)retval);\n"
        "    }\n"
        "    else {\n"
        "        ST(0) = newSV(0);\n"
        "    }\n"
        "    sv_2mortal(ST(0));\n"
        "    XSRETURN(1);\n"
        "}\n\n";
    char *xsub_def
        = CFCUtil_sprintf(pattern, c_name, c_name, allot_params, self_type_str,
                          self_type_str, refcount_mods, self_type_str,
                          func_sym, name_list);

    FREEMEM(refcount_mods);
    FREEMEM(allot_params);

    return xsub_def;
}

