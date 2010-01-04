#define CHAZ_USE_SHORT_NAMES

#include <string.h>
#include <stdlib.h>
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

static char     *cc_command   = NULL;
static char     *cc_flags     = NULL;
static char    **inc_dirs     = NULL;

/* Detect a supported compiler, or assume a generic GCC-compatible compiler
 * and hope for the best.  */
#ifdef __GNUC__
static char *compiler_nickname = "gcc";
static char *include_flag      = "-I ";
static char *object_flag       = "-o ";
static char *exe_flag          = "-o ";
#elif defined(_MSC_VER)
static char *compiler_nickname = "MSVC";
static char *include_flag      = "/I";
static char *object_flag       = "/Fo";
static char *exe_flag          = "/Fe";
#else
static char *compiler_nickname = "cc";
static char *include_flag      = "-I ";
static char *object_flag       = "-o ";
static char *exe_flag          = "-o ";
#endif

static void
S_do_test_compile();

void
CC_init(const char *compiler_command, const char *compiler_flags)
{
    if (Util_verbosity) { printf("Creating compiler object...\n"); }

    /* Assign. */
    cc_command      = strdup(compiler_command);
    cc_flags        = strdup(compiler_flags);

    /* Init. */
    inc_dirs              = (char**)calloc(sizeof(char*), 1);

    /* Add the current directory as an include dir. */
    CC_add_inc_dir(".");

    /* If we can't compile anything, game over. */
    S_do_test_compile();
}

void
CC_clean_up()
{
    char **dirs;

    for (dirs = inc_dirs; *dirs != NULL; dirs++) {
        free(*dirs);
    }
    free(inc_dirs);

    free(cc_command);
    free(cc_flags);
}

static char*
S_inc_dir_string()
{
    size_t needed = 0;
    char  *inc_dir_string;
    char **dirs;
    for (dirs = inc_dirs; *dirs != NULL; dirs++) {
        needed += strlen(include_flag) + 2;
        needed += strlen(*dirs);
    }
    inc_dir_string = (char*)malloc(needed + 1);
    inc_dir_string[0] = '\0';
    for (dirs = inc_dirs; *dirs != NULL; dirs++) {
        strcat(inc_dir_string, include_flag);
        strcat(inc_dir_string, *dirs);
        strcat(inc_dir_string, " ");
    }
    return inc_dir_string;
}

chaz_bool_t
CC_compile_exe(const char *source_path, const char *exe_name, 
               const char *code, size_t code_len)
{
    const char *exe_ext        = OS_exe_ext();
    size_t   exe_file_buf_size = strlen(exe_name) + strlen(exe_ext) + 1;
    char    *exe_file          = (char*)malloc(exe_file_buf_size);
    size_t   exe_file_buf_len  = sprintf(exe_file, "%s%s", exe_name, exe_ext);
    char    *inc_dir_string    = S_inc_dir_string();
    size_t   command_max_size  = strlen(cc_command)
                               + strlen(source_path)
                               + strlen(exe_flag)
                               + exe_file_buf_len
                               + strlen(inc_dir_string)
                               + strlen(cc_flags)
                               + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    chaz_bool_t result;
    (void)code_len; /* Unused. */
       
    /* Write the source file. */
    Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (Util_verbosity < 2 && chaz_ConfWriter_charm_run_available) {
        sprintf(command, "%s %s %s %s%s %s %s",
            "_charm_run ", 
            cc_command, source_path, 
            exe_flag, exe_file, 
            inc_dir_string, cc_flags);
        OS_run_local(command, NULL);
    }
    else {
        sprintf(command, "%s %s %s%s %s %s", 
            cc_command, source_path,
            exe_flag, exe_file,
            inc_dir_string, cc_flags);
        system(command);
    }

    /* See if compilation was successful. */
    result = Util_can_open_file(exe_file);

    free(command);
    free(inc_dir_string);
    free(exe_file);
    return result;
}

chaz_bool_t
CC_compile_obj(const char *source_path, const char *obj_name, 
               const char *code, size_t code_len)
{
    const char *obj_ext        = OS_obj_ext();
    size_t   obj_file_buf_size = strlen(obj_name) + strlen(obj_ext) + 1;
    char    *obj_file          = (char*)malloc(obj_file_buf_size);
    size_t   obj_file_buf_len  = sprintf(obj_file, "%s%s", obj_name, obj_ext);
    char    *inc_dir_string    = S_inc_dir_string();
    size_t   command_max_size  = strlen(cc_command)
                               + strlen(source_path)
                               + strlen(object_flag)
                               + obj_file_buf_len
                               + strlen(inc_dir_string)
                               + strlen(cc_flags)
                               + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    chaz_bool_t result;
    (void)code_len; /* Unused. */
    
    /* Write the source file. */
    Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (Util_verbosity < 2 && chaz_ConfWriter_charm_run_available) {
        sprintf(command, "%s %s %s %s%s %s %s",
            "_charm_run ", 
            cc_command, source_path, 
            object_flag, obj_file, 
            inc_dir_string,
            cc_flags);
        OS_run_local(command, NULL);
    }
    else {
        sprintf(command, "%s %s %s%s %s %s", 
            cc_command, source_path,
            object_flag, obj_file,
            inc_dir_string,
            cc_flags);
        system(command);
    }

    /* See if compilation was successful. */
    result = Util_can_open_file(obj_file);

    free(command);
    free(inc_dir_string);
    free(obj_file);
    return result;
}

static void
S_do_test_compile()
{
    char *code = "int main() { return 0; }\n";
    chaz_bool_t success;
    
    if (Util_verbosity) {
        printf("Trying to compile a small test file...\n");
    }

    /* Attempt compilation. */
    success = CC_compile_exe("_charm_try.c", "_charm_try", 
        code, strlen(code));
    if (!success) { Util_die("Failed to compile a small test file"); }
    
    /* Clean up. */
    remove("_charm_try.c");
    OS_remove_exe("_charm_try");
}

void
CC_add_inc_dir(const char *dir)
{
    size_t num_dirs = 0; 
    char **dirs = inc_dirs;

    /* Count up the present number of dirs, reallocate. */
    while (*dirs++ != NULL) { num_dirs++; }
    num_dirs += 1; /* Passed-in dir. */
    inc_dirs = (char**)realloc(inc_dirs, (num_dirs + 1)*sizeof(char*));

    /* Put the passed-in dir at the end of the list. */
    inc_dirs[num_dirs - 1] = strdup(dir);
    inc_dirs[num_dirs] = NULL;
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

