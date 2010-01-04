#define CHAZ_USE_SHORT_NAMES

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/OperatingSystem.h"

static char dev_null[20] = "";

#ifdef _WIN32
static char *exe_ext = ".exe";
static char *obj_ext = ".obj";
static char *local_command_start = ".\\";
#else
static char *exe_ext = "";
static char *obj_ext = "";
static char *local_command_start = "./";
#endif

static void
S_probe_dev_null(void);

void
OS_init() 
{
    if (Util_verbosity) {
        printf("Initializing Charmonizer/Core/OperatingSystem...\n");
    }

    S_probe_dev_null();
}

static void
S_probe_dev_null(void)
{
    if (Util_verbosity) {
        printf("Trying to find a bit-bucket a la /dev/null...\n");
    }

#ifdef _WIN32
    strcpy(dev_null, "nul");
#else
    {
        char *const options[] = {
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

void
OS_clean_up(void)
{
    return;
}

const char*
OS_exe_ext(void)
{
    return exe_ext;
}

const char*
OS_obj_ext(void)
{
    return obj_ext;
}

const char*
OS_dev_null(void)
{
    return dev_null;
}

void
OS_remove_exe(char *name)
{
    char *exe_name = (char*)malloc(strlen(name) + strlen(exe_ext) + 1);
    sprintf(exe_name, "%s%s", name, exe_ext);
    remove(exe_name);
    free(exe_name);
}

void
OS_remove_obj(char *name)
{
    char *obj_name = (char*)malloc(strlen(name) + strlen(obj_ext) + 1);
    sprintf(obj_name, "%s%s", name, obj_ext);
    remove(obj_name);
    free(obj_name);
}

int
OS_run_local(char *arg1, ...)
{
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

