#define CHAZ_USE_SHORT_NAMES

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/OperSys.h"

static void
S_probe_devnull(OperSys *self);

static void
S_destroy(OperSys *self);

static void
S_remove_exe(OperSys *self, char *name);

static void
S_remove_obj(OperSys *self, char *name);

static int 
S_run_local(OperSys *self, ...);

OperSys*
OS_new(const char *name) 
{
    OperSys *self = (OperSys*)malloc(sizeof(OperSys));

    if (Util_verbosity)
        printf("Creating os object...\n");

    /* assign */
    self->name = strdup(name);

    /* init */
    self->buf        = NULL;
    self->buf_len    = 0;
    self->remove_obj = S_remove_obj;
    self->remove_exe = S_remove_exe;
    self->run_local  = S_run_local;
    self->destroy    = S_destroy;

    /* derive */
    if (strcmp(name, "mswin32") == 0) {
        self->obj_ext = strdup(".obj");
        self->exe_ext = strdup(".exe");
        self->local_command_start = strdup(".\\");
        self->devnull = strdup("nul");
    }
    else {
        self->obj_ext = strdup("");
        self->exe_ext = strdup("");
        self->local_command_start = strdup("./");
        S_probe_devnull(self);
    }

    return self;
}

static void
S_probe_devnull(OperSys *self)
{
    char *const devnull_options[] = {
        "/dev/null", 
        "/dev/nul", 
        NULL
    };
    int i;

    if (Util_verbosity)
        printf("Trying to find a bit-bucket a la /dev/null...\n");

    /* iterate through names of possible devnulls trying to open them */
    for (i = 0; devnull_options[i] != NULL; i++) {
        if (Util_can_open_file(devnull_options[i])) {
            self->devnull = strdup(devnull_options[i]);
            return;
        }
    }

    /* bail out we couldn't find a devnull */
    Util_die("Couldn't find anything like /dev/null");
}

static void
S_destroy(OperSys *self)
{
    free(self->buf);
    free(self->name);
    free(self->obj_ext);
    free(self->exe_ext);
    free(self->local_command_start);
    free(self->devnull);
    free(self);
}

static void
S_remove_exe(OperSys *self, char *name)
{
    char *exe_name = (char*)malloc(strlen(name) + strlen(self->exe_ext) + 1);
    sprintf(exe_name, "%s%s", name, self->exe_ext);
    remove(exe_name);
    free(exe_name);
}

static void
S_remove_obj(OperSys *self, char *name)
{
    char *obj_name = (char*)malloc(strlen(name) + strlen(self->obj_ext) + 1);
    sprintf(obj_name, "%s%s", name, self->obj_ext);
    remove(obj_name);
    free(obj_name);
}

static int
S_run_local(OperSys *self, ...)
{
    va_list  args;
    char    *command = strdup(self->local_command_start);
    size_t   len     = strlen(command);
    int      retval;
    char    *arg;

    /* append all supplied texts */
    va_start(args, self);
    while (NULL != (arg = va_arg(args, char*))) {
        len += strlen(arg);
        command = (char*)realloc(command, len + 1);
        strcat(command, arg);
    }
    va_end(args);

    /* run the command */
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

