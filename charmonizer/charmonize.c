/* Charmonize.c -- Create Charmony.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Charmonizer/Probe.h"
#include "Charmonizer/Probe/DirManip.h"
#include "Charmonizer/Probe/Floats.h"
#include "Charmonizer/Probe/FuncMacro.h"
#include "Charmonizer/Probe/Headers.h"
#include "Charmonizer/Probe/Integers.h"
#include "Charmonizer/Probe/LargeFiles.h"
#include "Charmonizer/Probe/UnusedVars.h"
#include "Charmonizer/Probe/VariadicMacros.h"
#include "Charmonizer/Core/HeadCheck.h"
#include "Charmonizer/Core/ModHandler.h"

int main(int argc, char **argv) 
{
    /* Parse and process arguments. */
    if (argc < 4) {
        fprintf(stderr, 
            "Usage: ./charmonize CC_COMMAND CC_FLAGS OS_NAME [VERBOSITY]\n");
        exit(1);
    }
    else {
        char *cc_command = argv[1];
        char *cc_flags   = argv[2];
        char *os_name    = argv[3];
        if (argc > 4) {
            const long verbosity = strtol(argv[4], NULL, 10);
            chaz_Probe_set_verbosity(verbosity);
        }
        chaz_Probe_init(os_name, cc_command, cc_flags, NULL);
    }

    /* Run probe modules. */
    chaz_DirManip_run();
    chaz_Headers_run();
    chaz_FuncMacro_run();
    chaz_Integers_run();
    chaz_Floats_run();
    chaz_LargeFiles_run();
    chaz_UnusedVars_run();
    chaz_VariadicMacros_run();

    /* Write custom postamble. */
    if (chaz_HeadCheck_check_header("sys/mman.h")) {
        chaz_ModHand_append_conf("#define CHY_HAS_SYS_MMAN_H\n\n");
    }

    /* Clean up. */
    chaz_Probe_clean_up();

    return 0;
}

/**
 * Copyright 2006-2009 The Apache Software Foundation
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

