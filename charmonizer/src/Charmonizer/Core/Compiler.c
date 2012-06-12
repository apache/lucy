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

#define CHAZ_USE_SHORT_NAMES

#include <string.h>
#include <stdlib.h>
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

/* Detect macros which may help to identify some compilers.
 */
static void
S_detect_known_compilers(void);

/* Temporary files. */
#define TRY_SOURCE_PATH  "_charmonizer_try.c"
#define TRY_BASENAME     "_charmonizer_try"
#define TARGET_PATH      "_charmonizer_target"

/* Static vars. */
static char     *cc_command   = NULL;
static char     *cc_flags     = NULL;
static char    **inc_dirs     = NULL;
static char     *try_exe_name = NULL;
static char     *try_obj_name = NULL;
static char      include_flag[10] = "";
static char      object_flag[10]  = "";
static char      exe_flag[10]     = "";
static char      error_flag[10]   = "";
static int       defines___GNUC__  = 0;
static int       defines__MSC_VER  = 0;
static int       defines___clang__ = 0;
static int       warnings_as_errors = 0;    

void
CC_set_warnings_as_errors(const int flag) {
    warnings_as_errors = flag;
}

void
CC_init(const char *compiler_command, const char *compiler_flags) {
    const char *code = "int main() { return 0; }\n";
    int compile_succeeded = 0;

    if (Util_verbosity) { printf("Creating compiler object...\n"); }

    /* Assign. */
    cc_command      = Util_strdup(compiler_command);
    cc_flags        = Util_strdup(compiler_flags);

    /* Init. */
    inc_dirs              = (char**)calloc(sizeof(char*), 1);

    /* Add the current directory as an include dir. */
    CC_add_inc_dir(".");

    /* Set names for the targets which we "try" to compile. */
    {
        const char *exe_ext = OS_exe_ext();
        const char *obj_ext = OS_obj_ext();
        size_t exe_len = strlen(TRY_BASENAME) + strlen(exe_ext) + 1;
        size_t obj_len = strlen(TRY_BASENAME) + strlen(obj_ext) + 1;
        try_exe_name = (char*)malloc(exe_len);
        try_obj_name = (char*)malloc(obj_len);
        sprintf(try_exe_name, "%s%s", TRY_BASENAME, exe_ext);
        sprintf(try_obj_name, "%s%s", TRY_BASENAME, obj_ext);
    }

    /* If we can't compile anything, game over. */
    if (Util_verbosity) {
        printf("Trying to compile a small test file...\n");
    }
    /* Try POSIX argument style. */
    strcpy(include_flag, "-I ");
    strcpy(object_flag,  "-o ");
    strcpy(exe_flag,     "-o ");
    compile_succeeded = CC_test_compile(code, strlen(code));
    if (!compile_succeeded) {
        /* Try MSVC argument style. */
        strcpy(include_flag, "/I");
        strcpy(object_flag,  "/Fo");
        strcpy(exe_flag,     "/Fe");
        compile_succeeded = CC_test_compile(code, strlen(code));
    }
    if (!compile_succeeded) {
        Util_die("Failed to compile a small test file");
    }

    S_detect_known_compilers();
}

static const char detect_macro_code[] =
    QUOTE(  int main() {                   )
    QUOTE(  #ifndef %s                     )
    QUOTE(  #error "nope"                  )
    QUOTE(  #endif                         )
    QUOTE(      return 0;                  )
    QUOTE(  }                              );

static int
S_detect_macro(const char *macro) {
    size_t size = sizeof(detect_macro_code) + strlen(macro) + 20;
    char *code = (char*)malloc(size);
    int retval;
    sprintf(code, detect_macro_code, macro);
    retval = CC_test_compile(code, strlen(code));
    free(code);
    return retval;
}

static void
S_detect_known_compilers(void) {
    defines___GNUC__  = S_detect_macro("__GNUC__");
    defines__MSC_VER  = S_detect_macro("_MSC_VER");
    defines___clang__ = S_detect_macro("__clang__");
}

void
CC_clean_up(void) {
    char **dirs;

    for (dirs = inc_dirs; *dirs != NULL; dirs++) {
        free(*dirs);
    }
    free(inc_dirs);

    free(cc_command);
    free(cc_flags);

    free(try_obj_name);
    free(try_exe_name);
}

static char*
S_inc_dir_string(void) {
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

int
CC_compile_exe(const char *source_path, const char *exe_name,
               const char *code, size_t code_len) {
    const char *exe_ext        = OS_exe_ext();
    size_t   exe_file_buf_size = strlen(exe_name) + strlen(exe_ext) + 1;
    char    *exe_file          = (char*)malloc(exe_file_buf_size);
    size_t   junk_buf_size     = exe_file_buf_size + 3;
    char    *junk              = (char*)malloc(junk_buf_size);
    size_t   exe_file_buf_len  = sprintf(exe_file, "%s%s", exe_name, exe_ext);
    char    *inc_dir_string    = S_inc_dir_string();
    if (warnings_as_errors) {
        if (defines__MSC_VER)  {
            strcpy(error_flag, "/WX");
        } else {
            strcpy(error_flag, "-Werror");
        }
    }
    size_t   command_max_size  = strlen(cc_command)
                                 + strlen(error_flag)
                                 + strlen(source_path)
                                 + strlen(exe_flag)
                                 + exe_file_buf_len
                                 + strlen(inc_dir_string)
                                 + strlen(cc_flags)
                                 + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    int result;
    (void)code_len; /* Unused. */

    /* Write the source file. */
    Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    sprintf(command, "%s %s %s %s%s %s %s",
            cc_command, error_flag, 
            source_path, exe_flag, 
            exe_file, inc_dir_string, 
            cc_flags);
    if (Util_verbosity < 2) {
        OS_run_quietly(command);
    }
    else {
        system(command);
    }

    if (defines__MSC_VER) {
        /* Zap MSVC junk. */
        sprintf(junk, "%s.obj", exe_name);
        remove(junk);
        sprintf(junk, "%s.ilk", exe_name);
        remove(junk);
        sprintf(junk, "%s.pdb", exe_name);
        remove(junk);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = Util_can_open_file(exe_file);
    if (!Util_remove_and_verify(source_path)) {
        Util_die("Failed to remove '%s'", source_path);
    }

    free(command);
    free(inc_dir_string);
    free(junk);
    free(exe_file);
    return result;
}

int
CC_compile_obj(const char *source_path, const char *obj_name,
               const char *code, size_t code_len) {
    const char *obj_ext        = OS_obj_ext();
    size_t   obj_file_buf_size = strlen(obj_name) + strlen(obj_ext) + 1;
    char    *obj_file          = (char*)malloc(obj_file_buf_size);
    size_t   obj_file_buf_len  = sprintf(obj_file, "%s%s", obj_name, obj_ext);
    char    *inc_dir_string    = S_inc_dir_string();
    if (warnings_as_errors) {
        if (defines__MSC_VER)  {
            strcpy(error_flag, "/WX");
        } else {
            strcpy(error_flag, "-Werror");
        }
    }

    size_t   command_max_size  = strlen(cc_command)
                                 + strlen(error_flag)
                                 + strlen(source_path)
                                 + strlen(object_flag)
                                 + obj_file_buf_len
                                 + strlen(inc_dir_string)
                                 + strlen(cc_flags)
                                 + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    int result;
    (void)code_len; /* Unused. */

    /* Write the source file. */
    Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    sprintf(command, "%s %s %s %s%s %s %s",
            cc_command, error_flag,
            source_path, object_flag, 
            obj_file, inc_dir_string,
            cc_flags);
    if (Util_verbosity < 2) {
        OS_run_quietly(command);
    }
    else {
        system(command);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = Util_can_open_file(obj_file);
    if (!Util_remove_and_verify(source_path)) {
        Util_die("Failed to remove '%s'", source_path);
    }

    free(command);
    free(inc_dir_string);
    free(obj_file);
    return result;
}

int
CC_test_compile(const char *source, size_t source_len) {
    int compile_succeeded;
    if (!Util_remove_and_verify(try_obj_name)) {
        Util_die("Failed to delete file '%s'", try_obj_name);
    }
    compile_succeeded = CC_compile_obj(TRY_SOURCE_PATH, TRY_BASENAME,
                                       source, source_len);
    remove(try_obj_name);
    return compile_succeeded;
}

char*
CC_capture_output(const char *source, size_t source_len, size_t *output_len) {
    char *captured_output = NULL;
    int compile_succeeded;

    /* Clear out previous versions and test to make sure removal worked. */
    if (!Util_remove_and_verify(try_exe_name)) {
        Util_die("Failed to delete file '%s'", try_exe_name);
    }
    if (!Util_remove_and_verify(TARGET_PATH)) {
        Util_die("Failed to delete file '%s'", TARGET_PATH);
    }

    /* Attempt compilation; if successful, run app and slurp output. */
    compile_succeeded = CC_compile_exe(TRY_SOURCE_PATH, TRY_BASENAME,
                                       source, source_len);
    if (compile_succeeded) {
        OS_run_local(try_exe_name, NULL);
        captured_output = Util_slurp_file(TARGET_PATH, output_len);
    }
    else {
        *output_len = 0;
    }

    /* Remove all the files we just created. */
    remove(TRY_SOURCE_PATH);
    OS_remove_exe(TRY_BASENAME);
    remove(TARGET_PATH);

    return captured_output;
}

void
CC_add_inc_dir(const char *dir) {
    size_t num_dirs = 0;
    char **dirs = inc_dirs;

    /* Count up the present number of dirs, reallocate. */
    while (*dirs++ != NULL) { num_dirs++; }
    num_dirs += 1; /* Passed-in dir. */
    inc_dirs = (char**)realloc(inc_dirs, (num_dirs + 1) * sizeof(char*));

    /* Put the passed-in dir at the end of the list. */
    inc_dirs[num_dirs - 1] = Util_strdup(dir);
    inc_dirs[num_dirs] = NULL;
}

