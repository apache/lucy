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
#include "Charmonizer/Core/Dir.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/OperatingSystem.h"

/* Write the "_charm.h" file used by every probe.
 */
static void
S_write_charm_h();

void
Probe_init(const char *cc_command, const char *cc_flags,
           const char *charmony_start) {
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
Probe_clean_up() {
    if (Util_verbosity) { printf("Cleaning up...\n"); }

    /* Dispatch various clean up routines. */
    ConfWriter_clean_up();
    CC_clean_up();
    OS_clean_up();
    Dir_clean_up();

    if (Util_verbosity) { printf("Cleanup complete.\n"); }
}

void
Probe_set_verbosity(int level) {
    Util_verbosity = level;
}

FILE*
Probe_get_charmony_fh(void) {
    return ConfWriter_get_charmony_fh();
}

static char charm_h_code[] =
    QUOTE(  #ifndef CHARM_H                                                  )
    QUOTE(  #define CHARM_H 1                                                )
    QUOTE(  #include <stdio.h>                                               )
    QUOTE(  #define Charm_Setup freopen("_charmonizer_target", "w", stdout)  )
    QUOTE(  #endif                                                           );

static void
S_write_charm_h() {
    Util_write_file("_charm.h", charm_h_code);
}


