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
    char          **cleanups;
    size_t          num_cleanups;
};

/* Static vars. */
static struct {
    char *make_command;
    int   is_gnu_make;
    int   is_nmake;
} chaz_Make = {
    NULL,
    0, 0
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

void
chaz_Make_init(void) {
    const char *make;

    chaz_Make_detect("make", "gmake", "nmake", "dmake", NULL);
    make = chaz_Make.make_command;

    if (make) {
        if (strcmp(make, "make") == 0 || strcmp(make, "gmake") == 0) {
            /* TODO: Add a feature test for GNU make. */
            chaz_Make.is_gnu_make = 1;
        }
        else if (strcmp(make, "nmake") == 0) {
            chaz_Make.is_nmake = 1;
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
    const char pattern[] = "%s -f _charm_Makefile";
    size_t size = strlen(make) + sizeof(pattern) + 10;
    char *command = (char*)malloc(size);
    sprintf(command, pattern, make);

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

    makefile->cleanups = (char**)malloc(sizeof(char*));
    makefile->cleanups[0] = NULL;
    makefile->num_cleanups = 0;

    return makefile;
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
    chaz_MakeRule  *rule      = (chaz_MakeRule*)malloc(sizeof(chaz_MakeRule));
    chaz_MakeRule **rules     = makefile->rules;
    size_t          num_rules = makefile->num_rules + 1;

    rule->targets  = NULL;
    rule->prereqs  = NULL;
    rule->commands = NULL;

    if (target) { chaz_MakeRule_add_target(rule, target); }
    if (prereq) { chaz_MakeRule_add_prereq(rule, prereq); }

    rules = (chaz_MakeRule**)realloc(rules,
                                     (num_rules + 1) * sizeof(chaz_MakeRule*));
    rules[num_rules-1] = rule;
    rules[num_rules]   = NULL;
    makefile->rules = rules;
    makefile->num_rules = num_rules;

    return rule;
}

void
chaz_MakeFile_add_to_cleanup(chaz_MakeFile *makefile, const char *target) {
    char    *cleanup      = chaz_Util_strdup(target);
    char   **cleanups     = makefile->cleanups;
    size_t   num_cleanups = makefile->num_cleanups + 1;

    cleanups = (char**)realloc(cleanups, (num_cleanups + 1) * sizeof(char*));
    cleanups[num_cleanups-1] = cleanup;
    cleanups[num_cleanups]   = NULL;
    makefile->cleanups = cleanups;
    makefile->num_cleanups = num_cleanups;
}

void
chaz_MakeFile_add_exe(chaz_MakeFile *makefile, const char *exe,
                      const char *objects) {
    const char    *pattern     = "%s %s %s %s%s";
    const char    *link        = chaz_CC_link_command();
    const char    *link_flags  = chaz_CC_link_flags();
    const char    *output_flag = chaz_CC_link_output_flag();
    chaz_MakeRule *rule;
    char          *command;
    size_t         size;

    rule = chaz_MakeFile_add_rule(makefile, exe, objects);

    size = strlen(pattern)
           + strlen(link)
           + strlen(link_flags)
           + strlen(objects)
           + strlen(output_flag)
           + strlen(exe)
           + 50;
    command = (char*)malloc(size);
    sprintf(command, pattern, link, link_flags, objects, output_flag, exe);
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeFile_add_to_cleanup(makefile, exe);
}

void
chaz_MakeFile_add_shared_obj(chaz_MakeFile *makefile, const char *shared_obj,
                             const char *objects) {
    const char    *pattern     = "%s %s %s %s %s%s";
    const char    *link        = chaz_CC_link_command();
    const char    *shobj_flags = chaz_CC_link_shared_obj_flag();
    const char    *link_flags  = chaz_CC_link_flags();
    const char    *output_flag = chaz_CC_link_output_flag();
    chaz_MakeRule *rule;
    char          *command;
    size_t         size;

    rule = chaz_MakeFile_add_rule(makefile, shared_obj, objects);

    size = strlen(pattern)
           + strlen(link)
           + strlen(shobj_flags)
           + strlen(link_flags)
           + strlen(objects)
           + strlen(output_flag)
           + strlen(shared_obj)
           + 50;
    command = (char*)malloc(size);
    sprintf(command, pattern, link, shobj_flags, link_flags, objects,
            output_flag, shared_obj);
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeFile_add_to_cleanup(makefile, shared_obj);
}

void
chaz_MakeFile_write(chaz_MakeFile *makefile) {
    int     shell_type = chaz_OS_shell_type();
    FILE   *file;
    size_t  i;

    file = fopen("Makefile", "w");
    if (!file) {
        chaz_Util_die("Can't open Makefile\n");
    }

    for (i = 0; makefile->vars[i]; i++) {
        chaz_MakeVar *var = makefile->vars[i];
        fprintf(file, "%s = %s\n", var->name, var->value);
    }
    fprintf(file, "\n");

    for (i = 0; makefile->rules[i]; i++) {
        chaz_MakeRule *rule = makefile->rules[i];
        fprintf(file, "%s :", rule->targets);
        if (rule->prereqs) {
            fprintf(file, " %s", rule->prereqs);
        }
        fprintf(file, "\n");
        if (rule->commands) {
            fprintf(file, "%s", rule->commands);
        }
        fprintf(file, "\n");
    }

    if (makefile->cleanups[0]) {
        if (shell_type == CHAZ_OS_POSIX) {
            fprintf(file, "clean :\n\trm -f");
            for (i = 0; makefile->cleanups[i]; i++) {
                const char *cleanup = makefile->cleanups[i];
                fprintf(file, " \\\n\t    %s", cleanup);
            }
            fprintf(file, "\n\n");
        }
        else if (shell_type == CHAZ_OS_CMD_EXE) {
            fprintf(file, "clean :\n");
            for (i = 0; makefile->cleanups[i]; i++) {
                const char *cleanup = makefile->cleanups[i];
                fprintf(file, "\tfor %%i in (%s) do @if exist %%i del /f %%i\n",
                        cleanup);
            }
            fprintf(file, "\n");
        }
        else {
            chaz_Util_die("Unsupported shell type: %d", shell_type);
        }
    }

    fprintf(file, "distclean : clean\n");
    if (shell_type == CHAZ_OS_POSIX) {
        fprintf(file, "\trm -f charmonizer$(EXE_EXT) charmony.h Makefile\n\n");
    }
    else if (shell_type == CHAZ_OS_CMD_EXE) {
        fprintf(file,
            "\tfor %%i in (charmonizer$(EXE_EXT) charmonizer$(OBJ_EXT)"
            " charmony.h Makefile) do @if exist %%i del /f %%i\n\n");
    }
    else {
        chaz_Util_die("Unsupported shell type: %d", shell_type);
    }

    if (chaz_Make.is_nmake) {
        /* Inference rule for .c files. */
        fprintf(file, ".c.obj :\n");
        if (chaz_CC_msvc_version_num()) {
            fprintf(file, "\t$(CC) $(CFLAGS) /c $< /Fo$@\n\n");
        }
        else {
            fprintf(file, "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");
        }
    }

    fclose(file);
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

void
chaz_MakeRule_add_target(chaz_MakeRule *rule, const char *target) {
    char *targets;

    if (!rule->targets) {
        targets = chaz_Util_strdup(target);
    }
    else {
        targets = (char*)malloc(strlen(rule->targets) + strlen(target) + 20);
        sprintf(targets, "%s %s", rule->targets, target);
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
        prereqs = (char*)malloc(strlen(rule->prereqs) + strlen(prereq) + 20);
        sprintf(prereqs, "%s %s", rule->prereqs, prereq);
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
chaz_MakeRule_add_command_make(chaz_MakeRule *rule, const char *dir,
                               const char *target) {
    const char *make = chaz_Make.make_command;
    char *command;

    if (chaz_Make.is_gnu_make) {
        if (!target) {
            size_t size = strlen(dir) + 20;
            command = (char*)malloc(size);
            sprintf(command, "$(MAKE) -C %s", dir);
        }
        else {
            size_t size = strlen(dir) + strlen(target) + 20;
            command = (char*)malloc(size);
            sprintf(command, "$(MAKE) -C %s %s", dir, target);
        }
        chaz_MakeRule_add_command(rule, command);
        free(command);
    }
    else if (chaz_Make.is_nmake) {
        command = (char*)malloc(strlen(dir) + 20);
        sprintf(command, "cd %s", dir);
        chaz_MakeRule_add_command(rule, command);
        free(command);

        if (!target) {
            chaz_MakeRule_add_command(rule, "$(MAKE)");
        }
        else {
            size_t size = strlen(target) + 20;
            command = (char*)malloc(size);
            sprintf(command, "$(MAKE) %s", target);
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

        callback(file + prefix_len, context);
    }

    free(prefix);
    free(list);
}

