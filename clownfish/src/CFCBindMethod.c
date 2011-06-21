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

char*
CFCBindMeth_typdef_dec(struct CFCMethod *method) {
    const char *params 
        = CFCParamList_to_c(CFCFunction_get_param_list((CFCFunction*)method));
    const char *ret_type
        = CFCType_to_c(CFCFunction_get_return_type((CFCFunction*)method));
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
CFCBindMeth_abstract_method_def(CFCMethod *method) {
    CFCParamList *param_list = CFCFunction_get_param_list((CFCFunction*)method);
    const char *params = CFCParamList_to_c(param_list);
    const char *full_func_sym 
        = CFCFunction_full_func_sym((CFCFunction*)method);
    const char *vtable_var
        = CFCType_get_vtable_var(CFCMethod_self_type(method));
    CFCType    *return_type  = CFCFunction_get_return_type((CFCFunction*)method);
    const char *ret_type_str = CFCType_to_c(return_type);
    const char *macro_sym    = CFCMethod_get_macro_sym(method);
    
    /* Build list of unused params. */
    char *unused = CFCUtil_strdup("");
    CFCVariable **param_vars = CFCParamList_get_variables(param_list);
    for (int i = 1; param_vars[i] != NULL; i++) {
        const char *var_name = CFCVariable_micro_sym(param_vars[i]);
        size_t size = strlen(unused) + strlen(var_name) + 80;
        unused = (char*)REALLOCATE(unused, size);
        strcat(unused, "\n    CHY_UNUSED_VAR(");
        strcat(unused, var_name);
        strcat(unused, ");");
    }

    /* Create an unreachable return statement if necessary, in order to thwart
     * compiler warnings. */
    char *return_statement;
    if (CFCType_is_void(return_type)) {
        return_statement = CFCUtil_strdup("");
    }
    else {
        return_statement = MALLOCATE(strlen(ret_type_str) + 80);
        sprintf(return_statement, "\n    CHY_UNREACHABLE_RETURN(%s);",
                ret_type_str);
    }

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

