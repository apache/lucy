#define CHAZ_USE_SHORT_NAMES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Charmonizer/Probe.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/OperatingSystem.h"

void
Probe_init(const char *cc_command, const char *cc_flags, 
           const char *charmony_start)
{
    /* Create os and compiler objects. */
    ConfWriter_os       = OS_new();
    ConfWriter_compiler = CC_new(ConfWriter_os, cc_command, cc_flags);

    /* Dispatch other tasks. */
    ConfWriter_init();
    HeadCheck_init();
    ConfWriter_open_charmony_h(charmony_start);

    if (Util_verbosity) { printf("Initialization complete.\n"); }
}

void
Probe_clean_up()
{
    if (Util_verbosity) { printf("Cleaning up...\n"); }

    /* Dispatch ConfWriter's clean up routines, destroy objects. */
    ConfWriter_clean_up();
    ConfWriter_os->destroy(ConfWriter_os);
    ConfWriter_compiler->destroy(ConfWriter_compiler);

    if (Util_verbosity) { printf("Cleanup complete.\n"); }
}

void
Probe_set_verbosity(int level)
{
    Util_verbosity = level;
}

FILE*
Probe_get_charmony_fh(void)
{
    return ConfWriter_charmony_fh;
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

