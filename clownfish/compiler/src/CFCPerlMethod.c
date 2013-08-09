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
#include "CFCClass.h"
#include "CFCMethod.h"
#include "CFCSymbol.h"
#include "CFCType.h"
#include "CFCParcel.h"
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
S_self_assign_statement(CFCPerlMethod *self, CFCType *type);

// Return code for an xsub which uses labeled params.
static char*
S_xsub_def_labeled_params(CFCPerlMethod *self);

// Return code for an xsub which uses positional args.
static char*
S_xsub_def_positional_args(CFCPerlMethod *self);

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

/* Generate code which converts C types to Perl types and pushes arguments
 * onto the Perl stack.
 */
static char*
S_callback_start(CFCMethod *method);

/* Adapt the refcounts of parameters and return types.
 */
static char*
S_callback_refcount_mods(CFCMethod *method);

/* Return a function which throws a runtime error indicating which variable
 * couldn't be mapped.  TODO: it would be better to resolve all these cases at
 * compile-time.
 */
static char*
S_invalid_callback_def(CFCMethod *method);

// Create a callback for a method which operates in a void context.
static char*
S_void_callback_def(CFCMethod *method, const char *callback_start,
                    const char *refcount_mods);

// Create a callback which returns a primitive type.
static char*
S_primitive_callback_def(CFCMethod *method, const char *callback_start,
                         const char *refcount_mods);

/* Create a callback which returns an object type -- either a generic object or
 * a string. */
static char*
S_obj_callback_def(CFCMethod *method, const char *callback_start,
                   const char *refcount_mods);

static const CFCMeta CFCPERLMETHOD_META = {
    "Clownfish::CFC::Binding::Perl::Method",
    sizeof(CFCPerlMethod),
    (CFCBase_destroy_t)CFCPerlMethod_destroy
};

CFCPerlMethod*
CFCPerlMethod_new(CFCMethod *method, const char *alias) {
    CFCPerlMethod *self
        = (CFCPerlMethod*)CFCBase_allocate(&CFCPERLMETHOD_META);
    return CFCPerlMethod_init(self, method, alias);
}

CFCPerlMethod*
CFCPerlMethod_init(CFCPerlMethod *self, CFCMethod *method,
                   const char *alias) {
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    const char *class_name = CFCMethod_get_class_name(method);
    int use_labeled_params = CFCParamList_num_vars(param_list) > 2
                             ? 1 : 0;

    // The Clownfish destructor needs to be spelled DESTROY for Perl.
    if (!alias) {
        alias = CFCMethod_micro_sym(method);
    }
    static const char destroy_uppercase[] = "DESTROY";
    if (strcmp(alias, "destroy") == 0) {
        alias = destroy_uppercase;
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
    CFCParamList *param_list    = CFCMethod_get_param_list(method);
    CFCVariable **arg_vars      = CFCParamList_get_variables(param_list);
    const char   *name_list     = CFCParamList_name_list(param_list);
    char *body = CFCUtil_strdup("");

    CFCParcel *parcel = CFCMethod_get_parcel(method);
    const char *class_name = CFCMethod_get_class_name(method);
    CFCClass *klass = CFCClass_fetch_singleton(parcel, class_name);
    if (!klass) {
        CFCUtil_die("Can't find a CFCClass for '%s'", class_name);
    }

    // Extract the method function pointer.
    char *full_typedef = CFCMethod_full_typedef(method, klass);
    char *full_meth    = CFCMethod_full_method_sym(method, klass);
    char *method_ptr
        = CFCUtil_sprintf("%s method = CFISH_METHOD_PTR(%s, %s);\n    ",
                          full_typedef, CFCClass_full_vtable_var(klass),
                          full_meth);
    body = CFCUtil_cat(body, method_ptr, NULL);
    FREEMEM(full_typedef);
    FREEMEM(full_meth);
    FREEMEM(method_ptr);

    // Compensate for functions which eat refcounts.
    for (int i = 0; arg_vars[i] != NULL; i++) {
        CFCVariable *var = arg_vars[i];
        CFCType     *type = CFCVariable_get_type(var);
        if (CFCType_is_object(type) && CFCType_decremented(type)) {
            body = CFCUtil_cat(body, "CFISH_INCREF(",
                               CFCVariable_micro_sym(var), ");\n    ", NULL);
        }
    }

    if (CFCType_is_void(CFCMethod_get_return_type(method))) {
        // Invoke method in void context.
        body = CFCUtil_cat(body, "method(", name_list,
                           ");\n    XSRETURN(0);", NULL);
    }
    else {
        // Return a value for method invoked in a scalar context.
        CFCType *return_type = CFCMethod_get_return_type(method);
        const char *type_str = CFCType_to_c(return_type);
        char *assignment = CFCPerlTypeMap_to_perl(return_type, "retval");
        if (!assignment) {
            CFCUtil_die("Can't find typemap for '%s'", type_str);
        }
        body = CFCUtil_cat(body, type_str, " retval = method(",
                           name_list, ");\n    ST(0) = ", assignment, ";",
                           NULL);
        if (CFCType_is_object(return_type)
            && CFCType_incremented(return_type)
           ) {
            body = CFCUtil_cat(body, "\n    CFISH_DECREF(retval);", NULL);
        }
        body = CFCUtil_cat(body, "\n    sv_2mortal( ST(0) );\n    XSRETURN(1);",
                           NULL);
        FREEMEM(assignment);
    }

    return body;
}

// Create an assignment statement for extracting $self from the Perl stack.
static char*
S_self_assign_statement(CFCPerlMethod *self, CFCType *type) {
    (void)self; // unused
    const char *type_c = CFCType_to_c(type);
    if (!CFCType_is_object(type)) {
        CFCUtil_die("Not an object type: %s", type_c);
    }
    const char *vtable_var = CFCType_get_vtable_var(type);
    char pattern[] = "%s self = (%s)XSBind_sv_to_cfish_obj(ST(0), %s, NULL);";
    char *statement = CFCUtil_sprintf(pattern, type_c, type_c, vtable_var);

    return statement;
}

static char*
S_xsub_def_labeled_params(CFCPerlMethod *self) {
    const char *c_name = self->sub.c_name;
    CFCParamList *param_list = self->sub.param_list;
    CFCVariable **arg_vars   = CFCParamList_get_variables(param_list);
    CFCVariable *self_var    = arg_vars[0];
    CFCType     *self_type   = CFCVariable_get_type(self_var);
    const char  *self_micro_sym = CFCVariable_micro_sym(self_var);
    char *self_assign = S_self_assign_statement(self, self_type);
    char *allot_params = CFCPerlSub_build_allot_params((CFCPerlSub*)self);
    char *body = S_xsub_body(self);

    char pattern[] =
        "XS(%s);\n"
        "XS(%s) {\n"
        "    dXSARGS;\n"
        "    CFISH_UNUSED_VAR(cv);\n"
        "    if (items < 1) { CFISH_THROW(CFISH_ERR, \"Usage: %%s(%s, ...)\",  GvNAME(CvGV(cv))); }\n"
        "    SP -= items;\n"
        "\n"
        "    /* Extract vars from Perl stack. */\n"
        "    %s\n"
        "    %s\n"
        "\n"
        "    /* Execute */\n"
        "    %s\n"
        "}\n";
    char *xsub_def
        = CFCUtil_sprintf(pattern, c_name, c_name, self_micro_sym,
                          allot_params, self_assign, body);

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
    unsigned num_vars = (unsigned)CFCParamList_num_vars(param_list);
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
    char *num_args_check;
    if (min_required < num_vars) {
        num_args_check = CFCUtil_sprintf(num_args_pattern, "<", min_required,
                                         xs_name_list);
    }
    else {
        num_args_check = CFCUtil_sprintf(num_args_pattern, "!=", num_vars,
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
            char *statement = S_self_assign_statement(self, var_type);
            var_assignments = CFCUtil_cat(var_assignments, statement, NULL);
            FREEMEM(statement);
        }
        else {
            char perl_stack_var[30];
            sprintf(perl_stack_var, "ST(%u)", i);
            char *conversion
                = CFCPerlTypeMap_from_perl(var_type, perl_stack_var);
            if (!conversion) {
                CFCUtil_die("Can't map type '%s'", type_c);
            }
            if (val) {
                char pattern[] =
                    "\n    %s %s = ( items >= %u && XSBind_sv_defined(ST(%u)) )"
                    " ? %s : %s;";
                char *statement = CFCUtil_sprintf(pattern, type_c, var_name, i,
                                                  i, conversion, val);
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
        "    CFISH_UNUSED_VAR(cv);\n"
        "    SP -= items;\n"
        "    %s;\n"
        "\n"
        "    /* Extract vars from Perl stack. */\n"
        "    %s\n"
        "\n"
        "    /* Execute */\n"
        "    %s\n"
        "}\n";
    char *xsub
        = CFCUtil_sprintf(pattern, self->sub.c_name, self->sub.c_name,
                          num_args_check, var_assignments, body);

    FREEMEM(num_args_check);
    FREEMEM(var_assignments);
    FREEMEM(body);
    return xsub;
}

char*
CFCPerlMethod_callback_def(CFCMethod *method) {
    CFCType *return_type = CFCMethod_get_return_type(method);
    char *start = S_callback_start(method);
    char *callback_def = NULL;
    char *refcount_mods = S_callback_refcount_mods(method);

    if (!start) {
        // Can't map vars, because there's at least one type in the argument
        // list we don't yet support.  Return a callback wrapper that throws
        // an error error.
        callback_def = S_invalid_callback_def(method);
    }
    else if (CFCType_is_void(return_type)) {
        callback_def = S_void_callback_def(method, start, refcount_mods);
    }
    else if (CFCType_is_object(return_type)) {
        callback_def = S_obj_callback_def(method, start, refcount_mods);
    }
    else if (CFCType_is_integer(return_type)
             || CFCType_is_floating(return_type)
        ) {
        callback_def = S_primitive_callback_def(method, start, refcount_mods);
    }
    else {
        // Can't map return type.
        callback_def = S_invalid_callback_def(method);
    }

    FREEMEM(start);
    FREEMEM(refcount_mods);
    return callback_def;
}

static char*
S_build_unused_vars(CFCVariable **vars) {
    char *unused = CFCUtil_strdup("");

    for (int i = 0; vars[i] != NULL; i++) {
        const char *var_name = CFCVariable_micro_sym(vars[i]);
        size_t size = strlen(unused) + strlen(var_name) + 80;
        unused = (char*)REALLOCATE(unused, size);
        strcat(unused, "\n    CFISH_UNUSED_VAR(");
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
        char pattern[] = "\n    CFISH_UNREACHABLE_RETURN(%s);";
        return_statement = CFCUtil_sprintf(pattern, ret_type_str);
    }
    return return_statement;
}

static char*
S_callback_start(CFCMethod *method) {
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    static const char pattern[] =
        "    dSP;\n"
        "    EXTEND(SP, %d);\n"
        "    ENTER;\n"
        "    SAVETMPS;\n"
        "    PUSHMARK(SP);\n"
        "    mPUSHs((SV*)Cfish_Obj_To_Host((cfish_Obj*)self));\n";
    int num_args = (int)CFCParamList_num_vars(param_list) - 1;
    int num_to_extend = num_args == 0 ? 1
                      : num_args == 1 ? 2
                      : 1 + (num_args * 2);
    char *params = CFCUtil_sprintf(pattern, num_to_extend);

    // Iterate over arguments, mapping them to Perl scalars.
    CFCVariable **arg_vars = CFCParamList_get_variables(param_list);
    for (int i = 1; arg_vars[i] != NULL; i++) {
        CFCVariable *var      = arg_vars[i];
        const char  *name     = CFCVariable_micro_sym(var);
        CFCType     *type     = CFCVariable_get_type(var);
        const char  *c_type   = CFCType_to_c(type);

        // Add labels when there are two or more parameters.
        if (num_args > 1) {
            char num_buf[20];
            sprintf(num_buf, "%d", (int)strlen(name));
            params = CFCUtil_cat(params, "   mPUSHp(\"", name, "\", ",
                                 num_buf, ");\n", NULL);
        }

        if (CFCType_is_string_type(type)) {
            // Convert Clownfish string type to UTF-8 Perl string scalars.
            params = CFCUtil_cat(params, "    mPUSHs(XSBind_cb_to_sv(",
                                 "(cfish_CharBuf*)", name, "));\n", NULL);
        }
        else if (CFCType_is_object(type)) {
            // Wrap other Clownfish object types in Perl objects.
            params = CFCUtil_cat(params, "    mPUSHs(XSBind_cfish_to_perl(",
                                 "(cfish_Obj*)", name, "));\n", NULL);
        }
        else if (CFCType_is_integer(type)) {
            // Convert primitive integer types to IV Perl scalars.
            int width = (int)CFCType_get_width(type);
            if (width != 0 && width <= 4) {
                params = CFCUtil_cat(params, "   mPUSHi(",
                                     name, ");\n", NULL);
            }
            else {
                // If the Perl IV integer type is not wide enough, use
                // doubles.  This may be lossy if the value is above 2**52,
                // but practically speaking, it's important to handle numbers
                // between 2**32 and 2**52 cleanly.
                params = CFCUtil_cat(params,
                                     "    if (sizeof(IV) >= sizeof(", c_type,
                                     ")) { mPUSHi(", name, "); }\n",
                                     "    else { mPUSHn((double)", name,
                                     "); } // lossy \n", NULL);
            }
        }
        else if (CFCType_is_floating(type)) {
            // Convert primitive floating point types to NV Perl scalars.
            params = CFCUtil_cat(params, "   mPUSHn(",
                                 name, ");\n", NULL);
        }
        else {
            // Can't map variable type.  Signal to caller.
            FREEMEM(params);
            return NULL;
        }
    }

    // Restore the Perl stack pointer.
    params = CFCUtil_cat(params, "    PUTBACK;\n", NULL);

    return params;
}

static char*
S_callback_refcount_mods(CFCMethod *method) {
    char *refcount_mods = CFCUtil_strdup("");
    CFCType *return_type = CFCMethod_get_return_type(method);
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    CFCVariable **arg_vars = CFCParamList_get_variables(param_list);

    // `XSBind_perl_to_cfish()` returns an incremented object.  If this method
    // does not return an incremented object, we must cancel out that
    // refcount.  (No function can return a decremented object.)
    if (CFCType_is_object(return_type) && !CFCType_incremented(return_type)) {
        refcount_mods = CFCUtil_cat(refcount_mods,
                                    "\n    CFISH_DECREF(retval);", NULL);
    }

    // Adjust refcounts of arguments per method signature, so that Perl code
    // does not have to.
    for (int i = 0; arg_vars[i] != NULL; i++) {
        CFCVariable *var  = arg_vars[i];
        CFCType     *type = CFCVariable_get_type(var);
        const char  *name = CFCVariable_micro_sym(var);
        if (!CFCType_is_object(type)) {
            continue;
        }
        else if (CFCType_incremented(type)) {
            refcount_mods = CFCUtil_cat(refcount_mods, "\n    CFISH_INCREF(",
                                        name, ");", NULL);
        }
        else if (CFCType_decremented(type)) {
            refcount_mods = CFCUtil_cat(refcount_mods, "\n    CFISH_DECREF(",
                                        name, ");", NULL);
        }
    }

    return refcount_mods;
}

static char*
S_invalid_callback_def(CFCMethod *method) {
    char *full_method_sym = CFCMethod_full_method_sym(method, NULL);

    const char *override_sym = CFCMethod_full_override_sym(method);
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    const char *params = CFCParamList_to_c(param_list);
    CFCVariable **param_vars = CFCParamList_get_variables(param_list);

    // Thwart compiler warnings.
    CFCType *return_type = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    char *unused = S_build_unused_vars(param_vars);
    char *unreachable = S_maybe_unreachable(return_type);

    char pattern[] =
        "%s\n"
        "%s(%s) {%s\n"
        "    CFISH_THROW(CFISH_ERR, \"Can't override %s via binding\");%s\n"
        "}\n";
    char *callback_def
        = CFCUtil_sprintf(pattern, ret_type_str, override_sym, params, unused,
                          full_method_sym, unreachable);

    FREEMEM(full_method_sym);
    FREEMEM(unreachable);
    FREEMEM(unused);
    return callback_def;
}

static char*
S_void_callback_def(CFCMethod *method, const char *callback_start,
                    const char *refcount_mods) {
    const char *override_sym = CFCMethod_full_override_sym(method);
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    const char *micro_sym = CFCMethod_micro_sym(method);
    const char pattern[] =
        "void\n"
        "%s(%s) {\n"
        "%s"
        "    S_finish_callback_void(\"%s\");%s\n"
        "}\n";
    char *callback_def
        = CFCUtil_sprintf(pattern, override_sym, params, callback_start,
                          micro_sym, refcount_mods);

    return callback_def;
}

static char*
S_primitive_callback_def(CFCMethod *method, const char *callback_start,
                         const char *refcount_mods) {
    const char *override_sym = CFCMethod_full_override_sym(method);
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    CFCType *return_type = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    const char *micro_sym = CFCMethod_micro_sym(method);
    char callback_func[50];

    if (CFCType_is_integer(return_type)) {
        strcpy(callback_func, "S_finish_callback_i64");
    }
    else if (CFCType_is_floating(return_type)) {
        strcpy(callback_func, "S_finish_callback_f64");
    }
    else {
        CFCUtil_die("Unexpected type: %s", ret_type_str);
    }

    char pattern[] =
        "%s\n"
        "%s(%s) {\n"
        "%s"
        "    %s retval = (%s)%s(\"%s\");%s\n"
        "    return retval;\n"
        "}\n";
    char *callback_def
        = CFCUtil_sprintf(pattern, ret_type_str, override_sym, params,
                          callback_start, ret_type_str, ret_type_str,
                          callback_func, micro_sym, refcount_mods);

    return callback_def;
}

static char*
S_obj_callback_def(CFCMethod *method, const char *callback_start,
                   const char *refcount_mods) {
    const char *override_sym = CFCMethod_full_override_sym(method);
    const char *params = CFCParamList_to_c(CFCMethod_get_param_list(method));
    CFCType *return_type = CFCMethod_get_return_type(method);
    const char *ret_type_str = CFCType_to_c(return_type);
    const char *micro_sym = CFCMethod_micro_sym(method);
    const char *nullable  = CFCType_nullable(return_type) ? "true" : "false";

    char pattern[] =
        "%s\n"
        "%s(%s) {\n"
        "%s"
        "    %s retval = (%s)S_finish_callback_obj(self, \"%s\", %s);%s\n"
        "    return retval;\n"
        "}\n";
    char *callback_def
        = CFCUtil_sprintf(pattern, ret_type_str, override_sym, params,
                          callback_start, ret_type_str, ret_type_str,
                          micro_sym, nullable, refcount_mods);

    return callback_def;
}

