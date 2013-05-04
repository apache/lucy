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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

#define CHAZ_OS_TARGET_PATH  "_charmonizer_target"
#define CHAZ_OS_NAME_MAX     31

static struct {
    char name[CHAZ_OS_NAME_MAX+1];
    char dev_null[20];
    char exe_ext[5];
    char shared_lib_ext[7];
    char local_command_start[3];
    int  shell_type;
} chaz_OS = { "", "", "", "", "", 0 };

void
chaz_OS_init(void) {
    if (chaz_Util_verbosity) {
        printf("Initializing Charmonizer/Core/OperatingSystem...\n");
    }

    if (chaz_Util_verbosity) {
        printf("Trying to find a bit-bucket a la /dev/null...\n");
    }

    /* Detect shell based on whether the bitbucket is "/dev/null" or "nul". */
    if (chaz_Util_can_open_file("/dev/null")) {
        char   *uname;
        size_t  uname_len;
        size_t i;

        chaz_OS.shell_type = CHAZ_OS_POSIX;

        /* Detect Unix name. */
        uname = chaz_OS_run_and_capture("uname", &uname_len);
        for (i = 0; i < CHAZ_OS_NAME_MAX && i < uname_len; i++) {
            char c = uname[i];
            if (!c || isspace(c)) { break; }
            chaz_OS.name[i] = tolower(c);
        }
        if (i > 0) { chaz_OS.name[i] = '\0'; }
        else       { strcpy(chaz_OS.name, "unknown_unix"); }
        free(uname);

        strcpy(chaz_OS.dev_null, "/dev/null");
        strcpy(chaz_OS.exe_ext, "");
        if (memcmp(chaz_OS.name, "darwin", 6) == 0) {
            strcpy(chaz_OS.shared_lib_ext, ".dylib");
        }
        else if (memcmp(chaz_OS.name, "cygwin", 6) == 0) {
            strcpy(chaz_OS.shared_lib_ext, ".dll");
        }
        else {
            strcpy(chaz_OS.shared_lib_ext, ".so");
        }
        strcpy(chaz_OS.local_command_start, "./");
    }
    else if (chaz_Util_can_open_file("nul")) {
        strcpy(chaz_OS.name, "windows");
        strcpy(chaz_OS.dev_null, "nul");
        strcpy(chaz_OS.exe_ext, ".exe");
        strcpy(chaz_OS.shared_lib_ext, ".dll");
        strcpy(chaz_OS.local_command_start, ".\\");
        chaz_OS.shell_type = CHAZ_OS_CMD_EXE;
    }
    else {
        /* Bail out because we couldn't find anything like /dev/null. */
        chaz_Util_die("Couldn't find anything like /dev/null");
    }
}

const char*
chaz_OS_name(void) {
    return chaz_OS.name;
}

int
chaz_OS_is_darwin(void) {
    return memcmp(chaz_OS.name, "darwin", 6) == 0;
}

int
chaz_OS_is_cygwin(void) {
    return memcmp(chaz_OS.name, "cygwin", 6) == 0;
}

const char*
chaz_OS_exe_ext(void) {
    return chaz_OS.exe_ext;
}

const char*
chaz_OS_shared_lib_ext(void) {
    return chaz_OS.shared_lib_ext;
}

const char*
chaz_OS_dev_null(void) {
    return chaz_OS.dev_null;
}

int
chaz_OS_shell_type(void) {
    return chaz_OS.shell_type;
}

int
chaz_OS_remove(const char *name) {
    /*
     * On Windows it can happen that another process, typically a
     * virus scanner, still has an open handle on the file. This can
     * make the subsequent recreation of a file with the same name
     * fail. As a workaround, files are renamed to a random name
     * before deletion.
     */
    int retval;

    static const size_t num_random_chars = 16;

    size_t  name_len = strlen(name);
    size_t  i;
    char   *temp_name = (char*)malloc(name_len + num_random_chars + 1);

    strcpy(temp_name, name);
    for (i = 0; i < num_random_chars; i++) {
        temp_name[name_len+i] = 'A' + rand() % 26;
    }
    temp_name[name_len+num_random_chars] = '\0';

    if (rename(name, temp_name) == 0) {
        retval = !remove(temp_name);
    }
    else {
        // Error during rename, remove using old name.
        retval = !remove(name);
    }

    free(temp_name);
    return retval;
}

int
chaz_OS_run_local_redirected(const char *command, const char *path) {
    char *local_command
        = chaz_Util_join("", chaz_OS.local_command_start, command, NULL);
    int retval = chaz_OS_run_redirected(local_command, path);
    free(local_command);
    return retval;
}

int
chaz_OS_run_quietly(const char *command) {
    return chaz_OS_run_redirected(command, chaz_OS.dev_null);
}

int
chaz_OS_run_redirected(const char *command, const char *path) {
    int retval = 1;
    char *quiet_command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
        ) {
        quiet_command = chaz_Util_join(" ", command, ">", path, "2>&1", NULL);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    retval = system(quiet_command);
    free(quiet_command);
    return retval;
}

char*
chaz_OS_run_and_capture(const char *command, size_t *output_len) {
    char *output;
    chaz_OS_run_redirected(command, CHAZ_OS_TARGET_PATH);
    output = chaz_Util_slurp_file(CHAZ_OS_TARGET_PATH, output_len);
    chaz_Util_remove_and_verify(CHAZ_OS_TARGET_PATH);
    return output;
}

void
chaz_OS_mkdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
       ) {
        command = chaz_Util_join(" ", "mkdir", filepath, NULL);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    chaz_OS_run_quietly(command);
    free(command);
}

void
chaz_OS_rmdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX) {
        command = chaz_Util_join(" ", "rmdir", filepath, NULL);
    }
    else if (chaz_OS.shell_type == CHAZ_OS_CMD_EXE) {
        command = chaz_Util_join(" ", "rmdir", "/q", filepath, NULL);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    chaz_OS_run_quietly(command);
    free(command);
}

