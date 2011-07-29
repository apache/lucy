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

#define CFC_NEED_PERLSUB_STRUCT_DEF 1
#include "CFCPerlSub.h"
#include "CFCPerlMethod.h"
#include "CFCUtil.h"
#include "CFCMethod.h"
#include "CFCType.h"
#include "CFCParamList.h"
#include "CFCPerlTypeMap.h"
#include "CFCVariable.h"

struct CFCPerlMethod {
    CFCPerlSub  sub;
    CFCMethod  *method;
};

// Return the main chunk of the code for the xsub.
static char*
S_xsub_body(CFCPerlMethod *self);

// Create an assignment statement for extracting $self from the Perl stack.
static char*
S_self_assign_statement(CFCPerlMethod *self, CFCType *type,
                        const char *method_name);

// Return code for an xsub which uses labeled params.
static char*
S_xsub_def_labeled_params(CFCPerlMethod *self);

// Return code for an xsub which uses positional args.
static char*
S_xsub_def_positional_args(CFCPerlMethod *self);

CFCPerlMethod*
CFCPerlMethod_new(CFCMethod *method, const char *alias) {
    CFCPerlMethod *self
        = (CFCPerlMethod*)CFCBase_allocate(sizeof(CFCPerlMethod),
                                          "Clownfish::Binding::Perl::Method");
    return CFCPerlMethod_init(self, method, alias);
}

CFCPerlMethod*
CFCPerlMethod_init(CFCPerlMethod *self, CFCMethod *method,
                   const char *alias) {
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    const char *class_name = CFCMethod_get_class_name(method);
    int use_labeled_params = CFCParamList_num_vars(param_list) > 2
                             ? 1 : 0;
    if (!alias) {
        alias = CFCMethod_micro_sym(method);
    }
    CFCPerlSub_init((CFCPerlSub*)self, param_list, class_name, alias,
                    use_labeled_params);
    self->method = (CFCMethod*)CFCBase_incref((CFCBase*)method);
    return self;
}

void
CFCPerlMethod_destroy(CFCPerlMethod *self) {
    CFCBase_decref((CFCBase*)self->method);
    CFCPerlSub_destroy((CFCPerlSub*)self);
}

char*
CFCPerlMethod_xsub_def(CFCPerlMethod *self) {
    if (self->sub.use_labeled_params) {
        return S_xsub_def_labeled_params(self);
    }
    else {
        return S_xsub_def_positional_args(self);
    }
}

static char*
S_xsub_body(CFCPerlMethod *self) {
    CFCMethod    *method        = self->method;
    const char   *full_func_sym = CFCMethod_implementing_func_sym(method);
    CFCParamList *param_list    = CFCMethod_get_param_list(method);
    CFCVariable **arg_vars      = CFCParamList_get_variables(param_list);
    const char   *name_list     = CFCParamList_name_list(param_list);
    char *body = CFCUtil_strdup("");

    // Compensate for functions which eat refcounts.
    for (int i = 0; arg_vars[i] != NULL; i++) {
        CFCVariable *var = arg_vars[i];
        CFCType     *type = CFCVariable_get_type(var);
        if (CFCType_is_object(type) && CFCType_decremented(type)) {
            body = CFCUtil_cat(body, "LUCY_INCREF(",
                               CFCVariable_micro_sym(var), ");\n    ", NULL);
        }
    }

    if (CFCType_is_void(CFCMethod_get_return_type(method))) {
        // Invoke method in void context.
        body = CFCUtil_cat(body, full_func_sym, "(", name_list, 
                           ");\n    XSRETURN(0);", NULL);
    }
    else {
        // Return a value for method invoked in a scalar context.
        CFCType *return_type = CFCMethod_get_return_type(method);
        const char *type_str = CFCType_to_c(return_type);
        char *assignment = CFCPerlTypeMap_to_perl(return_type, "retval");
        body = CFCUtil_cat(body, type_str, " retval = ", full_func_sym, "(",
                           name_list, ");\n    ST(0) = ", assignment, ";",
                           NULL);
        if (CFCType_is_object(return_type) 
            && CFCType_incremented(return_type)
           ) {
            body = CFCUtil_cat(body, "\n    LUCY_DECREF(retval);", NULL);
        }
        body = CFCUtil_cat(body, "\n    sv_2mortal( ST(0) );\n    XSRETURN(1);",
                           NULL);
        FREEMEM(assignment);
    }

    return body;
}

// Create an assignment statement for extracting $self from the Perl stack.
static char*
S_self_assign_statement(CFCPerlMethod *self, CFCType *type,
                        const char *method_name) {
    const char *type_c = CFCType_to_c(type);
    if (!CFCType_is_object(type)) {
        CFCUtil_die("Not an object type: %s", type_c);
    }
    const char *vtable_var = CFCType_get_vtable_var(type);
    
    // Make an exception for deserialize -- allow self to be NULL if called as
    // a class method.
    char *binding_func = strcmp(method_name, "deserialize") == 0
                         ? "XSBind_maybe_sv_to_cfish_obj"
                         : "XSBind_sv_to_cfish_obj";
    char pattern[] = "%s self = (%s)%s(ST(0), %s, NULL);";
    size_t size = sizeof(pattern)
                  + strlen(type_c) * 2
                  + strlen(binding_func)
                  + strlen(vtable_var) 
                  + 10;
    char *statement = (char*)MALLOCATE(size);
    sprintf(statement, pattern, type_c, type_c, binding_func, vtable_var);

    return statement;
}

static char*
S_xsub_def_labeled_params(CFCPerlMethod *self) {
    const char *c_name = self->sub.c_name;
    CFCParamList *param_list = self->sub.param_list;
    const char **arg_inits   = CFCParamList_get_initial_values(param_list);
    CFCVariable **arg_vars   = CFCParamList_get_variables(param_list);
    CFCVariable *self_var    = arg_vars[0];
    CFCType     *self_type   = CFCVariable_get_type(self_var);
    const char  *self_micro_sym = CFCVariable_micro_sym(self_var);
    const char  *micro_sym   = CFCMethod_micro_sym(self->method);
    char *self_assign = S_self_assign_statement(self, self_type, micro_sym);
    char *allot_params = CFCPerlSub_build_allot_params((CFCPerlSub*)self);
    char *body = S_xsub_body(self);

    char pattern[] =
        "XS(%s);\n"
        "XS(%s) {\n"
        "    dXSARGS;\n"
        "    CHY_UNUSED_VAR(cv);\n"
        "    if (items < 1) { CFISH_THROW(CFISH_ERR, \"Usage: %%s(%s, ...)\",  GvNAME(CvGV(cv))); };\n"
        "    SP -= items;\n"
        "\n"
        "    /* Extract vars from Perl stack. */\n"
        "    %s\n"
        "    %s\n"
        "\n"
        "    /* Execute */\n"
        "    %s\n"
        "}\n";
    size_t size = sizeof(pattern)
                  + strlen(c_name) * 2
                  + strlen(self_micro_sym)
                  + strlen(allot_params)
                  + strlen(self_assign)
                  + strlen(body)
                  + 40;
    char *xsub_def = (char*)MALLOCATE(size);
    sprintf(xsub_def, pattern, c_name, c_name, self_micro_sym, allot_params,
            self_assign, body);

    FREEMEM(self_assign);
    FREEMEM(allot_params);
    FREEMEM(body);
    return xsub_def;
}

static char*
S_xsub_def_positional_args(CFCPerlMethod *self) {
    CFCMethod *method = self->method;
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    CFCVariable **arg_vars = CFCParamList_get_variables(param_list);
    const char **arg_inits = CFCParamList_get_initial_values(param_list);
    unsigned num_vars = CFCParamList_num_vars(param_list);
    char *body = S_xsub_body(self);

    // Determine how many args are truly required and build an error check.
    unsigned min_required = 0;
    for (unsigned i = 0; i < num_vars; i++) {
        if (arg_inits[i] == NULL) {
            min_required = i + 1;
        }
    }
    char *xs_name_list = num_vars > 0 
                         ? CFCUtil_strdup(CFCVariable_micro_sym(arg_vars[0]))
                         : CFCUtil_strdup("");
    for (unsigned i = 1; i < num_vars; i++) {
        const char *var_name = CFCVariable_micro_sym(arg_vars[i]);
        if (i < min_required) {
            xs_name_list = CFCUtil_cat(xs_name_list, ", ", var_name, NULL);
        }
        else {
            xs_name_list = CFCUtil_cat(xs_name_list, ", [", var_name, "]",
                                       NULL);
        }
    }
    const char num_args_pattern[] = 
        "if (items %s %u) { CFISH_THROW(CFISH_ERR, \"Usage: %%s(%s)\", GvNAME(CvGV(cv))); }";
    size_t num_args_check_size = sizeof(num_args_pattern)
                                 + strlen(xs_name_list)
                                 + 30;
    char *num_args_check = (char*)MALLOCATE(num_args_check_size);
    if (min_required < num_vars) {
        sprintf(num_args_check, num_args_pattern, "<", min_required,
                xs_name_list);
    }
    else {
        sprintf(num_args_check, num_args_pattern, "!=", num_vars,
                xs_name_list);
    }

    // Var assignments.
    char *var_assignments = CFCUtil_strdup("");
    for (unsigned i = 0; i < num_vars; i++) {
        CFCVariable *var = arg_vars[i];
        const char  *val = arg_inits[i];
        const char  *var_name = CFCVariable_micro_sym(var);
        CFCType     *var_type = CFCVariable_get_type(var);
        const char  *type_c   = CFCType_to_c(var_type);

        if (i == 0) {    // self
            const char *meth_micro_sym = CFCMethod_micro_sym(self->method);
            char *statement
                = S_self_assign_statement(self, var_type, meth_micro_sym);
            var_assignments = CFCUtil_cat(var_assignments, statement, NULL);
            FREEMEM(statement);
        }
        else {
            char perl_stack_var[30];
            sprintf(perl_stack_var, "ST(%u)", i);
            char *conversion
                = CFCPerlTypeMap_from_perl(var_type, perl_stack_var);
            if (val) {
                char pattern[] = 
                    "\n    %s %s = ( items >= %u && XSBind_sv_defined(ST(%u)) )"
                    " ? %s : %s;";
                size_t size = sizeof(pattern)
                              + strlen(type_c)
                              + strlen(var_name)
                              + strlen(conversion)
                              + strlen(val)
                              + 100;
                char *statement = (char*)MALLOCATE(size);
                sprintf(statement, pattern, type_c, var_name, i, i,
                        conversion, val);
                var_assignments
                    = CFCUtil_cat(var_assignments, statement, NULL);
                FREEMEM(statement);
            }
            else {
                var_assignments
                    = CFCUtil_cat(var_assignments, "\n    ", type_c, " ",
                                  var_name, " = ", conversion, ";", NULL);
            }
            FREEMEM(conversion);
        }
    }

    char pattern[] = 
        "XS(%s);\n"
        "XS(%s) {\n"
        "    dXSARGS;\n"
        "    CHY_UNUSED_VAR(cv);\n"
        "    SP -= items;\n"
        "    %s;\n"
        "\n"
        "    /* Extract vars from Perl stack. */\n"
        "    %s\n"
        "\n"
        "    /* Execute */\n"
        "    %s\n"
        "}\n";
    size_t size = sizeof(pattern)
                  + strlen(self->sub.c_name) * 2
                  + strlen(num_args_check)
                  + strlen(var_assignments)
                  + strlen(body)
                  + 20;
    char *xsub = (char*)MALLOCATE(size);
    sprintf(xsub, pattern, self->sub.c_name, self->sub.c_name, num_args_check,
            var_assignments, body);

    FREEMEM(num_args_check);
    FREEMEM(var_assignments);
    FREEMEM(body);
    return xsub;
}

