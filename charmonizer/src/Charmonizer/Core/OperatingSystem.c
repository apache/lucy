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
static const char *exe_ext = ".exe";
static const char *obj_ext = ".obj";
static const char *local_command_start = ".\\";
#else
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

void
OS_clean_up(void) {
    OS_remove_exe("_charm_run");
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

static const char charm_run_code[] =
    QUOTE(  #include <stdio.h>                                           )
    QUOTE(  #include <stdlib.h>                                          )
    QUOTE(  #include <string.h>                                          )
    QUOTE(  #include <stddef.h>                                          )
    QUOTE(  int main(int argc, char **argv)                              )
    QUOTE(  {                                                            )
    QUOTE(      char *command;                                           )
    QUOTE(      size_t command_len = 1; /* Terminating null. */          )
    QUOTE(      int i;                                                   )
    QUOTE(      int retval;                                              )
    /* Rebuild command line args. */
    QUOTE(      for (i = 1; i < argc; i++) {                             )
    QUOTE(          command_len += strlen(argv[i]) + 1;                  )
    QUOTE(      }                                                        )
    QUOTE(      command = (char*)calloc(command_len, sizeof(char));      )
    QUOTE(      if (command == NULL) {                                   )
    QUOTE(          fprintf(stderr, "calloc failed\n");                  )
    QUOTE(          exit(1);                                             )
    QUOTE(      }                                                        )
    QUOTE(      for (i = 1; i < argc; i++) {                             )
    QUOTE(          strcat( strcat(command, " "), argv[i] );             )
    QUOTE(      }                                                        )
    /* Redirect all output to /dev/null or equivalent. */
    QUOTE(      freopen("%s", "w", stdout);                              )
    QUOTE(      freopen("%s", "w", stderr);                              )
    /* Run commmand and return its value to parent. */
    QUOTE(      retval = system(command);                                )
    QUOTE(      free(command);                                           )
    QUOTE(      return retval;                                           )
    QUOTE(  }                                                            );

static void
S_build_charm_run(void) {
    chaz_bool_t compile_succeeded = false;
    size_t needed = sizeof(charm_run_code)
                    + strlen(dev_null)
                    + strlen(dev_null)
                    + 20;
    char *code = (char*)malloc(needed);

    sprintf(code, charm_run_code, dev_null, dev_null);
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
OS_remove_exe(const char *name) {
    char *exe_name = (char*)malloc(strlen(name) + strlen(exe_ext) + 1);
    sprintf(exe_name, "%s%s", name, exe_ext);
    remove(exe_name);
    free(exe_name);
}

void
OS_remove_obj(const char *name) {
    char *obj_name = (char*)malloc(strlen(name) + strlen(obj_ext) + 1);
    sprintf(obj_name, "%s%s", name, obj_ext);
    remove(obj_name);
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
    retval = system(quiet_command);
    free(quiet_command);
#else
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
#endif

    return retval;
}


