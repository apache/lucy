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
chaz_Probe_init(const char *cc_command, const char *cc_flags) {
    /* Proces CHARM_VERBOSITY environment variable. */
    const char *verbosity_env = getenv("CHARM_VERBOSITY");
    if (verbosity_env && strlen(verbosity_env)) {
        chaz_Util_verbosity = strtol(verbosity_env, NULL, 10);
    }

    /* Dispatch other initializers. */
    chaz_OS_init();
    chaz_CC_init(cc_command, cc_flags);
    chaz_ConfWriter_init();
    chaz_HeadCheck_init();

    if (chaz_Util_verbosity) { printf("Initialization complete.\n"); }
}

void
chaz_Probe_clean_up(void) {
    if (chaz_Util_verbosity) { printf("Cleaning up...\n"); }

    /* Dispatch various clean up routines. */
    chaz_ConfWriter_clean_up();
    chaz_CC_clean_up();

    if (chaz_Util_verbosity) { printf("Cleanup complete.\n"); }
}

