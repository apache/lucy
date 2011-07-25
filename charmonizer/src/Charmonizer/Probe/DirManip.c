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

#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/OperatingSystem.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Probe/DirManip.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int   mkdir_num_args  = 0;
static int   mkdir_available = 0;
static char  mkdir_command_buf[7];
static char *mkdir_command = mkdir_command_buf;
static int   rmdir_available = 0;

/* Source code for standard POSIX mkdir */
static const char posix_mkdir_code[] =
    QUOTE(  #include <%s>                                          )
    QUOTE(  int main(int argc, char **argv) {                      )
    QUOTE(      if (argc != 2) { return 1; }                       )
    QUOTE(      if (mkdir(argv[1], 0777) != 0) { return 2; }       )
    QUOTE(      return 0;                                          )
    QUOTE(  }                                                      );

/* Source code for Windows _mkdir. */
static const char win_mkdir_code[] =
    QUOTE(  #include <direct.h>                                    )
    QUOTE(  int main(int argc, char **argv) {                      )
    QUOTE(      if (argc != 2) { return 1; }                       )
    QUOTE(      if (_mkdir(argv[1]) != 0) { return 2; }            )
    QUOTE(      return 0;                                          )
    QUOTE(  }                                                      );

/* Source code for rmdir. */
static const char rmdir_code[] =
    QUOTE(  #include <%s>                                          )
    QUOTE(  int main(int argc, char **argv) {                      )
    QUOTE(      if (argc != 2) { return 1; }                       )
    QUOTE(      if (rmdir(argv[1]) != 0) { return 2; }             )
    QUOTE(      return 0;                                          )
    QUOTE(  }                                                      );

static chaz_bool_t
S_compile_posix_mkdir(const char *header) {
    size_t needed = sizeof(posix_mkdir_code) + 30;
    char *code_buf = (char*)malloc(needed);

    /* Attempt compilation. */
    sprintf(code_buf, posix_mkdir_code, header);
    mkdir_available = CC_test_compile(code_buf, strlen(code_buf));

    /* Set vars on success. */
    if (mkdir_available) {
        strcpy(mkdir_command, "mkdir");
        if (strcmp(header, "direct.h") == 0) {
            mkdir_num_args = 1;
        }
        else {
            mkdir_num_args = 2;
        }
    }

    free(code_buf);
    return mkdir_available;
}

static chaz_bool_t
S_compile_win_mkdir(void) {
    mkdir_available = CC_test_compile(win_mkdir_code, strlen(win_mkdir_code));
    if (mkdir_available) {
        strcpy(mkdir_command, "_mkdir");
        mkdir_num_args = 1;
    }
    return mkdir_available;
}

static void
S_try_mkdir(void) {
    if (HeadCheck_check_header("windows.h")) {
        if (S_compile_win_mkdir())               { return; }
        if (S_compile_posix_mkdir("direct.h"))   { return; }
    }
    if (S_compile_posix_mkdir("sys/stat.h")) { return; }
}

static chaz_bool_t
S_compile_rmdir(const char *header) {
    size_t needed = sizeof(posix_mkdir_code) + 30;
    char *code_buf = (char*)malloc(needed);
    sprintf(code_buf, rmdir_code, header);
    rmdir_available = CC_test_compile(code_buf, strlen(code_buf));
    free(code_buf);
    return rmdir_available;
}

static void
S_try_rmdir(void) {
    if (S_compile_rmdir("unistd.h"))   { return; }
    if (S_compile_rmdir("dirent.h"))   { return; }
    if (S_compile_rmdir("direct.h"))   { return; }
}

static const char cygwin_code[] =
    QUOTE(#ifndef __CYGWIN__            )
    QUOTE(  #error "Not Cygwin"         )
    QUOTE(#endif                        )
    QUOTE(int main() { return 0; }      );

void
DirManip_run(void) {
    FILE *f;
    char dir_sep[3];
    chaz_bool_t remove_zaps_dirs = false;
    chaz_bool_t has_dirent_h = HeadCheck_check_header("dirent.h");
    chaz_bool_t has_direct_h = HeadCheck_check_header("direct.h");
    chaz_bool_t has_dirent_d_namlen = false;
    chaz_bool_t has_dirent_d_type   = false;

    ConfWriter_start_module("DirManip");
    S_try_mkdir();
    S_try_rmdir();

    /* Header checks. */
    if (has_dirent_h) {
        ConfWriter_append_conf("#define CHY_HAS_DIRENT_H\n");
    }
    if (has_direct_h) {
        ConfWriter_append_conf("#define CHY_HAS_DIRECT_H\n");
    }

    /* Check for members in struct dirent. */
    if (has_dirent_h) {
        has_dirent_d_namlen = HeadCheck_contains_member(
                                  "struct dirent", "d_namlen",
                                  "#include <sys/types.h>\n#include <dirent.h>"
                              );
        if (has_dirent_d_namlen) {
            ConfWriter_append_conf("#define CHY_HAS_DIRENT_D_NAMLEN\n", dir_sep);
        }
        has_dirent_d_type = HeadCheck_contains_member(
                                "struct dirent", "d_type",
                                "#include <sys/types.h>\n#include <dirent.h>"
                            );
        if (has_dirent_d_type) {
            ConfWriter_append_conf("#define CHY_HAS_DIRENT_D_TYPE\n", dir_sep);
        }
    }

    if (mkdir_num_args == 2) {
        /* It's two args, but the command isn't "mkdir". */
        ConfWriter_append_conf("#define chy_makedir(_dir, _mode) %s(_dir, _mode)\n",
                               mkdir_command);
        ConfWriter_append_conf("#define CHY_MAKEDIR_MODE_IGNORED 0\n");
    }
    else if (mkdir_num_args == 1) {
        /* It's one arg... mode arg will be ignored. */
        ConfWriter_append_conf("#define chy_makedir(_dir, _mode) %s(_dir)\n",
                               mkdir_command);
        ConfWriter_append_conf("#define CHY_MAKEDIR_MODE_IGNORED 1\n");
    }

    if (CC_test_compile(cygwin_code, strlen(cygwin_code))) {
        strcpy(dir_sep, "/");
    }
    else if (HeadCheck_check_header("windows.h")) {
        strcpy(dir_sep, "\\\\");
    }
    else {
        strcpy(dir_sep, "/");
    }

    ConfWriter_append_conf("#define CHY_DIR_SEP \"%s\"\n", dir_sep);

    /* See whether remove works on directories. */
    OS_mkdir("_charm_test_remove_me");
    if (0 == remove("_charm_test_remove_me")) {
        remove_zaps_dirs = true;
        ConfWriter_append_conf("#define CHY_REMOVE_ZAPS_DIRS\n");
    }
    OS_rmdir("_charm_test_remove_me");

    /* Shorten. */
    ConfWriter_start_short_names();
    ConfWriter_shorten_macro("DIR_SEP");
    if (has_dirent_h)     { ConfWriter_shorten_macro("HAS_DIRENT_H"); }
    if (has_direct_h)     { ConfWriter_shorten_macro("HAS_DIRECT_H"); }
    if (has_dirent_d_namlen) { ConfWriter_shorten_macro("HAS_DIRENT_D_NAMLEN"); }
    if (has_dirent_d_type)   { ConfWriter_shorten_macro("HAS_DIRENT_D_TYPE"); }
    ConfWriter_shorten_function("makedir");
    ConfWriter_shorten_macro("MAKEDIR_MODE_IGNORED");
    if (remove_zaps_dirs) { ConfWriter_shorten_macro("REMOVE_ZAPS_DIRS"); }

    ConfWriter_end_short_names();

    ConfWriter_end_module();
}



