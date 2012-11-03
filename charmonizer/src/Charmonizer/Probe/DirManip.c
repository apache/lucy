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

#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/OperatingSystem.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Probe/DirManip.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct {
    int  mkdir_num_args;
    char mkdir_command[7];
} chaz_DirManip = { 0, "" };

/* Source code for rmdir. */
static int
chaz_DirManip_compile_posix_mkdir(const char *header) {
    static const char posix_mkdir_code[] =
        CHAZ_QUOTE(  #include <%s>                                      )
        CHAZ_QUOTE(  int main(int argc, char **argv) {                  )
        CHAZ_QUOTE(      if (argc != 2) { return 1; }                   )
        CHAZ_QUOTE(      if (mkdir(argv[1], 0777) != 0) { return 2; }   )
        CHAZ_QUOTE(      return 0;                                      )
        CHAZ_QUOTE(  }                                                  );
    char code_buf[sizeof(posix_mkdir_code) + 30];
    int mkdir_available;
    if (strlen(header) > 25) {
        chaz_Util_die("Header name too long: '%s'", header);
    }

    /* Attempt compilation. */
    sprintf(code_buf, posix_mkdir_code, header);
    mkdir_available = chaz_CC_test_compile(code_buf);

    /* Set vars on success. */
    if (mkdir_available) {
        strcpy(chaz_DirManip.mkdir_command, "mkdir");
        if (strcmp(header, "direct.h") == 0) {
            chaz_DirManip.mkdir_num_args = 1;
        }
        else {
            chaz_DirManip.mkdir_num_args = 2;
        }
    }

    return mkdir_available;
}

static int
chaz_DirManip_compile_win_mkdir(void) {
    static const char win_mkdir_code[] =
        CHAZ_QUOTE(  #include <direct.h>                                )
        CHAZ_QUOTE(  int main(int argc, char **argv) {                  )
        CHAZ_QUOTE(      if (argc != 2) { return 1; }                   )
        CHAZ_QUOTE(      if (_mkdir(argv[1]) != 0) { return 2; }        )
        CHAZ_QUOTE(      return 0;                                      )
        CHAZ_QUOTE(  }                                                  );
    int mkdir_available;

    mkdir_available = chaz_CC_test_compile(win_mkdir_code);
    if (mkdir_available) {
        strcpy(chaz_DirManip.mkdir_command, "_mkdir");
        chaz_DirManip.mkdir_num_args = 1;
    }
    return mkdir_available;
}

static void
chaz_DirManip_try_mkdir(void) {
    if (chaz_HeadCheck_check_header("windows.h")) {
        if (chaz_DirManip_compile_win_mkdir())               { return; }
        if (chaz_DirManip_compile_posix_mkdir("direct.h"))   { return; }
    }
    if (chaz_DirManip_compile_posix_mkdir("sys/stat.h")) { return; }
}

static int
chaz_DirManip_compile_rmdir(const char *header) {
    static const char rmdir_code[] =
        CHAZ_QUOTE(  #include <%s>                                      )
        CHAZ_QUOTE(  int main(int argc, char **argv) {                  )
        CHAZ_QUOTE(      if (argc != 2) { return 1; }                   )
        CHAZ_QUOTE(      if (rmdir(argv[1]) != 0) { return 2; }         )
        CHAZ_QUOTE(      return 0;                                      )
        CHAZ_QUOTE(  }                                                  );
    char code_buf[sizeof(rmdir_code) + 30];
    int rmdir_available;
    if (strlen(header) > 25) {
        chaz_Util_die("Header name too long: '%s'", header);
    }
    sprintf(code_buf, rmdir_code, header);
    rmdir_available = chaz_CC_test_compile(code_buf);
    return rmdir_available;
}

static void
chaz_DirManip_try_rmdir(void) {
    if (chaz_DirManip_compile_rmdir("unistd.h"))   { return; }
    if (chaz_DirManip_compile_rmdir("dirent.h"))   { return; }
    if (chaz_DirManip_compile_rmdir("direct.h"))   { return; }
}

static int
chaz_DirManip_is_cygwin(void) {
    static int is_cygwin = -1;
    static const char cygwin_code[] =
        CHAZ_QUOTE(#ifndef __CYGWIN__            )
        CHAZ_QUOTE(  #error "Not Cygwin"         )
        CHAZ_QUOTE(#endif                        )
        CHAZ_QUOTE(int main() { return 0; }      );
    if (is_cygwin == -1) {
        is_cygwin = chaz_CC_test_compile(cygwin_code);
    }
    return is_cygwin;
}

void
chaz_DirManip_run(void) {
    char dir_sep[3];
    int remove_zaps_dirs = false;
    int has_dirent_h = chaz_HeadCheck_check_header("dirent.h");
    int has_direct_h = chaz_HeadCheck_check_header("direct.h");
    int has_dirent_d_namlen = false;
    int has_dirent_d_type   = false;

    chaz_ConfWriter_start_module("DirManip");
    chaz_DirManip_try_mkdir();
    chaz_DirManip_try_rmdir();

    /* Header checks. */
    if (has_dirent_h) {
        chaz_ConfWriter_add_def("HAS_DIRENT_H", NULL);
    }
    if (has_direct_h) {
        chaz_ConfWriter_add_def("HAS_DIRECT_H", NULL);
    }

    /* Check for members in struct dirent. */
    if (has_dirent_h) {
        has_dirent_d_namlen = chaz_HeadCheck_contains_member(
                                  "struct dirent", "d_namlen",
                                  "#include <sys/types.h>\n#include <dirent.h>"
                              );
        if (has_dirent_d_namlen) {
            chaz_ConfWriter_add_def("HAS_DIRENT_D_NAMLEN", NULL);
        }
        has_dirent_d_type = chaz_HeadCheck_contains_member(
                                "struct dirent", "d_type",
                                "#include <sys/types.h>\n#include <dirent.h>"
                            );
        if (has_dirent_d_type) {
            chaz_ConfWriter_add_def("HAS_DIRENT_D_TYPE", NULL);
        }
    }

    if (chaz_DirManip.mkdir_num_args == 2) {
        /* It's two args, but the command isn't "mkdir". */
        char scratch[50];
        if (strlen(chaz_DirManip.mkdir_command) > 30) {
            chaz_Util_die("Command too long: '%s'", chaz_DirManip.mkdir_command);
        }
        sprintf(scratch, "%s(_dir, _mode)", chaz_DirManip.mkdir_command);
        chaz_ConfWriter_add_def("makedir(_dir, _mode)", scratch);
        chaz_ConfWriter_add_def("MAKEDIR_MODE_IGNORED", "0");
    }
    else if (chaz_DirManip.mkdir_num_args == 1) {
        /* It's one arg... mode arg will be ignored. */
        char scratch[50];
        if (strlen(chaz_DirManip.mkdir_command) > 30) {
            chaz_Util_die("Command too long: '%s'", chaz_DirManip.mkdir_command);
        }
        sprintf(scratch, "%s(_dir)", chaz_DirManip.mkdir_command);
        chaz_ConfWriter_add_def("makedir(_dir, _mode)", scratch);
        chaz_ConfWriter_add_def("MAKEDIR_MODE_IGNORED", "1");
    }

    if (chaz_DirManip_is_cygwin()) {
        strcpy(dir_sep, "/");
    }
    else if (chaz_HeadCheck_check_header("windows.h")) {
        strcpy(dir_sep, "\\\\");
    }
    else {
        strcpy(dir_sep, "/");
    }

    {
        char scratch[5];
        sprintf(scratch, "\"%s\"", dir_sep);
        chaz_ConfWriter_add_def("DIR_SEP", scratch);
    }

    /* See whether remove works on directories. */
    chaz_OS_mkdir("_charm_test_remove_me");
    if (0 == remove("_charm_test_remove_me")) {
        remove_zaps_dirs = true;
        chaz_ConfWriter_add_def("REMOVE_ZAPS_DIRS", NULL);
    }
    chaz_OS_rmdir("_charm_test_remove_me");

    chaz_ConfWriter_end_module();
}



