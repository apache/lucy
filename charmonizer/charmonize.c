/* charmonize.c -- Create Charmony.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

char *cc_command, *cc_flags, *os_name, *verbosity_str;

/* Process command line args, set up Charmonizer, etc. 
 */
void
init(int argc, char **argv);

/* Find <tag_name> and </tag_name> within a string and return the text between
 * them as a newly allocated substring.
 */
static char*
S_extract_delim(char *source, size_t source_len, const char *tag_name);

/* Version of extract delim which dies rather than returns NULL upon failure.
 */
static char*
S_extract_delim_and_verify(char *source, size_t source_len, 
                           const char *tag_name);

/* Write some stuff to the end of charmony.h
 */
static void
S_write_charmony_postamble(void);

/* Print a message to stderr and exit.
 */
void
die(char *format, ...);

int main(int argc, char **argv) 
{
    init(argc, argv);

    /* modules section */
    chaz_DirManip_run();
    chaz_Headers_run();
    chaz_FuncMacro_run();
    chaz_Integers_run();
    chaz_Floats_run();
    chaz_LargeFiles_run();
    chaz_UnusedVars_run();
    chaz_VariadicMacros_run();

    /* write custom postamble */
    S_write_charmony_postamble();

    /* clean up */
    chaz_Probe_clean_up();
    free(cc_command);
    free(cc_flags);
    free(os_name);
    free(verbosity_str);

    return 0;
}

void
init(int argc, char **argv) 
{
    char *infile_str;
    size_t infile_len;

    /* parse the infile */
    if (argc != 2)
        die("Usage: ./charmonize INFILE");
    infile_str = chaz_Probe_slurp_file(argv[1], &infile_len);
    cc_command = S_extract_delim_and_verify(infile_str, infile_len, 
        "charm_cc_command");
    cc_flags = S_extract_delim_and_verify(infile_str, infile_len, 
        "charm_cc_flags");
    os_name = S_extract_delim_and_verify(infile_str, infile_len, 
        "charm_os_name");
    verbosity_str = S_extract_delim(infile_str, infile_len, "charm_verbosity");

    /* set up Charmonizer */
    if (verbosity_str != NULL) {
        const long verbosity = strtol(verbosity_str, NULL, 10);
        chaz_Probe_set_verbosity(verbosity);
    }
    chaz_Probe_init(os_name, cc_command, cc_flags, NULL);

    /* clean up */
    free(infile_str);
}

static char*
S_extract_delim(char *source, size_t source_len, const char *tag_name)
{
    const size_t tag_name_len = strlen(tag_name);
    const size_t opening_delim_len = tag_name_len + 2;
    const size_t closing_delim_len = tag_name_len + 3;
    char opening_delim[100];
    char closing_delim[100];
    const char *limit = source + source_len - closing_delim_len;
    char *start, *end;
    char *retval = NULL;

    /* sanity check, then create delimiter strings to match against */
    if (tag_name_len > 95)
        die("tag name too long: '%s'");
    sprintf(opening_delim, "<%s>", tag_name);
    sprintf(closing_delim, "</%s>", tag_name);
    
    /* find opening <delimiter> */
    for (start = source; start < limit; start++) {
        if (strncmp(start, opening_delim, opening_delim_len) == 0) {
            start += opening_delim_len;
            break;
        }
    }

    /* find closing </delimiter> */
    for (end = start; end < limit; end++) {
        if (strncmp(end, closing_delim, closing_delim_len) == 0) {
            const size_t retval_len = end - start;
            retval = (char*)malloc((retval_len + 1) * sizeof(char));
            retval[retval_len] = '\0';
            strncpy(retval, start, retval_len);
            break;
        }
    }
    
    return retval;
}

static char*
S_extract_delim_and_verify(char *source, size_t source_len, 
                           const char *tag_name)
{
    char *retval = S_extract_delim(source, source_len, tag_name);
    if (retval == NULL)
        die("Couldn't extract value for '%s'", tag_name);
    return retval;
}

static void
S_write_charmony_postamble(void)
{
    if (chaz_HeadCheck_check_header("sys/mman.h")) {
        chaz_ModHand_append_conf("#define CHY_HAS_SYS_MMAN_H\n\n");
    }
}

void 
die(char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
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

