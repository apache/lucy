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
CFCBindMeth_typdef_dec(struct CFCMethod *method) {
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    const char *ret_type = CFCType_to_c(CFCMethod_get_return_type(method));
    const char *full_typedef = CFCMethod_full_typedef(method);
    size_t size = strlen(params)
                  + strlen(ret_type)
                  + strlen(full_typedef)
                  + 20
                  + sizeof("\0");
    char *buf = (char*)MALLOCATE(size);
    sprintf(buf, "typedef %s\n(*%s)(%s);\n", ret_type, full_typedef, params);
    return buf;
}

char*
CFCBindMeth_callback_dec(CFCMethod *method)
{
    const char *full_callback_sym = CFCMethod_full_callback_sym(method);
    size_t size = strlen(full_callback_sym) + 50;
    char *callback_dec = (char*)MALLOCATE(size);
    sprintf(callback_dec, "extern cfish_Callback %s;\n", full_callback_sym);
    return callback_dec;
}

char*
CFCBindMeth_callback_obj_def(CFCMethod *method, const char *offset) {
    const char *macro_sym         = CFCMethod_get_macro_sym(method);
    unsigned    macro_sym_len     = strlen(macro_sym);
    const char *full_override_sym = CFCMethod_full_override_sym(method);
    const char *full_callback_sym = CFCMethod_full_callback_sym(method);
    char template[] = 
         "cfish_Callback %s = {\"%s\", %u, (cfish_method_t)%s, %s};\n";
    size_t size = sizeof(template)
                  + macro_sym_len
                  + strlen(full_override_sym)
                  + strlen(full_callback_sym)
                  + strlen(offset)
                  + 30;
    char *def = (char*)MALLOCATE(size);
    sprintf(def, template, full_callback_sym, macro_sym, macro_sym_len,
        full_override_sym, offset);
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

/* Create an unreachable return statement if necessary, in order to thwart
 * compiler warnings. */
static char*
S_maybe_unreachable(CFCType *return_type) {
    char *return_statement;
    if (CFCType_is_void(return_type)) {
        return_statement = CFCUtil_strdup("");
    }
    else {
        const char *ret_type_str = CFCType_to_c(return_type);
        return_statement = MALLOCATE(strlen(ret_type_str) + 60);
        sprintf(return_statement, "\n    CHY_UNREACHABLE_RETURN(%s);",
                ret_type_str);
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

    char template[] = 
        "%s\n"
        "%s(%s) {\n"
        "    cfish_CharBuf *klass = self ? Cfish_Obj_Get_Class_Name((cfish_Obj*)self) : %s->name;%s\n"
        "    CFISH_THROW(CFISH_ERR, \"Abstract method '%s' not defined by %%o\", klass);%s\n"
        "}\n";
    size_t needed = sizeof(template)
                    + strlen(ret_type_str)
                    + strlen(full_func_sym)
                    + strlen(params)
                    + strlen(vtable_var)
                    + strlen(unused)
                    + strlen(macro_sym)
                    + strlen(return_statement)
                    + 50;
    char *abstract_def = (char*)MALLOCATE(needed);
    sprintf(abstract_def, template, ret_type_str, full_func_sym, params, 
            vtable_var, unused, macro_sym, return_statement);

    FREEMEM(unused);
    FREEMEM(return_statement);
    return abstract_def;
}

char*
CFCBindMeth_callback_def(CFCMethod *method) {
    CFCType *return_type = CFCMethod_get_return_type(method);
    char *params = CFCBindMeth_callback_params(method);
    char *callback_def = NULL;

    if (!params) {
        // Can't map vars, because there's at least one type in the argument
        // list we don't yet support.  Return a callback wrapper that throws
        // an error error.
        callback_def = CFCBindMeth_invalid_callback_def(method);
    }
    else if (CFCType_is_void(return_type)) {
        callback_def = CFCBindMeth_void_callback_def(method, params);
    }
    else if (CFCType_is_object(return_type)) {
        callback_def = CFCBindMeth_obj_callback_def(method, params);
    }
    else {
        callback_def = CFCBindMeth_primitive_callback_def(method, params);
    }

    FREEMEM(params);
    return callback_def;
}

/* Return a string which maps arguments to various arg wrappers conforming
 * to Host's callback interface.  For instance, (int32_t foo, Obj *bar)
 * produces the following:
 *
 *   CFISH_ARG_I32("foo", foo),
 *   CFISH_ARG_OBJ("bar", bar)
 */
char*
CFCBindMeth_callback_params(CFCMethod *method) {
    const char *micro_sym = CFCSymbol_micro_sym((CFCSymbol*)method);
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    unsigned num_params = CFCParamList_num_vars(param_list) - 1;
    size_t needed = strlen(micro_sym) + 30;
    char *params = (char*)MALLOCATE(needed);

    // TODO: use something other than micro_sym here.
    sprintf(params, "self, \"%s\", %u", micro_sym, num_params);

    // Iterate over arguments, mapping them to various arg wrappers which
    // conform to Host's callback interface.
    CFCVariable **arg_vars = CFCParamList_get_variables(param_list);
    for (int i = 1; arg_vars[i] != NULL; i++) {
        CFCVariable *var      = arg_vars[i];
        const char  *name     = CFCVariable_micro_sym(var);
        size_t       name_len = strlen(name);
        CFCType     *type     = CFCVariable_get_type(var);
        const char  *c_type   = CFCType_to_c(type);
        size_t       size     = strlen(params) 
                                + strlen(c_type) 
                                + name_len * 2
                                + 30;
        char        *new_buf  = (char*)MALLOCATE(size); 

        if (CFCType_is_string_type(type)) {
            sprintf(new_buf, "%s, CFISH_ARG_STR(\"%s\", %s)", params, name, name);
        }
        else if (CFCType_is_object(type)) {
            sprintf(new_buf, "%s, CFISH_ARG_OBJ(\"%s\", %s)", params, name, name);
        }
        else if (CFCType_is_integer(type)) {
            int width = CFCType_get_width(type);
            if (width) {
                if (width <= 4) {
                    sprintf(new_buf, "%s, CFISH_ARG_I32(\"%s\", %s)", params,
                            name, name);
                }
                else {
                    sprintf(new_buf, "%s, CFISH_ARG_I64(\"%s\", %s)", params,
                            name, name);
                }
            }
            else {
                sprintf(new_buf, "%s, CFISH_ARG_I(%s, \"%s\", %s)", params,
                        c_type, name, name);
            }
        }
        else if (CFCType_is_floating(type)) {
            sprintf(new_buf, "%s, CFISH_ARG_F64(\"%s\", %s)", params, name, name);
        }
        else {
            // Can't map variable type.  Signal to caller.
            FREEMEM(params);
            FREEMEM(new_buf);
            return NULL;
        }

        FREEMEM(params);
        params = new_buf;
    }

    return params;
}

/* Return a function which throws a runtime error indicating which variable
 * couldn't be mapped.  TODO: it would be better to resolve all these cases at
 * compile-time.
 */
char*
CFCBindMeth_invalid_callback_def(CFCMethod *method) {
    const char *class_cnick = CFCMethod_get_class_cnick(method);
    size_t meth_sym_size = CFCMethod_full_method_sym(method, class_cnick, NULL, 0);
    char *full_method_sym = (char*)MALLOCATE(meth_sym_size);
    CFCMethod_full_method_sym(method, class_cnick, full_method_sym, 
                              meth_sym_size);

    const char *override_sym = CFCMethod_full_override_sym(method);
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    const char *params = CFCParamList_to_c(param_list);
    CFCVariable **param_vars = CFCParamList_get_variables(param_list);

    // Thwart compiler warnings.
    CFCType *return_type = CFCMethod_get_return_type(method);
    char *unused = S_build_unused_vars(param_vars);
    char *unreachable = S_maybe_unreachable(return_type);

    char pattern[] = 
        "void\n"
        "%s(%s) {%s\n"
        "    CFISH_THROW(CFISH_ERR, \"Can't override %s via binding\");%s\n"
        "}\n";
    size_t size = sizeof(pattern) 
                  + strlen(override_sym)
                  + strlen(params)
                  + strlen(unused)
                  + strlen(full_method_sym)
                  + strlen(unreachable)
                  + 20;
    char *callback_def = (char*)MALLOCATE(size);
    sprintf(callback_def, pattern, override_sym, params, unused,
            full_method_sym, unreachable);

    FREEMEM(unused);
    return callback_def;
}

// Create a callback for a method which operates in a void context.
char*
CFCBindMeth_void_callback_def(CFCMethod *method, 
                              const char *callback_params) {
    const char *override_sym = CFCMethod_full_override_sym(method);
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    const char pattern[] = 
        "void\n"
        "%s(%s) {\n"
        "    cfish_Host_callback(%s);\n"
        "}\n";
    size_t size = sizeof(pattern)
                  + strlen(override_sym)
                  + strlen(params)
                  + strlen(callback_params)
                  + 200;
    char *callback_def = (char*)MALLOCATE(size);
    sprintf(callback_def, pattern, override_sym, params, callback_params);
    return callback_def;
}

// Create a callback which returns a primitive type.
char*
CFCBindMeth_primitive_callback_def(CFCMethod *method,
                                   const char *callback_params) {
    const char *override_sym = CFCMethod_full_override_sym(method);
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    CFCType *return_type = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    char cb_func_name[40];
    if (CFCType_is_floating(return_type)) {
        strcpy(cb_func_name, "cfish_Host_callback_f64");
    }
    else if (CFCType_is_integer(return_type)) {
        strcpy(cb_func_name, "cfish_Host_callback_i64");
    }
    else if (strcmp(ret_type_str, "void*") == 0) {
        strcpy(cb_func_name, "cfish_Host_callback_host");
    }
    else {
        CFCUtil_die("unrecognized type: %s", ret_type_str);
    }

    char pattern[] = 
        "%s\n"
        "%s(%s) {\n"
        "    return (%s)%s(%s);\n"
        "}\n";
    size_t size = sizeof(pattern)
                  + strlen(ret_type_str)
                  + strlen(override_sym)
                  + strlen(params)
                  + strlen(ret_type_str)
                  + strlen(cb_func_name)
                  + strlen(callback_params)
                  + 20;
    char *callback_def = (char*)MALLOCATE(size);
    sprintf(callback_def, pattern, ret_type_str, override_sym, params,
            ret_type_str, cb_func_name, callback_params);

    return callback_def;
}

// Create a callback which returns an object type -- either a generic object or
// a string.
char*
CFCBindMeth_obj_callback_def(CFCMethod *method, const char *callback_params) {
    const char *override_sym = CFCMethod_full_override_sym(method);
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    CFCType *return_type = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    const char *cb_func_name = CFCType_is_string_type(return_type)
                               ? "cfish_Host_callback_str"
                               : "cfish_Host_callback_obj";

    char *nullable_check = CFCUtil_strdup("");
    if (!CFCType_nullable(return_type)) {
        const char *macro_sym = CFCMethod_get_macro_sym(method);
        char pattern[] = 
            "\n    if (!retval) { CFISH_THROW(CFISH_ERR, "
            "\"%s() for class '%%o' cannot return NULL\", "
            "Cfish_Obj_Get_Class_Name((cfish_Obj*)self)); }";
        size_t size = sizeof(pattern) + strlen(macro_sym) + 30;
        nullable_check = (char*)REALLOCATE(nullable_check, size);
        sprintf(nullable_check, pattern, macro_sym);
    }

    const char *decrement = CFCType_incremented(return_type)
                            ? "" 
                            : "\n    LUCY_DECREF(retval);";

    char pattern[] = 
        "%s\n"
        "%s(%s) {\n"
        "    %s retval = (%s)%s(%s);%s%s\n"
        "    return retval;\n"
        "}\n";
    size_t size = sizeof(pattern)
                  + strlen(ret_type_str)
                  + strlen(override_sym)
                  + strlen(params)
                  + strlen(ret_type_str)
                  + strlen(ret_type_str)
                  + strlen(cb_func_name)
                  + strlen(callback_params)
                  + strlen(nullable_check)
                  + strlen(decrement)
                  + 30;
    char *callback_def = (char*)MALLOCATE(size);
    sprintf(callback_def, pattern, ret_type_str, override_sym, params,
            ret_type_str, ret_type_str, cb_func_name, callback_params,
            nullable_check, decrement);

    FREEMEM(nullable_check);
    return callback_def;
}

