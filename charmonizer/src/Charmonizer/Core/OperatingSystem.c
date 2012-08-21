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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

static char dev_null[20] = "";

#ifdef _WIN32
#define SHELL_IS_CMD_EXE
static const char *exe_ext = ".exe";
static const char *obj_ext = ".obj";
static const char *local_command_start = ".\\";
#else
#define SHELL_IS_POSIX
static const char *exe_ext = "";
static const char *obj_ext = "";
static const char *local_command_start = "./";
#endif

static void
S_probe_dev_null(void);

/* Compile a small wrapper application which is used to redirect error output
 * to dev_null.
 */
static void
S_build_charm_run(void);

static chaz_bool_t charm_run_initialized = false;
static chaz_bool_t charm_run_ok = false;

void
OS_init(void) {
    if (Util_verbosity) {
        printf("Initializing Charmonizer/Core/OperatingSystem...\n");
    }

    S_probe_dev_null();
}

static void
S_probe_dev_null(void) {
    if (Util_verbosity) {
        printf("Trying to find a bit-bucket a la /dev/null...\n");
    }

#ifdef _WIN32
    strcpy(dev_null, "nul");
#else
    {
        const char *const options[] = {
            "/dev/null",
            "/dev/nul",
            NULL
        };
        int i;

        /* Iterate through names of possible devnulls trying to open them. */
        for (i = 0; options[i] != NULL; i++) {
            if (Util_can_open_file(options[i])) {
                strcpy(dev_null, options[i]);
                return;
            }
        }

        /* Bail out because we couldn't find anything like /dev/null. */
        Util_die("Couldn't find anything like /dev/null");
    }
#endif
}

const char*
OS_exe_ext(void) {
    return exe_ext;
}

const char*
OS_obj_ext(void) {
    return obj_ext;
}

const char*
OS_dev_null(void) {
    return dev_null;
}

int
OS_remove(const char *name) {
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

void
OS_remove_exe(const char *name) {
    char *exe_name = (char*)malloc(strlen(name) + strlen(exe_ext) + 1);
    sprintf(exe_name, "%s%s", name, exe_ext);
    OS_remove(exe_name);
    free(exe_name);
}

void
OS_remove_obj(const char *name) {
    char *obj_name = (char*)malloc(strlen(name) + strlen(obj_ext) + 1);
    sprintf(obj_name, "%s%s", name, obj_ext);
    OS_remove(obj_name);
    free(obj_name);
}

int
OS_run_local(const char *arg1, ...) {
    va_list  args;
    size_t   len     = strlen(local_command_start) + strlen(arg1);
    char    *command = (char*)malloc(len + 1);
    int      retval;
    char    *arg;

    /* Append all supplied texts. */
    sprintf(command, "%s%s", local_command_start, arg1);
    va_start(args, arg1);
    while (NULL != (arg = va_arg(args, char*))) {
        len += strlen(arg);
        command = (char*)realloc(command, len + 1);
        strcat(command, arg);
    }
    va_end(args);

    /* Run the command. */
    retval = system(command);
    free(command);
    return retval;
}

int
OS_run_quietly(const char *command) {
    int retval = 1;
#ifdef _WIN32
    char pattern[] = "%s > NUL 2> NUL";
    size_t size = sizeof(pattern) + strlen(command) + 10;
    char *quiet_command = (char*)malloc(size);
    sprintf(quiet_command, pattern, command);
#else
    char pattern[] = "%s > %s 2>&1";
    size_t size = sizeof(pattern) + strlen(command) + strlen(dev_null) + 10;
    char *quiet_command = (char*)malloc(size);
    sprintf(quiet_command, pattern, command, dev_null);
#endif
    retval = system(quiet_command);
    free(quiet_command);

    return retval;
}

void
OS_mkdir(const char *filepath) {
    #if (defined(SHELL_IS_POSIX) || defined (SHELL_IS_CMD_EXE))
    char *mkdir_command = "mkdir";
    #endif
    unsigned size = strlen(mkdir_command) + 1 + strlen(filepath) + 1;
    char *command = (char*)malloc(size);
    sprintf(command, "%s %s", mkdir_command, filepath);
    OS_run_quietly(command);
    free(command);
}

void
OS_rmdir(const char *filepath) {
    #ifdef SHELL_IS_POSIX
    char *rmdir_command = "rmdir";
    #elif defined(SHELL_IS_CMD_EXE)
    char *rmdir_command = "rmdir /q";
    #endif
    unsigned size = strlen(rmdir_command) + 1 + strlen(filepath) + 1;
    char *command = (char*)malloc(size);
    sprintf(command, "%s %s", rmdir_command, filepath);
    OS_run_quietly(command);
    free(command);
}

