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
#include <stdlib.h>
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

/* Detect macros which may help to identify some compilers.
 */
static void
chaz_CC_detect_known_compilers(void);

/* Temporary files. */
#define CHAZ_CC_TRY_SOURCE_PATH  "_charmonizer_try.c"
#define CHAZ_CC_TRY_BASENAME     "_charmonizer_try"
#define CHAZ_CC_TARGET_PATH      "_charmonizer_target"

/* Static vars. */
static struct {
    char     *cc_command;
    char     *cc_flags;
    char     *try_exe_name;
    char     *try_obj_name;
    char      include_flag[10];
    char      object_flag[10];
    char      exe_flag[10];
    char      no_link_flag[10];
    char      error_flag[10];
    int       intval___GNUC__;
    int       intval__MSC_VER;
    int       intval___clang__;
    int       warnings_as_errors;
} chaz_CC = {
    NULL, NULL, NULL, NULL,
    "", "", "", "", "",
    0, 0, 0, 0
};

void
chaz_CC_set_warnings_as_errors(const int flag) {
    chaz_CC.warnings_as_errors = flag;
    if (chaz_CC.warnings_as_errors) {
        if (chaz_CC.intval__MSC_VER)  {
            strcpy(chaz_CC.error_flag, "/WX");
        } else {
            strcpy(chaz_CC.error_flag, "-Werror");
        }
    }
    else {
        strcpy(chaz_CC.error_flag, "");
    }
}

void
chaz_CC_init(const char *compiler_command, const char *compiler_flags) {
    const char *code = "int main() { return 0; }\n";
    int compile_succeeded = 0;

    if (chaz_Util_verbosity) { printf("Creating compiler object...\n"); }

    /* Assign. */
    chaz_CC.cc_command      = chaz_Util_strdup(compiler_command);
    chaz_CC.cc_flags        = chaz_Util_strdup(compiler_flags);

    /* Set names for the targets which we "try" to compile. */
    {
        const char *exe_ext = chaz_OS_exe_ext();
        const char *obj_ext = chaz_OS_obj_ext();
        size_t exe_len = strlen(CHAZ_CC_TRY_BASENAME) + strlen(exe_ext) + 1;
        size_t obj_len = strlen(CHAZ_CC_TRY_BASENAME) + strlen(obj_ext) + 1;
        chaz_CC.try_exe_name = (char*)malloc(exe_len);
        chaz_CC.try_obj_name = (char*)malloc(obj_len);
        sprintf(chaz_CC.try_exe_name, "%s%s", CHAZ_CC_TRY_BASENAME, exe_ext);
        sprintf(chaz_CC.try_obj_name, "%s%s", CHAZ_CC_TRY_BASENAME, obj_ext);
    }

    /* If we can't compile anything, game over. */
    if (chaz_Util_verbosity) {
        printf("Trying to compile a small test file...\n");
    }
    /* Try POSIX argument style. */
    strcpy(chaz_CC.include_flag, "-I ");
    strcpy(chaz_CC.object_flag,  "-o ");
    strcpy(chaz_CC.exe_flag,     "-o ");
    strcpy(chaz_CC.no_link_flag, "-c ");
    compile_succeeded = chaz_CC_test_compile(code);
    if (!compile_succeeded) {
        /* Try MSVC argument style. */
        strcpy(chaz_CC.include_flag, "/I");
        strcpy(chaz_CC.object_flag,  "/Fo");
        strcpy(chaz_CC.exe_flag,     "/Fe");
        strcpy(chaz_CC.no_link_flag, "/c");
        compile_succeeded = chaz_CC_test_compile(code);
    }
    if (!compile_succeeded) {
        chaz_Util_die("Failed to compile a small test file");
    }

    chaz_CC_detect_known_compilers();
}

static const char chaz_CC_detect_macro_code[] =
    CHAZ_QUOTE(  #include <stdio.h>             )
    CHAZ_QUOTE(  int main() {                   )
    CHAZ_QUOTE(  #ifndef %s                     )
    CHAZ_QUOTE(  #error "nope"                  )
    CHAZ_QUOTE(  #endif                         )
    CHAZ_QUOTE(      printf("%%d", %s);         )
    CHAZ_QUOTE(      return 0;                  )
    CHAZ_QUOTE(  }                              );

static int
chaz_CC_detect_macro(const char *macro) {
    size_t size = sizeof(chaz_CC_detect_macro_code)
                  + (strlen(macro) * 2)
                  + 20;
    char *code = (char*)malloc(size);
    int retval = 0;
    char *output;
    size_t len;
    sprintf(code, chaz_CC_detect_macro_code, macro, macro);
    output = chaz_CC_capture_output(code, &len);
    if (output) {
        retval = atoi(output);
        free(output);
    }
    free(code);
    return retval;
}

static void
chaz_CC_detect_known_compilers(void) {
    chaz_CC.intval___GNUC__  = chaz_CC_detect_macro("__GNUC__");
    chaz_CC.intval__MSC_VER  = chaz_CC_detect_macro("_MSC_VER");
    chaz_CC.intval___clang__ = chaz_CC_detect_macro("__clang__");
}

void
chaz_CC_clean_up(void) {
    free(chaz_CC.cc_command);
    free(chaz_CC.cc_flags);
    free(chaz_CC.try_obj_name);
    free(chaz_CC.try_exe_name);
}

int
chaz_CC_compile_exe(const char *source_path, const char *exe_name,
                    const char *code) {
    const char *exe_ext        = chaz_OS_exe_ext();
    size_t   exe_file_buf_size = strlen(exe_name) + strlen(exe_ext) + 1;
    char    *exe_file          = (char*)malloc(exe_file_buf_size);
    size_t   junk_buf_size     = exe_file_buf_size + 3;
    char    *junk              = (char*)malloc(junk_buf_size);
    size_t   exe_file_buf_len  = sprintf(exe_file, "%s%s", exe_name, exe_ext);
    size_t   command_max_size  = strlen(chaz_CC.cc_command)
                                 + strlen(chaz_CC.error_flag)
                                 + strlen(source_path)
                                 + strlen(chaz_CC.exe_flag)
                                 + exe_file_buf_len
                                 + strlen(chaz_CC.cc_flags)
                                 + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    sprintf(command, "%s %s %s %s%s %s",
            chaz_CC.cc_command, chaz_CC.error_flag, 
            source_path, chaz_CC.exe_flag, 
            exe_file,
            chaz_CC.cc_flags);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        system(command);
    }

    if (chaz_CC.intval__MSC_VER) {
        /* Zap MSVC junk. */
        sprintf(junk, "%s.obj", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.ilk", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.pdb", exe_name);
        chaz_Util_remove_and_verify(junk);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(exe_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    free(command);
    free(junk);
    free(exe_file);
    return result;
}

int
chaz_CC_compile_obj(const char *source_path, const char *obj_name,
                    const char *code) {
    const char *obj_ext        = chaz_OS_obj_ext();
    size_t   obj_file_buf_size = strlen(obj_name) + strlen(obj_ext) + 1;
    char    *obj_file          = (char*)malloc(obj_file_buf_size);
    size_t   obj_file_buf_len  = sprintf(obj_file, "%s%s", obj_name, obj_ext);
    size_t   command_max_size  = strlen(chaz_CC.cc_command)
                                 + strlen(chaz_CC.no_link_flag)
                                 + strlen(chaz_CC.error_flag)
                                 + strlen(source_path)
                                 + strlen(chaz_CC.object_flag)
                                 + obj_file_buf_len
                                 + strlen(chaz_CC.cc_flags)
                                 + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    sprintf(command, "%s %s %s %s %s%s %s",
            chaz_CC.cc_command, chaz_CC.no_link_flag, chaz_CC.error_flag,
            source_path, chaz_CC.object_flag, 
            obj_file,
            chaz_CC.cc_flags);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        system(command);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(obj_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    free(command);
    free(obj_file);
    return result;
}

int
chaz_CC_test_compile(const char *source) {
    int compile_succeeded;
    if (!chaz_Util_remove_and_verify(chaz_CC.try_obj_name)) {
        chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_obj_name);
    }
    compile_succeeded = chaz_CC_compile_obj(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    chaz_Util_remove_and_verify(chaz_CC.try_obj_name);
    return compile_succeeded;
}

char*
chaz_CC_capture_output(const char *source, size_t *output_len) {
    char *captured_output = NULL;
    int compile_succeeded;

    /* Clear out previous versions and test to make sure removal worked. */
    if (!chaz_Util_remove_and_verify(chaz_CC.try_exe_name)) {
        chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_exe_name);
    }
    if (!chaz_Util_remove_and_verify(CHAZ_CC_TARGET_PATH)) {
        chaz_Util_die("Failed to delete file '%s'", CHAZ_CC_TARGET_PATH);
    }

    /* Attempt compilation; if successful, run app and slurp output. */
    compile_succeeded = chaz_CC_compile_exe(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    if (compile_succeeded) {
        chaz_OS_run_local_redirected(chaz_CC.try_exe_name,
                                     CHAZ_CC_TARGET_PATH);
        captured_output = chaz_Util_slurp_file(CHAZ_CC_TARGET_PATH,
                                               output_len);
    }
    else {
        *output_len = 0;
    }

    /* Remove all the files we just created. */
    chaz_Util_remove_and_verify(CHAZ_CC_TRY_SOURCE_PATH);
    chaz_Util_remove_and_verify(chaz_CC.try_exe_name);
    chaz_Util_remove_and_verify(CHAZ_CC_TARGET_PATH);

    return captured_output;
}

