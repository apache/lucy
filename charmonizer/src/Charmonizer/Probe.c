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
#include "Charmonizer/Core/ConfWriterC.h"
#include "Charmonizer/Core/ConfWriterPerl.h"
#include "Charmonizer/Core/ConfWriterRuby.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/OperatingSystem.h"

int
chaz_Probe_parse_cli_args(int argc, const char *argv[],
                          struct chaz_CLIArgs *args) {
    int i;
    int output_enabled = 0;

    /* Zero out args struct. */
    memset(args, 0, sizeof(struct chaz_CLIArgs));

    /* Parse most args. */
    for (i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--") == 0) {
            /* From here on out, everything will be a compiler flag. */
            i++;
            break;
        }
        if (strcmp(arg, "--enable-c") == 0) {
            args->charmony_h = 1;
            output_enabled = 1;
        }
        else if (strcmp(arg, "--enable-perl") == 0) {
            args->charmony_pm = 1;
            output_enabled = 1;
        }
        else if (strcmp(arg, "--enable-ruby") == 0) {
            args->charmony_rb = 1;
            output_enabled = 1;
        }
        else if (memcmp(arg, "--cc=", 5) == 0) {
            if (strlen(arg) > CHAZ_PROBE_MAX_CC_LEN - 5) {
                fprintf(stderr, "Exceeded max length for compiler command");
                exit(1);
            }
            strcpy(args->cc, arg + 5);
        }
    } /* preserve value of i */

    /* Accumulate compiler flags. */
    for (; i < argc; i++) {
        const char *arg = argv[i];
        size_t new_len = strlen(arg) + strlen(args->ccflags) + 2;
        if (new_len >= CHAZ_PROBE_MAX_FLAGS_LEN) {
            fprintf(stderr, "Exceeded max length for compiler flags");
            exit(1);
        }
        strcat(args->ccflags, " ");
        strcat(args->ccflags, arg);
    }

    /* Process CHARM_VERBOSITY environment variable. */
    {
        const char *verbosity_env = getenv("CHARM_VERBOSITY");
        if (verbosity_env && strlen(verbosity_env)) {
            args->verbosity = strtol(verbosity_env, NULL, 10);
        }
    }

    /* Validate. */
    if (!strlen(args->cc) || !output_enabled) {
        return false;
    }

    return true;
}

void
chaz_Probe_die_usage(void) {
    fprintf(stderr,
            "Usage: ./charmonize --cc=CC_COMMAND [--enable-c] "
            "[--enable-perl] [--enable-ruby] -- CC_FLAGS\n");
    exit(1);
}

void
chaz_Probe_init(struct chaz_CLIArgs *args) {
    int output_enabled = 0;

    {
        /* Process CHARM_VERBOSITY environment variable. */
        const char *verbosity_env = getenv("CHARM_VERBOSITY");
        if (verbosity_env && strlen(verbosity_env)) {
            chaz_Util_verbosity = strtol(verbosity_env, NULL, 10);
        }
    }

    /* Dispatch other initializers. */
    chaz_OS_init();
    chaz_CC_init(args->cc, args->ccflags);
    chaz_ConfWriter_init();
    chaz_HeadCheck_init();

    /* Enable output. */
    if (args->charmony_h) {
        chaz_ConfWriterC_enable();
        output_enabled = true;
    }
    if (args->charmony_pm) {
        chaz_ConfWriterPerl_enable();
        output_enabled = true;
    }
    if (args->charmony_rb) {
        chaz_ConfWriterRuby_enable();
        output_enabled = true;
    }
    if (!output_enabled) {
        fprintf(stderr, "No output formats enabled\n");
        exit(1);
    }

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

int
chaz_Probe_gcc_version_num(void) {
    return chaz_CC_gcc_version_num();
}

const char*
chaz_Probe_gcc_version(void) {
    return chaz_CC_gcc_version_num() ? chaz_CC_gcc_version() : NULL;
}

int
chaz_Probe_compiler_is_msvc(void) {
    return chaz_CC_compiler_is_msvc();
}
