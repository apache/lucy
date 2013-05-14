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

#include <ctype.h>
#include <string.h>
#include "Charmonizer/Core/Make.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/OperatingSystem.h"
#include "Charmonizer/Core/Util.h"

struct chaz_MakeVar {
    char   *name;
    char   *value;
    size_t  num_elements;
};

struct chaz_MakeRule {
    char *targets;
    char *prereqs;
    char *commands;
};

struct chaz_MakeFile {
    chaz_MakeVar  **vars;
    size_t          num_vars;
    chaz_MakeRule **rules;
    size_t          num_rules;
    chaz_MakeRule  *clean;
    chaz_MakeRule  *distclean;
};

/* Static vars. */
static struct {
    char *make_command;
    int   is_gnu_make;
    int   is_nmake;
    int   shell_type;
} chaz_Make = {
    NULL,
    0, 0, 0
};

/* Detect make command.
 *
 * The argument list must be a NULL-terminated series of different spellings
 * of `make`, which will be auditioned in the order they are supplied.  Here
 * are several possibilities:
 *
 *      make
 *      gmake
 *      nmake
 *      dmake
 */
static int
chaz_Make_detect(const char *make1, ...);

static int
chaz_Make_audition(const char *make);

static chaz_MakeRule*
S_new_rule(const char *target, const char *prereq);

static void
S_destroy_rule(chaz_MakeRule *rule);

static void
S_write_rule(chaz_MakeRule *rule, FILE *out);

void
chaz_Make_init(void) {
    const char *make;

    chaz_Make_detect("make", "gmake", "nmake", "dmake", "mingw32-make",
                     "mingw64-make", NULL);
    make = chaz_Make.make_command;

    if (make) {
        if (strcmp(make, "make") == 0
            || strcmp(make, "gmake") == 0
            || strcmp(make, "mingw32-make") == 0
            || strcmp(make, "mingw64-make") == 0
           ) {
            /* TODO: Add a feature test for GNU make. */
            chaz_Make.is_gnu_make = 1;
            /* TODO: Feature test which shell GNU make uses on Windows. */
            chaz_Make.shell_type = CHAZ_OS_POSIX;
        }
        else if (strcmp(make, "nmake") == 0) {
            chaz_Make.is_nmake = 1;
            chaz_Make.shell_type = CHAZ_OS_CMD_EXE;
        }
    }
}

void
chaz_Make_clean_up(void) {
    free(chaz_Make.make_command);
}

const char*
chaz_Make_get_make(void) {
    return chaz_Make.make_command;
}

int
chaz_Make_shell_type(void) {
    return chaz_Make.shell_type;
}

static int
chaz_Make_detect(const char *make1, ...) {
    va_list args;
    const char *candidate;
    int found = 0;
    const char makefile_content[] = "foo:\n\techo \"foo!\"\n";
    chaz_Util_write_file("_charm_Makefile", makefile_content);

    /* Audition candidates. */
    found = chaz_Make_audition(make1);
    va_start(args, make1);
    while (!found && (NULL != (candidate = va_arg(args, const char*)))) {
        found = chaz_Make_audition(candidate);
    }
    va_end(args);

    chaz_Util_remove_and_verify("_charm_Makefile");

    return found;
}

static int
chaz_Make_audition(const char *make) {
    int succeeded = 0;
    char *command = chaz_Util_join(" ", make, "-f", "_charm_Makefile", NULL);

    chaz_Util_remove_and_verify("_charm_foo");
    chaz_OS_run_redirected(command, "_charm_foo");
    if (chaz_Util_can_open_file("_charm_foo")) {
        size_t len;
        char *content = chaz_Util_slurp_file("_charm_foo", &len);
        if (NULL != strstr(content, "foo!")) {
            succeeded = 1;
        }
        free(content);
    }
    chaz_Util_remove_and_verify("_charm_foo");

    if (succeeded) {
        chaz_Make.make_command = chaz_Util_strdup(make);
    }

    free(command);
    return succeeded;
}

chaz_MakeFile*
chaz_MakeFile_new() {
    chaz_MakeFile *makefile = (chaz_MakeFile*)malloc(sizeof(chaz_MakeFile));

    makefile->vars = (chaz_MakeVar**)malloc(sizeof(chaz_MakeVar*));
    makefile->vars[0] = NULL;
    makefile->num_vars = 0;

    makefile->rules = (chaz_MakeRule**)malloc(sizeof(chaz_MakeRule*));
    makefile->rules[0] = NULL;
    makefile->num_rules = 0;

    makefile->clean     = S_new_rule("clean", NULL);
    makefile->distclean = S_new_rule("distclean", "clean");

    chaz_MakeRule_add_rm_command(makefile->distclean,
                                 "charmonizer$(EXE_EXT) charmonizer$(OBJ_EXT)"
                                 " charmony.h Makefile");

    return makefile;
}

void
chaz_MakeFile_destroy(chaz_MakeFile *makefile) {
    size_t i;

    for (i = 0; makefile->vars[i]; i++) {
        chaz_MakeVar *var = makefile->vars[i];
        free(var->name);
        free(var->value);
        free(var);
    }
    free(makefile->vars);

    for (i = 0; makefile->rules[i]; i++) {
        S_destroy_rule(makefile->rules[i]);
    }
    free(makefile->rules);

    S_destroy_rule(makefile->clean);
    S_destroy_rule(makefile->distclean);

    free(makefile);
}

chaz_MakeVar*
chaz_MakeFile_add_var(chaz_MakeFile *makefile, const char *name,
                      const char *value) {
    chaz_MakeVar  *var      = (chaz_MakeVar*)malloc(sizeof(chaz_MakeVar));
    chaz_MakeVar **vars     = makefile->vars;
    size_t         num_vars = makefile->num_vars + 1;

    var->name         = chaz_Util_strdup(name);
    var->value        = chaz_Util_strdup("");
    var->num_elements = 0;

    if (value) { chaz_MakeVar_append(var, value); }

    vars = (chaz_MakeVar**)realloc(vars,
                                   (num_vars + 1) * sizeof(chaz_MakeVar*));
    vars[num_vars-1] = var;
    vars[num_vars]   = NULL;
    makefile->vars = vars;
    makefile->num_vars = num_vars;

    return var;
}

chaz_MakeRule*
chaz_MakeFile_add_rule(chaz_MakeFile *makefile, const char *target,
                       const char *prereq) {
    chaz_MakeRule  *rule      = S_new_rule(target, prereq);
    chaz_MakeRule **rules     = makefile->rules;
    size_t          num_rules = makefile->num_rules + 1;

    rules = (chaz_MakeRule**)realloc(rules,
                                     (num_rules + 1) * sizeof(chaz_MakeRule*));
    rules[num_rules-1] = rule;
    rules[num_rules]   = NULL;
    makefile->rules = rules;
    makefile->num_rules = num_rules;

    return rule;
}

chaz_MakeRule*
chaz_MakeFile_clean_rule(chaz_MakeFile *makefile) {
    return makefile->clean;
}

chaz_MakeRule*
chaz_MakeFile_distclean_rule(chaz_MakeFile *makefile) {
    return makefile->distclean;
}

chaz_MakeRule*
chaz_MakeFile_add_exe(chaz_MakeFile *makefile, const char *exe,
                      const char *sources, chaz_CFlags *link_flags) {
    chaz_CFlags   *local_flags  = chaz_CC_new_cflags();
    const char    *link         = chaz_CC_link_command();
    const char    *link_flags_string = "";
    const char    *local_flags_string;
    chaz_MakeRule *rule;
    char          *command;

    rule = chaz_MakeFile_add_rule(makefile, exe, sources);

    if (link_flags) {
        link_flags_string = chaz_CFlags_get_string(link_flags);
    }
    if (chaz_CC_msvc_version_num()) {
        chaz_CFlags_append(local_flags, "/nologo");
    }
    chaz_CFlags_set_link_output(local_flags, exe);
    local_flags_string = chaz_CFlags_get_string(local_flags);
    command = chaz_Util_join(" ", link, sources, link_flags_string,
                             local_flags_string, NULL);
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeRule_add_rm_command(makefile->clean, exe);

    chaz_CFlags_destroy(local_flags);
    free(command);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_compiled_exe(chaz_MakeFile *makefile, const char *exe,
                               const char *sources, chaz_CFlags *cflags) {
    chaz_CFlags   *local_flags   = chaz_CC_new_cflags();
    const char    *cc            = chaz_CC_get_cc();
    const char    *cflags_string = "";
    const char    *local_flags_string;
    chaz_MakeRule *rule;
    char          *command;

    rule = chaz_MakeFile_add_rule(makefile, exe, sources);

    if (cflags) {
        cflags_string = chaz_CFlags_get_string(cflags);
    }
    if (chaz_CC_msvc_version_num()) {
        chaz_CFlags_append(local_flags, "/nologo");
    }
    chaz_CFlags_set_output_exe(local_flags, exe);
    local_flags_string = chaz_CFlags_get_string(local_flags);
    command = chaz_Util_join(" ", cc, sources, cflags_string,
                             local_flags_string, NULL);
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeRule_add_rm_command(makefile->clean, exe);
    /* TODO: Clean .obj file on Windows. */

    chaz_CFlags_destroy(local_flags);
    free(command);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_shared_lib(chaz_MakeFile *makefile, chaz_SharedLib *lib,
                             const char *sources, chaz_CFlags *link_flags) {
    chaz_CFlags   *local_flags  = chaz_CC_new_cflags();
    const char    *link         = chaz_CC_link_command();
    const char    *link_flags_string = "";
    const char    *local_flags_string;
    chaz_MakeRule *rule;
    char          *filename;
    char          *command;

    filename = chaz_SharedLib_filename(lib);
    rule = chaz_MakeFile_add_rule(makefile, filename, sources);

    if (link_flags) {
        link_flags_string = chaz_CFlags_get_string(link_flags);
    }
    if (chaz_CC_msvc_version_num()) {
        chaz_CFlags_append(local_flags, "/nologo");
    }
    chaz_CFlags_link_shared_library(local_flags);
    chaz_CFlags_set_shared_library_version(local_flags, lib);
    chaz_CFlags_set_link_output(local_flags, filename);
    local_flags_string = chaz_CFlags_get_string(local_flags);
    command = chaz_Util_join(" ", link, sources, link_flags_string,
                             local_flags_string, NULL);
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeRule_add_rm_command(makefile->clean, filename);

    if (chaz_CC_msvc_version_num()) {
        /* Remove import library and export file under MSVC. */
        char *lib_filename = chaz_SharedLib_implib_filename(lib);
        char *exp_filename = chaz_SharedLib_export_filename(lib);
        chaz_MakeRule_add_rm_command(makefile->clean, lib_filename);
        chaz_MakeRule_add_rm_command(makefile->clean, exp_filename);
        free(lib_filename);
        free(exp_filename);
    }

    chaz_CFlags_destroy(local_flags);
    free(filename);
    free(command);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_lemon_exe(chaz_MakeFile *makefile, const char *dir) {
    chaz_CFlags   *cflags = chaz_CC_new_cflags();
    chaz_MakeRule *rule;
    const char *dir_sep = chaz_OS_dir_sep();
    const char *exe_ext = chaz_OS_exe_ext();
    char *lemon_exe = chaz_Util_join("", dir, dir_sep, "lemon", exe_ext, NULL);
    char *lemon_c   = chaz_Util_join(dir_sep, dir, "lemon.c", NULL);

    chaz_CFlags_enable_optimization(cflags);
    chaz_MakeFile_add_var(makefile, "LEMON_EXE", lemon_exe);
    rule = chaz_MakeFile_add_compiled_exe(makefile, "$(LEMON_EXE)", lemon_c,
                                          cflags);

    chaz_CFlags_destroy(cflags);
    free(lemon_c);
    free(lemon_exe);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_lemon_grammar(chaz_MakeFile *makefile,
                                const char *base_name) {
    char *c_file  = chaz_Util_join(".", base_name, "c", NULL);
    char *h_file  = chaz_Util_join(".", base_name, "h", NULL);
    char *y_file  = chaz_Util_join(".", base_name, "y", NULL);
    char *command = chaz_Util_join(" ", "$(LEMON_EXE) -q", y_file, NULL);

    chaz_MakeRule *rule = chaz_MakeFile_add_rule(makefile, c_file, y_file);
    chaz_MakeRule *clean_rule = chaz_MakeFile_clean_rule(makefile);

    chaz_MakeRule_add_prereq(rule, "$(LEMON_EXE)");
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeRule_add_rm_command(clean_rule, h_file);
    chaz_MakeRule_add_rm_command(clean_rule, c_file);

    free(c_file);
    free(h_file);
    free(y_file);
    free(command);
    return rule;
}

void
chaz_MakeFile_write(chaz_MakeFile *makefile) {
    FILE   *out;
    size_t  i;

    out = fopen("Makefile", "w");
    if (!out) {
        chaz_Util_die("Can't open Makefile\n");
    }

    for (i = 0; makefile->vars[i]; i++) {
        chaz_MakeVar *var = makefile->vars[i];
        fprintf(out, "%s = %s\n", var->name, var->value);
    }
    fprintf(out, "\n");

    for (i = 0; makefile->rules[i]; i++) {
        S_write_rule(makefile->rules[i], out);
    }

    S_write_rule(makefile->clean, out);
    S_write_rule(makefile->distclean, out);

    if (chaz_Make.is_nmake) {
        /* Inference rule for .c files. */
        fprintf(out, ".c.obj :\n");
        if (chaz_CC_msvc_version_num()) {
            fprintf(out, "\t$(CC) /nologo $(CFLAGS) /c $< /Fo$@\n\n");
        }
        else {
            fprintf(out, "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");
        }
    }

    fclose(out);
}

void
chaz_MakeVar_append(chaz_MakeVar *var, const char *element) {
    char *value;

    if (element[0] == '\0') { return; }

    if (var->num_elements == 0) {
        value = chaz_Util_strdup(element);
    }
    else {
        value = (char*)malloc(strlen(var->value) + strlen(element) + 20);

        if (var->num_elements == 1) {
            sprintf(value, "\\\n    %s \\\n    %s", var->value, element);
        }
        else {
            sprintf(value, "%s \\\n    %s", var->value, element);
        }
    }

    free(var->value);
    var->value = value;
    var->num_elements++;
}

static chaz_MakeRule*
S_new_rule(const char *target, const char *prereq) {
    chaz_MakeRule *rule = (chaz_MakeRule*)malloc(sizeof(chaz_MakeRule));

    rule->targets  = NULL;
    rule->prereqs  = NULL;
    rule->commands = NULL;

    if (target) { chaz_MakeRule_add_target(rule, target); }
    if (prereq) { chaz_MakeRule_add_prereq(rule, prereq); }

    return rule;
}

static void
S_destroy_rule(chaz_MakeRule *rule) {
    if (rule->targets)  { free(rule->targets); }
    if (rule->prereqs)  { free(rule->prereqs); }
    if (rule->commands) { free(rule->commands); }
    free(rule);
}

static void
S_write_rule(chaz_MakeRule *rule, FILE *out) {
    fprintf(out, "%s :", rule->targets);
    if (rule->prereqs) {
        fprintf(out, " %s", rule->prereqs);
    }
    fprintf(out, "\n");
    if (rule->commands) {
        fprintf(out, "%s", rule->commands);
    }
    fprintf(out, "\n");
}

void
chaz_MakeRule_add_target(chaz_MakeRule *rule, const char *target) {
    char *targets;

    if (!rule->targets) {
        targets = chaz_Util_strdup(target);
    }
    else {
        targets = chaz_Util_join(" ", rule->targets, target, NULL);
        free(rule->targets);
    }

    rule->targets = targets;
}

void
chaz_MakeRule_add_prereq(chaz_MakeRule *rule, const char *prereq) {
    char *prereqs;

    if (!rule->prereqs) {
        prereqs = chaz_Util_strdup(prereq);
    }
    else {
        prereqs = chaz_Util_join(" ", rule->prereqs, prereq, NULL);
        free(rule->prereqs);
    }

    rule->prereqs = prereqs;
}

void
chaz_MakeRule_add_command(chaz_MakeRule *rule, const char *command) {
    char *commands;

    if (!rule->commands) {
        commands = (char*)malloc(strlen(command) + 20);
        sprintf(commands, "\t%s\n", command);
    }
    else {
        commands = (char*)malloc(strlen(rule->commands) + strlen(command) + 20);
        sprintf(commands, "%s\t%s\n", rule->commands, command);
        free(rule->commands);
    }

    rule->commands = commands;
}

void
chaz_MakeRule_add_rm_command(chaz_MakeRule *rule, const char *files) {
    char *command;

    if (chaz_Make.shell_type == CHAZ_OS_POSIX) {
        command = chaz_Util_join(" ", "rm -f", files, NULL);
    }
    else if (chaz_Make.shell_type == CHAZ_OS_CMD_EXE) {
        command = chaz_Util_join("", "for %i in (", files,
                                 ") do @if exist %i del /f %i", NULL);
    }
    else {
        chaz_Util_die("Unsupported shell type: %d", chaz_Make.shell_type);
    }

    chaz_MakeRule_add_command(rule, command);
    free(command);
}

void
chaz_MakeRule_add_recursive_rm_command(chaz_MakeRule *rule, const char *dirs) {
    char *command;

    if (chaz_Make.shell_type == CHAZ_OS_POSIX) {
        command = chaz_Util_join(" ", "rm -rf", dirs, NULL);
    }
    else if (chaz_Make.shell_type == CHAZ_OS_CMD_EXE) {
        command = chaz_Util_join("", "for %i in (", dirs,
                                 ") do @if exist %i rmdir /s /q %i", NULL);
    }
    else {
        chaz_Util_die("Unsupported shell type: %d", chaz_Make.shell_type);
    }

    chaz_MakeRule_add_command(rule, command);
    free(command);
}

void
chaz_MakeRule_add_make_command(chaz_MakeRule *rule, const char *dir,
                               const char *target) {
    char *command;

    if (chaz_Make.is_gnu_make) {
        if (!target) {
            command = chaz_Util_join(" ", "$(MAKE)", "-C", dir, NULL);
        }
        else {
            command = chaz_Util_join(" ", "$(MAKE)", "-C", dir, target, NULL);
        }
        chaz_MakeRule_add_command(rule, command);
        free(command);
    }
    else if (chaz_Make.is_nmake) {
        command = chaz_Util_join(" ", "cd", dir, NULL);
        chaz_MakeRule_add_command(rule, command);
        free(command);

        if (!target) {
            chaz_MakeRule_add_command(rule, "$(MAKE) /nologo");
        }
        else {
            command = chaz_Util_join(" ", "$(MAKE) /nologo", target, NULL);
            chaz_MakeRule_add_command(rule, command);
            free(command);
        }

        chaz_MakeRule_add_command(rule, "cd $(MAKEDIR)");
    }
    else {
        chaz_Util_die("Couldn't find a supported 'make' utility.");
    }
}

void
chaz_Make_list_files(const char *dir, const char *ext,
                     chaz_Make_list_files_callback_t callback, void *context) {
    int         shell_type = chaz_OS_shell_type();
    const char *pattern;
    char       *command;
    char       *list;
    char       *prefix;
    char       *file;
    size_t      command_size;
    size_t      list_len;
    size_t      prefix_len;

    /* List files using shell. */

    if (shell_type == CHAZ_OS_POSIX) {
        pattern = "find %s -name '*.%s' -type f";
    }
    else if (shell_type == CHAZ_OS_CMD_EXE) {
        pattern = "dir %s\\*.%s /s /b /a-d";
    }
    else {
        chaz_Util_die("Unknown shell type %d", shell_type);
    }

    command_size = strlen(pattern) + strlen(dir) + strlen(ext) + 10;
    command = (char*)malloc(command_size);
    sprintf(command, pattern, dir, ext);
    list = chaz_OS_run_and_capture(command, &list_len);
    free(command);
    if (!list) {
        chaz_Util_die("Failed to list files in '%s'", dir);
    }
    list[list_len-1] = 0;

    /* Find directory prefix to strip from files */

    if (shell_type == CHAZ_OS_POSIX) {
        prefix_len = strlen(dir);
        prefix = (char*)malloc(prefix_len + 2);
        memcpy(prefix, dir, prefix_len);
        prefix[prefix_len++] = '/';
        prefix[prefix_len]   = '\0';
    }
    else {
        char   *output;
        size_t  output_len;

        /* 'dir /s' returns absolute paths, so we have to find the absolute
         * path of the directory. This is done by using the variable
         * substitution feature of the 'for' command.
         */
        pattern = "for %%I in (%s) do @echo %%~fI";
        command_size = strlen(pattern) + strlen(dir) + 10;
        command = (char*)malloc(command_size);
        sprintf(command, pattern, dir);
        output = chaz_OS_run_and_capture(command, &output_len);
        free(command);
        if (!output) { chaz_Util_die("Failed to find absolute path"); }

        /* Strip whitespace from end of output. */
        for (prefix_len = output_len; prefix_len > 0; --prefix_len) {
            if (!isspace(output[prefix_len-1])) { break; }
        }
        prefix = (char*)malloc(prefix_len + 2);
        memcpy(prefix, output, prefix_len);
        prefix[prefix_len++] = '\\';
        prefix[prefix_len]   = '\0';
        free(output);
    }

    /* Iterate file list and invoke callback. */

    for (file = strtok(list, "\r\n"); file; file = strtok(NULL, "\r\n")) {
        if (strlen(file) <= prefix_len
            || memcmp(file, prefix, prefix_len) != 0
           ) {
            chaz_Util_die("Expected prefix '%s' for file name '%s'", prefix,
                          file);
        }

        callback(dir, file + prefix_len, context);
    }

    free(prefix);
    free(list);
}

