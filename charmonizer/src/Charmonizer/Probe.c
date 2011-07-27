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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Charmonizer/Probe.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/OperatingSystem.h"

/* Write the "_charm.h" file used by every probe.
 */
static void
S_write_charm_h(void);

static void
S_remove_charm_h(void);

void
Probe_init(const char *cc_command, const char *cc_flags,
           const char *charmony_start) {
    /* Proces CHARM_VERBOSITY environment variable. */
    const char *verbosity_env = getenv("CHARM_VERBOSITY");
    if (verbosity_env && strlen(verbosity_env)) {
        Util_verbosity = strtol(verbosity_env, NULL, 10);
    }

    /* Dispatch other initializers. */
    OS_init();
    CC_init(cc_command, cc_flags);
    ConfWriter_init();
    HeadCheck_init();
    ConfWriter_open_charmony_h(charmony_start);
    S_write_charm_h();

    if (Util_verbosity) { printf("Initialization complete.\n"); }
}

void
Probe_clean_up(void) {
    if (Util_verbosity) { printf("Cleaning up...\n"); }

    /* Dispatch various clean up routines. */
    S_remove_charm_h();
    ConfWriter_clean_up();
    CC_clean_up();

    if (Util_verbosity) { printf("Cleanup complete.\n"); }
}

FILE*
Probe_get_charmony_fh(void) {
    return ConfWriter_get_charmony_fh();
}

static const char charm_h_code[] =
    QUOTE(  #ifndef CHARM_H                                                  )
    QUOTE(  #define CHARM_H 1                                                )
    QUOTE(  #include <stdio.h>                                               )
    QUOTE(  #define Charm_Setup freopen("_charmonizer_target", "w", stdout)  )
    QUOTE(  #endif                                                           );

static void
S_write_charm_h(void) {
    Util_write_file("_charm.h", charm_h_code);
}

static void
S_remove_charm_h(void) {
    remove("_charm.h");
}

