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

/* Compile a small wrapper application which is used to redirect error output
 * to dev_null.
 */
static void
S_build_charm_run();

static chaz_bool_t charm_run_initialized = false;
static chaz_bool_t charm_run_ok = false;

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
    OS_remove_exe("_charm_run");
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

static char charm_run_code_a[] = METAQUOTE
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stddef.h>
    int main(int argc, char **argv)
    {
        char *command;
        size_t command_len = 1; /* Terminating null. */
        int i;
        int retval;
        
        /* Rebuild the command line args, minus the name of this utility. */
        for (i = 1; i < argc; i++) {
            command_len += strlen(argv[i]) + 1;
        }
        command = (char*)calloc(command_len, sizeof(char));
METAQUOTE;

static char charm_run_code_b[] = METAQUOTE
        if (command == NULL) {
            fprintf(stderr, "calloc failed\n");
            exit(1);
        }
        for (i = 1; i < argc; i++) {
            strcat( strcat(command, " "), argv[i] );
        }

        /* Redirect stdout and stderr to /dev/null or equivalent. */
        freopen( 
METAQUOTE;

static char charm_run_code_c[] = METAQUOTE
             , "w", stdout);
        freopen( 
METAQUOTE;

static char charm_run_code_d[] = METAQUOTE
             , "w", stderr);

        /* Run the commmand and return its value to the parent process. */
        retval = system(command);
        free(command);
        return retval;
    }
METAQUOTE;

static void
S_build_charm_run()
{
    chaz_bool_t compile_succeeded = false;
    const char *dev_null = OS_dev_null();
    size_t needed = sizeof(charm_run_code_a)
                  + sizeof(charm_run_code_b)
                  + strlen(dev_null)
                  + sizeof(charm_run_code_c)
                  + strlen(dev_null)
                  + sizeof(charm_run_code_d)
                  + 20;
    char *code = (char*)malloc(needed);

    sprintf(code, "%s%s \"%s\" %s \"%s\" %s", 
        charm_run_code_a, 
        charm_run_code_b,
        dev_null,
        charm_run_code_c,
        dev_null,
        charm_run_code_d);
    compile_succeeded = CC_compile_exe("_charm_run.c", "_charm_run", 
        code, strlen(code));
    if (!compile_succeeded) {
        Util_die("failed to compile _charm_run helper utility");
    }

    remove("_charm_run.c");
    free(code);
    charm_run_ok = true;
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

int
OS_run_quietly(const char *command)
{
    int retval = 1;
    
    if (!charm_run_initialized) {
        charm_run_initialized = true;
        S_build_charm_run();
    }

    if (!charm_run_ok) {
        retval = system(command);
    }
    else {
        retval = OS_run_local("_charm_run ", command, NULL);
    }

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

