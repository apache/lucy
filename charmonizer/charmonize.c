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

/* Charmonize.c -- Create Charmony.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Charmonizer/Probe.h"
#include "Charmonizer/Probe/AtomicOps.h"
#include "Charmonizer/Probe/DirManip.h"
#include "Charmonizer/Probe/Floats.h"
#include "Charmonizer/Probe/FuncMacro.h"
#include "Charmonizer/Probe/Headers.h"
#include "Charmonizer/Probe/Integers.h"
#include "Charmonizer/Probe/LargeFiles.h"
#include "Charmonizer/Probe/Memory.h"
#include "Charmonizer/Probe/UnusedVars.h"
#include "Charmonizer/Probe/VariadicMacros.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"

int main(int argc, char **argv) {
    /* Parse and process arguments. */
    if (argc != 3) {
        fprintf(stderr,
                "Usage: ./charmonize CC_COMMAND CC_FLAGS\n");
        exit(1);
    }
    else {
        char *cc_command = argv[1];
        char *cc_flags   = argv[2];
        chaz_Probe_init(cc_command, cc_flags, NULL);
    }

    /* Run probe modules. */
    chaz_DirManip_run();
    chaz_Headers_run();
    chaz_AtomicOps_run();
    chaz_FuncMacro_run();
    chaz_Integers_run();
    chaz_Floats_run();
    chaz_LargeFiles_run();
    chaz_Memory_run();
    chaz_UnusedVars_run();
    chaz_VariadicMacros_run();

    /* Write custom postamble. */
    chaz_ConfWriter_append_conf(
        "#ifdef CHY_HAS_SYS_TYPES_H\n"
        "  #include <sys/types.h>\n"
        "#endif\n\n"
    );
    chaz_ConfWriter_append_conf(
        "#ifdef CHY_HAS_ALLOCA_H\n"
        "  #include <alloca.h>\n"
        "#elif defined(CHY_HAS_MALLOC_H)\n"
        "  #include <malloc.h>\n"
        "#elif defined(CHY_ALLOCA_IN_STDLIB_H)\n"
        "  #include <stdlib.h>\n"
        "#endif\n\n"
    );
    chaz_ConfWriter_append_conf(
        "#ifdef CHY_HAS_WINDOWS_H\n"
        "  /* Target Windows XP. */\n"
        "  #ifndef WINVER\n"
        "    #define WINVER 0x0500\n"
        "  #endif\n"
        "  #ifndef _WIN32_WINNT\n"
        "    #define _WIN32_WINNT 0x0500\n"
        "  #endif\n"
        "#endif\n\n"
    );

    /* Clean up. */
    chaz_Probe_clean_up();

    return 0;
}


