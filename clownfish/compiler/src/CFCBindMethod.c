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

#include <stdio.h>
#include <string.h>
#include "CFCBindMethod.h"
#include "CFCUtil.h"
#include "CFCMethod.h"
#include "CFCFunction.h"
#include "CFCParamList.h"
#include "CFCType.h"
#include "CFCVariable.h"
#include "CFCSymbol.h"
#include "CFCClass.h"

/* Create a macro definition that aliases to a function name directly, since
 * this method may not be overridden. */
static char*
S_final_method_def(CFCMethod *method, CFCClass *klass);

static char*
S_virtual_method_def(CFCMethod *method, CFCClass *klass);

/* Take a NULL-terminated list of CFCVariables and build up a string of
 * directives like:
 *
 *     UNUSED_VAR(var1);
 *     UNUSED_VAR(var2);
 */
static char*
S_build_unused_vars(CFCVariable **vars);

/* Create an unreachable return statement if necessary, in order to thwart
 * compiler warnings. */
static char*
S_maybe_unreachable(CFCType *return_type);

char*
CFCBindMeth_method_def(CFCMethod *method, CFCClass *klass) {
    if (CFCMethod_final(method)) {
        return S_final_method_def(method, klass);
    }
    else {
        return S_virtual_method_def(method, klass);
    }
}

/* Create a macro definition that aliases to a function name directly, since
 * this method may not be overridden. */
static char*
S_final_method_def(CFCMethod *method, CFCClass *klass) {
    const char *self_type = CFCType_to_c(CFCMethod_self_type(method));
    const char *full_func_sym = CFCMethod_implementing_func_sym(method);
    const char *arg_names 
        = CFCParamList_name_list(CFCMethod_get_param_list(method));

    char *full_meth_sym   = CFCMethod_full_method_sym(method, klass);
    char *full_offset_sym = CFCMethod_full_offset_sym(method, klass);

    const char pattern[] =
        "extern size_t %s;\n"
        "#define %s(%s) \\\n"
        "    %s((%s)%s)\n";
    char *method_def
        = CFCUtil_sprintf(pattern, full_offset_sym, full_meth_sym, arg_names,
                          full_func_sym, self_type, arg_names);

    FREEMEM(full_offset_sym);
    FREEMEM(full_meth_sym);
    return method_def;
}

static char*
S_virtual_method_def(CFCMethod *method, CFCClass *klass) {
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    const char *PREFIX         = CFCClass_get_PREFIX(klass);
    const char *invoker_struct = CFCClass_full_struct_sym(klass);
    const char *common_struct 
        = CFCType_get_specifier(CFCMethod_self_type(method));

    char *full_meth_sym   = CFCMethod_full_method_sym(method, klass);
    char *full_offset_sym = CFCMethod_full_offset_sym(method, klass);
    char *full_typedef    = CFCMethod_full_typedef(method, klass);

    // Prepare parameter lists, minus invoker.  The invoker gets forced to
    // "self" later.
    if (CFCParamList_variadic(param_list)) {
        CFCUtil_die("Variadic methods not supported");
    }
    const char *arg_names_minus_invoker = CFCParamList_name_list(param_list);
    const char *params_minus_invoker    = CFCParamList_to_c(param_list);
    while (*arg_names_minus_invoker && *arg_names_minus_invoker != ',') {
        arg_names_minus_invoker++;
    }
    while (*params_minus_invoker && *params_minus_invoker != ',') {
        params_minus_invoker++;
    }

    // Prepare a return statement... or not.
    CFCType *return_type = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    const char *maybe_return = CFCType_is_void(return_type) ? "" : "return ";

    const char pattern[] =
        "extern %sVISIBLE size_t %s;\n"
        "static CFISH_INLINE %s\n"
        "%s(const %s *self%s) {\n"
        "    const %s method = (%s)cfish_obj_method(self, %s);\n"
        "    %smethod((%s*)self%s);\n"
        "}\n";
    char *method_def
        = CFCUtil_sprintf(pattern, PREFIX, full_offset_sym, ret_type_str,
                          full_meth_sym, invoker_struct, params_minus_invoker,
                          full_typedef, full_typedef, full_offset_sym,
                          maybe_return, common_struct,
                          arg_names_minus_invoker);

    FREEMEM(full_offset_sym);
    FREEMEM(full_meth_sym);
    FREEMEM(full_typedef);
    return method_def;
}

char*
CFCBindMeth_typedef_dec(struct CFCMethod *method, CFCClass *klass) {
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    const char *ret_type = CFCType_to_c(CFCMethod_get_return_type(method));
    char *full_typedef = CFCMethod_full_typedef(method, klass);
    char *buf = CFCUtil_sprintf("typedef %s\n(*%s)(%s);\n", ret_type,
                                full_typedef, params);
    FREEMEM(full_typedef);
    return buf;
}

char*
CFCBindMeth_spec_def(CFCMethod *method) {
    const char *macro_sym   = CFCMethod_get_macro_sym(method);
    const char *impl_sym    = CFCMethod_implementing_func_sym(method);
    int         is_novel    = CFCMethod_novel(method);

    const char *full_override_sym = "NULL";
    if (is_novel && !CFCMethod_final(method)) {
        full_override_sym = CFCMethod_full_override_sym(method);
    }

    char *full_offset_sym = CFCMethod_full_offset_sym(method, NULL);

    char pattern[] =
        "    {\n"
        "        %d, /* is_novel */\n"
        "        \"%s\", /* name */\n"
        "        (cfish_method_t)%s, /* func */\n"
        "        (cfish_method_t)%s, /* callback_func */\n"
        "        &%s /* offset */\n"
        "    }";
    char *def
        = CFCUtil_sprintf(pattern, is_novel, macro_sym, impl_sym,
                          full_override_sym, full_offset_sym);

    FREEMEM(full_offset_sym);
    return def;
}

static char*
S_build_unused_vars(CFCVariable **vars) {
    char *unused = CFCUtil_strdup("");

    for (int i = 0; vars[i] != NULL; i++) {
        const char *var_name = CFCVariable_micro_sym(vars[i]);
        size_t size = strlen(unused) + strlen(var_name) + 80;
        unused = (char*)REALLOCATE(unused, size);
        strcat(unused, "\n    CHY_UNUSED_VAR(");
        strcat(unused, var_name);
        strcat(unused, ");");
    }

    return unused;
}

static char*
S_maybe_unreachable(CFCType *return_type) {
    char *return_statement;
    if (CFCType_is_void(return_type)) {
        return_statement = CFCUtil_strdup("");
    }
    else {
        const char *ret_type_str = CFCType_to_c(return_type);
        char pattern[] = "\n    CHY_UNREACHABLE_RETURN(%s);";
        return_statement = CFCUtil_sprintf(pattern, ret_type_str);
    }
    return return_statement;
}

char*
CFCBindMeth_abstract_method_def(CFCMethod *method) {
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    const char *params = CFCParamList_to_c(param_list);
    const char *full_func_sym = CFCMethod_implementing_func_sym(method);
    const char *vtable_var
        = CFCType_get_vtable_var(CFCMethod_self_type(method));
    CFCType    *return_type  = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    const char *macro_sym    = CFCMethod_get_macro_sym(method);

    // Thwart compiler warnings.
    CFCVariable **param_vars = CFCParamList_get_variables(param_list);
    char *unused = S_build_unused_vars(param_vars + 1);
    char *return_statement = S_maybe_unreachable(return_type);

    char pattern[] =
        "%s\n"
        "%s(%s) {\n"
        "    cfish_CharBuf *klass = self ? Cfish_Obj_Get_Class_Name((cfish_Obj*)self) : %s->name;%s\n"
        "    CFISH_THROW(CFISH_ERR, \"Abstract method '%s' not defined by %%o\", klass);%s\n"
        "}\n";
    char *abstract_def
        = CFCUtil_sprintf(pattern, ret_type_str, full_func_sym, params,
                          vtable_var, unused, macro_sym, return_statement);

    FREEMEM(unused);
    FREEMEM(return_statement);
    return abstract_def;
}

char*
CFCBindMeth_callback_dec(CFCMethod *method) {
    CFCType *return_type = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    const char *override_sym = CFCMethod_full_override_sym(method);
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));

    char pattern[] =
        "%s\n"
        "%s(%s);\n";
    char *callback_dec
        = CFCUtil_sprintf(pattern, ret_type_str, override_sym, params);

    return callback_dec;
}

