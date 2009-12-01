#define CHAZ_USE_SHORT_NAMES

#include <string.h>
#include <stdlib.h>
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/CompilerSpec.h"
#include "Charmonizer/Core/OperSys.h"

extern chaz_bool_t chaz_ModHand_charm_run_available;

static void
S_destroy(Compiler *self);

static chaz_bool_t
S_compile_exe(Compiler *self, const char *source_path, const char *exe_name, 
              const char *code, size_t code_len);

static chaz_bool_t
S_compile_obj(Compiler *self, const char *source_path, const char *obj_name, 
              const char *code, size_t code_len);

static void
S_add_inc_dir(Compiler *self, const char *dir);

static void
S_do_test_compile(Compiler *self);

Compiler*
CC_new(OperSys *oper_sys, const char *cc_command, const char *cc_flags)
{
    CompilerSpec *compiler_spec = CCSpec_find_spec();
    Compiler *self = (Compiler*)malloc(sizeof(Compiler));

    if (Util_verbosity)
        printf("Creating compiler object...\n");

    /* assign */
    self->os              = oper_sys;
    self->cc_command      = strdup(cc_command);
    self->cc_flags        = strdup(cc_flags);

    /* init */
    self->compile_exe     = S_compile_exe;
    self->compile_obj     = S_compile_obj;
    self->add_inc_dir     = S_add_inc_dir;
    self->destroy         = S_destroy;
    self->inc_dirs        = (char**)calloc(sizeof(char*), 1);

    /* set compiler-specific vars */
    self->include_flag    = strdup(compiler_spec->include_flag);
    self->object_flag     = strdup(compiler_spec->object_flag);
    self->exe_flag        = strdup(compiler_spec->exe_flag);

    /* add the current directory as an include dir */
    self->add_inc_dir(self, ".");

    /* if we can't compile anything, game over */
    S_do_test_compile(self);

    return self;
}

static void
S_destroy(Compiler *self)
{
    char **inc_dirs;

    for (inc_dirs = self->inc_dirs; *inc_dirs != NULL; inc_dirs++) {
        free(*inc_dirs);
    }
    free(self->inc_dirs);

    free(self->cc_command);
    free(self->cc_flags);
    free(self->include_flag);
    free(self->object_flag);
    free(self->exe_flag);
    free(self);
}

static char*
S_inc_dir_string(Compiler *self)
{
    size_t needed = 0;
    char  *inc_dir_string;
    char **inc_dirs;
    for (inc_dirs = self->inc_dirs; *inc_dirs != NULL; inc_dirs++) {
        needed += strlen(self->include_flag) + 2;
        needed += strlen(*inc_dirs);
    }
    inc_dir_string = malloc(needed + 1);
    inc_dir_string[0] = '\0';
    for (inc_dirs = self->inc_dirs; *inc_dirs != NULL; inc_dirs++) {
        strcat(inc_dir_string, self->include_flag);
        strcat(inc_dir_string, *inc_dirs);
        strcat(inc_dir_string, " ");
    }
    return inc_dir_string;
}

static chaz_bool_t
S_compile_exe(Compiler *self, const char *source_path, const char *exe_name, 
              const char *code, size_t code_len)
{
    OperSys *os                = self->os;
    size_t   exe_file_buf_size = strlen(exe_name) + strlen(os->exe_ext) + 1;
    char    *exe_file          = malloc(exe_file_buf_size);
    size_t   exe_file_buf_len  = sprintf(exe_file, "%s%s", exe_name, os->exe_ext);
    char    *inc_dir_string    = S_inc_dir_string(self);
    size_t   command_max_size  = strlen(os->local_command_start)
                               + strlen(self->cc_command)
                               + strlen(source_path)
                               + strlen(self->exe_flag)
                               + exe_file_buf_len
                               + strlen(inc_dir_string)
                               + strlen(self->cc_flags)
                               + 200;
    char *command = malloc(command_max_size);
    chaz_bool_t result;
    (void)code_len; /* Unused. */
    
    /* Prepare the compiler command. */
    if (Util_verbosity < 2 && chaz_ModHand_charm_run_available) {
        sprintf(command, "%s%s %s %s %s%s %s %s",
            os->local_command_start, "_charm_run ", 
            self->cc_command, source_path, 
            self->exe_flag, exe_file, 
            inc_dir_string,
            self->cc_flags);
    }
    else {
        sprintf(command, "%s %s %s%s %s %s", 
            self->cc_command, source_path,
            self->exe_flag, exe_file,
            inc_dir_string,
            self->cc_flags);
    }

    /* Write the source file. */
    Util_write_file(source_path, code);

    /* Run the compiler command.  See if compilation was successful. */
    system(command);
    result = Util_can_open_file(exe_file);

    free(command);
    free(inc_dir_string);
    free(exe_file);
    return result;
}

static chaz_bool_t
S_compile_obj(Compiler *self, const char *source_path, const char *obj_name, 
              const char *code, size_t code_len)
{
    OperSys *os                = self->os;
    size_t   obj_file_buf_size = strlen(obj_name) + strlen(os->obj_ext) + 1;
    char    *obj_file          = malloc(obj_file_buf_size);
    size_t   obj_file_buf_len  = sprintf(obj_file, "%s%s", obj_name, os->obj_ext);
    char    *inc_dir_string    = S_inc_dir_string(self);
    size_t   command_max_size  = strlen(os->local_command_start)
                               + strlen(self->cc_command)
                               + strlen(source_path)
                               + strlen(self->object_flag)
                               + obj_file_buf_len
                               + strlen(inc_dir_string)
                               + strlen(self->cc_flags)
                               + 200;
    char *command = malloc(command_max_size);
    chaz_bool_t result;
    (void)code_len; /* Unused. */
    
    /* Prepare the compiler command. */
    if (Util_verbosity < 2 && chaz_ModHand_charm_run_available) {
        sprintf(command, "%s%s %s %s %s%s %s %s",
            os->local_command_start, "_charm_run ", 
            self->cc_command, source_path, 
            self->object_flag, obj_file, 
            inc_dir_string,
            self->cc_flags);
    }
    else {
        sprintf(command, "%s %s %s%s %s %s", 
            self->cc_command, source_path,
            self->object_flag, obj_file,
            inc_dir_string,
            self->cc_flags);
    }

    /* Write the source file. */
    Util_write_file(source_path, code);

    /* Run the compiler command.  See if compilation was successful. */
    system(command);
    result = Util_can_open_file(obj_file);

    free(command);
    free(inc_dir_string);
    free(obj_file);
    return result;
}

static void
S_do_test_compile(Compiler *self)
{
    char *code = "int main() { return 0; }\n";
    chaz_bool_t success;
    
    if (Util_verbosity) 
        printf("Trying to compile a small test file...\n");

    /* attempt compilation */
    success = self->compile_exe(self, "_charm_try.c", 
        "_charm_try", code, strlen(code));
    if (!success)
        Util_die("Failed to compile a small test file");
    
    /* clean up */
    remove("_charm_try.c");
    self->os->remove_exe(self->os, "_charm_try");
}

static void
S_add_inc_dir(Compiler *self, const char *dir)
{
    size_t num_dirs = 0; 
    char **dirs = self->inc_dirs;

    /* count up the present number of dirs, reallocate */
    while (*dirs++ != NULL) { num_dirs++; }
    num_dirs += 1; /* passed-in dir */
    self->inc_dirs = realloc(self->inc_dirs, (num_dirs + 1)*sizeof(char*));

    /* put the passed-in dir at the end of the list */
    self->inc_dirs[num_dirs - 1] = strdup(dir);
    self->inc_dirs[num_dirs] = NULL;
}

/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

