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

#include "charmony.h"
#include "CFCCClass.h"
#include "CFCClass.h"
#include "CFCDocuComment.h"
#include "CFCFunction.h"
#include "CFCMethod.h"
#include "CFCParamList.h"
#include "CFCSymbol.h"
#include "CFCType.h"
#include "CFCUtil.h"
#include "CFCVariable.h"

#ifndef true
    #define true 1
    #define false 0
#endif

typedef struct CFCPodLink {
    size_t      total_size;
    const char *text;
    size_t      text_size;
} CFCPodLink;

static char*
S_man_create_name(CFCClass *klass);

static char*
S_man_create_synopsis(CFCClass *klass);

static char*
S_man_create_description(CFCClass *klass);

static char*
S_man_create_functions(CFCClass *klass);

static char*
S_man_create_methods(CFCClass *klass);

static char*
S_man_create_inherited_methods(CFCClass *klass);

static char*
S_man_create_func(CFCClass *klass, CFCFunction *func, const char *short_sym,
                  const char *full_sym);

static char*
S_man_create_param_list(CFCFunction *func, const char *full_sym);

static char*
S_man_create_inheritance(CFCClass *klass);

static char*
S_man_escape_content(const char *content);

static void
S_parse_pod_link(const char *content, CFCPodLink *pod_link);

// Declare dummy host callbacks.
char*
CFCCClass_callback_decs(CFCClass *klass) {
    CFCMethod **fresh_methods = CFCClass_fresh_methods(klass);
    char       *cb_decs       = CFCUtil_strdup("");

    for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = fresh_methods[meth_num];

        // Define callback to NULL.
        if (CFCMethod_novel(method) && !CFCMethod_final(method)) {
            const char *override_sym = CFCMethod_full_override_sym(method);
            cb_decs = CFCUtil_cat(cb_decs, "#define ", override_sym, " NULL\n",
                                  NULL);
        }
    }

    FREEMEM(fresh_methods);

    return cb_decs;
}

char*
CFCCClass_create_man_page(CFCClass *klass) {
    if (!CFCSymbol_public((CFCSymbol*)klass)) { return NULL; }

    const char *class_name = CFCClass_get_class_name(klass);

    // Create NAME.
    char *name = S_man_create_name(klass);

    // Create SYNOPSIS.
    char *synopsis = S_man_create_synopsis(klass);

    // Create DESCRIPTION.
    char *description = S_man_create_description(klass);

    // Create CONSTRUCTORS.
    char *functions_man = S_man_create_functions(klass);

    // Create METHODS, possibly including an ABSTRACT METHODS section.
    char *methods_man = S_man_create_methods(klass);

    // Build an INHERITANCE section describing class ancestry.
    char *inheritance = S_man_create_inheritance(klass);

    // Put it all together.
    const char pattern[] =
    ".\\\" Licensed to the Apache Software Foundation (ASF) under one or more\n"
    ".\\\" contributor license agreements.  See the NOTICE file distributed with\n"
    ".\\\" this work for additional information regarding copyright ownership.\n"
    ".\\\" The ASF licenses this file to You under the Apache License, Version 2.0\n"
    ".\\\" (the \"License\"); you may not use this file except in compliance with\n"
    ".\\\" the License.  You may obtain a copy of the License at\n"
    ".\\\"\n"
    ".\\\"     http://www.apache.org/licenses/LICENSE-2.0\n"
    ".\\\"\n"
    ".\\\" Unless required by applicable law or agreed to in writing, software\n"
    ".\\\" distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    ".\\\" WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    ".\\\" See the License for the specific language governing permissions and\n"
    ".\\\" limitations under the License.\n"
    ".TH %s 3\n"
    "%s"
    "%s"
    "%s"
    "%s"
    "%s"
    "%s";
    char *man_page
        = CFCUtil_sprintf(pattern, class_name, name, synopsis, description,
                          functions_man, methods_man, inheritance);

    FREEMEM(name);
    FREEMEM(synopsis);
    FREEMEM(description);
    FREEMEM(functions_man);
    FREEMEM(methods_man);
    FREEMEM(inheritance);

    return man_page;
}

static char*
S_man_create_name(CFCClass *klass) {
    char *result = CFCUtil_strdup(".SH NAME\n");
    result = CFCUtil_cat(result, CFCClass_get_class_name(klass), NULL);

    CFCDocuComment *docucom = CFCClass_get_docucomment(klass);
    if (docucom) {
        const char *raw_brief = CFCDocuComment_get_brief(docucom);
        if (raw_brief && raw_brief[0] != '\0') {
            char *brief = S_man_escape_content(raw_brief);
            result = CFCUtil_cat(result, " \\- ", brief, NULL);
            FREEMEM(brief);
        }
    }

    result = CFCUtil_cat(result, "\n", NULL);

    return result;
}

static char*
S_man_create_synopsis(CFCClass *klass) {
    CHY_UNUSED_VAR(klass);
    return CFCUtil_strdup("");
}

static char*
S_man_create_description(CFCClass *klass) {
    char *result  = CFCUtil_strdup("");

    CFCDocuComment *docucom = CFCClass_get_docucomment(klass);
    if (!docucom) { return result; }

    const char *raw_description = CFCDocuComment_get_long(docucom);
    if (!raw_description || raw_description[0] == '\0') { return result; }

    char *description = S_man_escape_content(raw_description);
    result = CFCUtil_cat(result, ".SH DESCRIPTION\n", description, "\n", NULL);
    FREEMEM(description);

    return result;
}

static char*
S_man_create_functions(CFCClass *klass) {
    CFCFunction **functions = CFCClass_functions(klass);
    char         *result    = CFCUtil_strdup("");

    for (int func_num = 0; functions[func_num] != NULL; func_num++) {
        CFCFunction *func = functions[func_num];
        if (!CFCFunction_public(func)) { continue; }

        if (result[0] == '\0') {
            result = CFCUtil_cat(result, ".SH FUNCTIONS\n", NULL);
        }

        const char *micro_sym     = CFCFunction_micro_sym(func);
        const char *full_func_sym = CFCFunction_full_func_sym(func);

        char *redman = S_man_create_func(klass, func, micro_sym,
                                         full_func_sym);
        result = CFCUtil_cat(result, redman, NULL);
        FREEMEM(redman);
    }

    return result;
}

static char*
S_man_create_methods(CFCClass *klass) {
    CFCMethod **fresh_methods = CFCClass_fresh_methods(klass);
    char       *methods_man   = CFCUtil_strdup("");
    char       *novel_man     = CFCUtil_strdup("");
    char       *result;

    for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = fresh_methods[meth_num];
        if (!CFCMethod_public(method) || !CFCMethod_novel(method)) {
            continue;
        }

        const char *macro_sym = CFCMethod_get_macro_sym(method);
        char *full_method_sym = CFCMethod_full_method_sym(method, NULL);
        char *method_man = S_man_create_func(klass, (CFCFunction*)method,
                                             macro_sym, full_method_sym);

        if (CFCMethod_abstract(method)) {
            if (methods_man[0] == '\0') {
                methods_man = CFCUtil_cat(methods_man,
                                          ".SS Abstract methods\n", NULL);
            }
            methods_man = CFCUtil_cat(methods_man, method_man, NULL);
        }
        else {
            if (novel_man[0] == '\0') {
                novel_man = CFCUtil_cat(novel_man,
                                        ".SS Novel methods\n", NULL);
            }
            novel_man = CFCUtil_cat(novel_man, method_man, NULL);
        }

        FREEMEM(method_man);
        FREEMEM(full_method_sym);
    }

    methods_man = CFCUtil_cat(methods_man, novel_man, NULL);

    // Add methods from parent classes excluding Clownfish::Obj
    CFCClass *parent = CFCClass_get_parent(klass);
    while (parent) {
        if (strcmp(CFCClass_get_class_name(parent), "Clownfish::Obj") == 0) {
            break;
        }
        char *inherited_man = S_man_create_inherited_methods(parent);
        methods_man = CFCUtil_cat(methods_man, inherited_man, NULL);
        FREEMEM(inherited_man);
        parent = CFCClass_get_parent(parent);
    }

    if (methods_man[0] == '\0') {
        result = CFCUtil_strdup("");
    }
    else {
        result = CFCUtil_sprintf(".SH METHODS\n%s", methods_man);
    }

    FREEMEM(methods_man);
    FREEMEM(novel_man);
    FREEMEM(fresh_methods);
    return result;
}

static char*
S_man_create_inherited_methods(CFCClass *klass) {
    CFCMethod **fresh_methods = CFCClass_fresh_methods(klass);
    char       *result        = CFCUtil_strdup("");

    for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = fresh_methods[meth_num];
        if (!CFCMethod_public(method) || !CFCMethod_novel(method)) {
            continue;
        }

        if (result[0] == '\0') {
            result = CFCUtil_cat(result, ".SS Methods inherited from ",
                                 CFCClass_get_class_name(klass), "\n", NULL);
        }

        const char *macro_sym = CFCMethod_get_macro_sym(method);
        char *full_method_sym = CFCMethod_full_method_sym(method, NULL);
        char *method_man = S_man_create_func(klass, (CFCFunction*)method,
                                             macro_sym, full_method_sym);
        result = CFCUtil_cat(result, method_man, NULL);

        FREEMEM(method_man);
        FREEMEM(full_method_sym);
    }

    FREEMEM(fresh_methods);
    return result;
}

static char*
S_man_create_func(CFCClass *klass, CFCFunction *func, const char *short_sym,
                  const char *full_sym) {
    CFCType    *return_type   = CFCFunction_get_return_type(func);
    const char *return_type_c = CFCType_to_c(return_type);
    const char *incremented   = "";

    if (CFCType_incremented(return_type)) {
        incremented = " // incremented";
    }

    char *param_list = S_man_create_param_list(func, full_sym);

    const char *pattern =
        ".TP\n"
        ".B %s\n"
        ".na\n"
        "%s%s\n"
        ".br\n"
        "%s"
        ".ad\n";
    char *result = CFCUtil_sprintf(pattern, short_sym, return_type_c,
                                   incremented, param_list);

    FREEMEM(param_list);

    // Get documentation, which may be inherited.
    CFCDocuComment *docucomment = CFCFunction_get_docucomment(func);
    if (!docucomment) {
        const char *micro_sym = CFCFunction_micro_sym(func);
        CFCClass *parent = klass;
        while (NULL != (parent = CFCClass_get_parent(parent))) {
            CFCFunction *parent_func
                = (CFCFunction*)CFCClass_method(parent, micro_sym);
            if (!parent_func) { break; }
            docucomment = CFCFunction_get_docucomment(parent_func);
            if (docucomment) { break; }
        }
    }

    if (docucomment) {
        // Description
        const char *raw_desc = CFCDocuComment_get_description(docucomment);
        char *desc = S_man_escape_content(raw_desc);
        result = CFCUtil_cat(result, ".IP\n", desc, "\n", NULL);
        FREEMEM(desc);

        // Params
        const char **param_names
            = CFCDocuComment_get_param_names(docucomment);
        const char **param_docs
            = CFCDocuComment_get_param_docs(docucomment);
        if (param_names[0]) {
            result = CFCUtil_cat(result, ".RS\n", NULL);
            for (size_t i = 0; param_names[i] != NULL; i++) {
                char *doc = S_man_escape_content(param_docs[i]);
                result = CFCUtil_cat(result, ".TP\n.I ", param_names[i],
                                     "\n", doc, "\n", NULL);
                FREEMEM(doc);
            }
            result = CFCUtil_cat(result, ".RE\n", NULL);
        }

        // Return value
        const char *retval_doc = CFCDocuComment_get_retval(docucomment);
        if (retval_doc && strlen(retval_doc)) {
            char *doc = S_man_escape_content(retval_doc);
            result = CFCUtil_cat(result, ".IP\n.B Returns:\n", doc, "\n",
                                 NULL);
            FREEMEM(doc);
        }
    }

    return result;
}

static char*
S_man_create_param_list(CFCFunction *func, const char *full_sym) {
    CFCParamList  *param_list = CFCFunction_get_param_list(func);
    CFCVariable  **variables  = CFCParamList_get_variables(param_list);

    if (!variables[0]) {
        return CFCUtil_sprintf(".BR %s (void);\n", full_sym);
    }

    char *result = CFCUtil_sprintf(".BR %s (", full_sym);

    for (int i = 0; variables[i]; ++i) {
        CFCVariable *variable = variables[i];
        CFCType     *type     = CFCVariable_get_type(variable);
        const char  *type_c   = CFCType_to_c(type);
        const char  *name     = CFCVariable_micro_sym(variable);

        result = CFCUtil_cat(result, "\n.br\n.RB \"    ", type_c, " \" ", name,
                             NULL);

        if (variables[i+1] || CFCType_decremented(type)) {
            result = CFCUtil_cat(result, " \"", NULL);
            if (variables[i+1]) {
                result = CFCUtil_cat(result, ",", NULL);
            }
            else {
                result = CFCUtil_cat(result, " // decremented", NULL);
            }
            result = CFCUtil_cat(result, "\"", NULL);
        }
    }

    result = CFCUtil_cat(result, "\n.br\n);\n.br\n", NULL);

    return result;
}

static char*
S_man_create_inheritance(CFCClass *klass) {
    CFCClass *ancestor = CFCClass_get_parent(klass);
    char     *result   = CFCUtil_strdup("");

    if (!ancestor) { return result; }

    const char *class_name = CFCClass_get_class_name(klass);
    result = CFCUtil_cat(result, ".SH INHERITANCE\n", class_name, NULL);
    while (ancestor) {
        const char *ancestor_name = CFCClass_get_class_name(ancestor);
        result = CFCUtil_cat(result, " is a ", ancestor_name, NULL);
        ancestor = CFCClass_get_parent(ancestor);
    }
    result = CFCUtil_cat(result, ".\n", NULL);

    return result;
}

static char*
S_man_escape_content(const char *content) {
    size_t  result_len = 0;
    size_t  result_cap = strlen(content) + 256;
    char   *result     = (char*)MALLOCATE(result_cap + 1);

    for (size_t i = 0; content[i]; i++) {
        const char *subst      = content + i;
        size_t      subst_size = 1;

        switch (content[i]) {
            case '\\':
                // Escape backslash.
                subst      = "\\e";
                subst_size = 2;
                break;
            case '-':
                // Escape hyphen.
                subst      = "\\-";
                subst_size = 2;
                break;
            case '\n':
                // Escape dot after newline.
                if (content[i+1] == '.') {
                    subst      = "\n\\";
                    subst_size = 2;
                }
                break;
            case '<':
                // <code> markup.
                if (strncmp(content + i + 1, "code>", 5) == 0) {
                    subst      = "\\fI";
                    subst_size = 3;
                    i += 5;
                }
                else if (strncmp(content + i + 1, "/code>", 6) == 0) {
                    subst      = "\\fP";
                    subst_size = 3;
                    i += 6;
                }
                break;
            case 'L':
                if (content[i+1] == '<') {
                    // POD-style link.
                    struct CFCPodLink pod_link;
                    S_parse_pod_link(content + i + 2, &pod_link);
                    if (pod_link.total_size) {
                        subst      = pod_link.text;
                        subst_size = pod_link.text_size;
                        i += pod_link.total_size + 1;
                    }
                }
                break;
            default:
                break;
        }

        if (result_len + subst_size > result_cap) {
            result_cap += 256;
            result = (char*)REALLOCATE(result, result_cap + 1);
        }

        memcpy(result + result_len, subst, subst_size);
        result_len += subst_size;
    }

    result[result_len] = '\0';

    return result;
}

// Quick and dirty parsing of POD links. The syntax isn't fully supported
// and the result isn't man-escaped. But it should be good enough for now
// since at some point we'll switch to another format anyway.
static void
S_parse_pod_link(const char *content, CFCPodLink *pod_link) {
    int in_text = true;

    for (size_t i = 0; i < 256 && content[i]; ++i) {
        if (content[i] == '|') {
            if (in_text) {
                pod_link->text_size = i;
                in_text = false;
            }
        }
        else if (content[i] == '>') {
            pod_link->total_size = i + 1;
            pod_link->text       = content;
            if (in_text) {
                pod_link->text_size = i;
            }
            return;
        }
    }

    pod_link->total_size = 0;
    pod_link->text       = NULL;
    pod_link->text_size  = 0;
}

