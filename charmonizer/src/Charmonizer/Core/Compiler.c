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
    char     *cflags;
    char     *try_exe_name;
    char      obj_ext[10];
    char      gcc_version_str[30];
    int       cflags_style;
    int       intval___GNUC__;
    int       intval___GNUC_MINOR__;
    int       intval___GNUC_PATCHLEVEL__;
    int       intval__MSC_VER;
    int       intval___clang__;
    chaz_CFlags *extra_cflags;
    chaz_CFlags *temp_cflags;
} chaz_CC = {
    NULL, NULL, NULL,
    "", "",
    0, 0, 0, 0, 0, 0,
    NULL, NULL
};

void
chaz_CC_init(const char *compiler_command, const char *compiler_flags) {
    const char *code = "int main() { return 0; }\n";
    int compile_succeeded = 0;

    if (chaz_Util_verbosity) { printf("Creating compiler object...\n"); }

    /* Assign, init. */
    chaz_CC.cc_command   = chaz_Util_strdup(compiler_command);
    chaz_CC.cflags       = chaz_Util_strdup(compiler_flags);
    chaz_CC.extra_cflags = NULL;
    chaz_CC.temp_cflags  = NULL;

    /* Set names for the targets which we "try" to compile. */
    chaz_CC.try_exe_name
        = chaz_Util_join("", CHAZ_CC_TRY_BASENAME, chaz_OS_exe_ext(), NULL);

    /* If we can't compile anything, game over. */
    if (chaz_Util_verbosity) {
        printf("Trying to compile a small test file...\n");
    }
    /* Try MSVC argument style. */
    strcpy(chaz_CC.obj_ext, ".obj");
    chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_MSVC;
    compile_succeeded = chaz_CC_test_compile(code);
    if (!compile_succeeded) {
        /* Try POSIX argument style. */
        strcpy(chaz_CC.obj_ext, ".o");
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_POSIX;
        compile_succeeded = chaz_CC_test_compile(code);
    }
    if (!compile_succeeded) {
        chaz_Util_die("Failed to compile a small test file");
    }

    chaz_CC_detect_known_compilers();

    if (chaz_CC.intval___GNUC__) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_GNU;
    }
    else if (chaz_CC.intval__MSC_VER) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_MSVC;
    }
    else {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_POSIX;
    }
    chaz_CC.extra_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    chaz_CC.temp_cflags  = chaz_CFlags_new(chaz_CC.cflags_style);
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
    if (chaz_CC.intval___GNUC__) {
        chaz_CC.intval___GNUC_MINOR__
            = chaz_CC_detect_macro("__GNUC_MINOR__");
        chaz_CC.intval___GNUC_PATCHLEVEL__
            = chaz_CC_detect_macro("__GNUC_PATCHLEVEL__");
        sprintf(chaz_CC.gcc_version_str, "%d.%d.%d", chaz_CC.intval___GNUC__,
                chaz_CC.intval___GNUC_MINOR__,
                chaz_CC.intval___GNUC_PATCHLEVEL__);
    }
    chaz_CC.intval__MSC_VER  = chaz_CC_detect_macro("_MSC_VER");
    chaz_CC.intval___clang__ = chaz_CC_detect_macro("__clang__");
}

void
chaz_CC_clean_up(void) {
    free(chaz_CC.cc_command);
    free(chaz_CC.cflags);
    free(chaz_CC.try_exe_name);
    chaz_CFlags_destroy(chaz_CC.extra_cflags);
    chaz_CFlags_destroy(chaz_CC.temp_cflags);
}

int
chaz_CC_compile_exe(const char *source_path, const char *exe_name,
                    const char *code) {
    chaz_CFlags *local_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    const char *extra_cflags_string = "";
    const char *temp_cflags_string  = "";
    const char *local_cflags_string;
    char *exe_file = chaz_Util_join("", exe_name, chaz_OS_exe_ext(), NULL);
    char *command;
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (chaz_CC.extra_cflags) {
        extra_cflags_string = chaz_CFlags_get_string(chaz_CC.extra_cflags);
    }
    if (chaz_CC.temp_cflags) {
        temp_cflags_string = chaz_CFlags_get_string(chaz_CC.temp_cflags);
    }
    chaz_CFlags_set_output_exe(local_cflags, exe_file);
    local_cflags_string = chaz_CFlags_get_string(local_cflags);
    command = chaz_Util_join(" ", chaz_CC.cc_command, chaz_CC.cflags,
                             source_path, extra_cflags_string,
                             temp_cflags_string, local_cflags_string, NULL);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        system(command);
    }

    if (chaz_CC.intval__MSC_VER) {
        /* Zap MSVC junk. */
        size_t  junk_buf_size = strlen(exe_file) + 4;
        char   *junk          = (char*)malloc(junk_buf_size);
        sprintf(junk, "%s.obj", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.ilk", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.pdb", exe_name);
        chaz_Util_remove_and_verify(junk);
        free(junk);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(exe_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    chaz_CFlags_destroy(local_cflags);
    free(command);
    free(exe_file);
    return result;
}

int
chaz_CC_compile_obj(const char *source_path, const char *obj_name,
                    const char *code) {
    chaz_CFlags *local_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    const char *extra_cflags_string = "";
    const char *temp_cflags_string  = "";
    const char *local_cflags_string;
    char *obj_file = chaz_Util_join("", obj_name, chaz_CC.obj_ext, NULL);
    char *command;
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (chaz_CC.extra_cflags) {
        extra_cflags_string = chaz_CFlags_get_string(chaz_CC.extra_cflags);
    }
    if (chaz_CC.temp_cflags) {
        temp_cflags_string = chaz_CFlags_get_string(chaz_CC.temp_cflags);
    }
    chaz_CFlags_set_output_obj(local_cflags, obj_file);
    local_cflags_string = chaz_CFlags_get_string(local_cflags);
    command = chaz_Util_join(" ", chaz_CC.cc_command, chaz_CC.cflags,
                             source_path, extra_cflags_string,
                             temp_cflags_string, local_cflags_string, NULL);
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

    chaz_CFlags_destroy(local_cflags);
    free(command);
    free(obj_file);
    return result;
}

int
chaz_CC_test_compile(const char *source) {
    int compile_succeeded;
    char *try_obj_name
        = chaz_Util_join("", CHAZ_CC_TRY_BASENAME, chaz_CC.obj_ext, NULL);
    if (!chaz_Util_remove_and_verify(try_obj_name)) {
        chaz_Util_die("Failed to delete file '%s'", try_obj_name);
    }
    compile_succeeded = chaz_CC_compile_obj(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    chaz_Util_remove_and_verify(try_obj_name);
    free(try_obj_name);
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

const char*
chaz_CC_get_cc(void) {
    return chaz_CC.cc_command;
}

const char*
chaz_CC_get_cflags(void) {
    return chaz_CC.cflags;
}

int
chaz_CC_get_cflags_style(void) {
    return chaz_CC.cflags_style;
}

chaz_CFlags*
chaz_CC_get_extra_cflags(void) {
    return chaz_CC.extra_cflags;
}

chaz_CFlags*
chaz_CC_get_temp_cflags(void) {
    return chaz_CC.temp_cflags;
}

const char*
chaz_CC_obj_ext(void) {
    return chaz_CC.obj_ext;
}

int
chaz_CC_gcc_version_num(void) {
    return 10000 * chaz_CC.intval___GNUC__
           + 100 * chaz_CC.intval___GNUC_MINOR__
           + chaz_CC.intval___GNUC_PATCHLEVEL__;
}

const char*
chaz_CC_gcc_version(void) {
    return chaz_CC.intval___GNUC__ ? chaz_CC.gcc_version_str : NULL;
}

int
chaz_CC_msvc_version_num(void) {
    return chaz_CC.intval__MSC_VER;
}

const char*
chaz_CC_link_command() {
    if (chaz_CC.intval__MSC_VER) {
        return "link";
    }
    else {
        return chaz_CC.cc_command;
    }
}

char*
chaz_CC_shared_lib_file(const char *name) {
    const char *prefix = "";
    const char *shlib_ext = chaz_OS_shared_lib_ext();
    if (!chaz_CC.intval__MSC_VER) {
        if (chaz_OS_is_cygwin()) {
            prefix = "cyg";
        }
        else {
            prefix = "lib";
        }
    }
    return chaz_Util_join("", prefix, name, shlib_ext, NULL);
}


