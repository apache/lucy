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

#ifndef H_CHAZ
#define H_CHAZ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>

#define CHAZ_PROBE_MAX_CC_LEN 100
#define CHAZ_PROBE_MAX_CFLAGS_LEN 2000

struct chaz_CLIArgs {
    char cc[CHAZ_PROBE_MAX_CC_LEN + 1];
    char cflags[CHAZ_PROBE_MAX_CFLAGS_LEN + 1];
    int  charmony_h;
    int  charmony_pm;
    int  charmony_rb;
    int  verbosity;
};

/* Parse command line arguments, initializing and filling in the supplied
 * `args` struct.
 *
 *     APP_NAME --cc=CC_COMMAND
 *              [--enable-c]
 *              [--enable-perl]
 *              [--enable-ruby]
 *              [-- [CFLAGS]]
 *
 * @return true if argument parsing proceeds without incident, false if
 * unexpected arguments are encountered or values are missing or invalid.
 */
int
chaz_Probe_parse_cli_args(int argc, const char *argv[],
                          struct chaz_CLIArgs *args);

/* Exit after printing usage instructions to stderr.
 */
void
chaz_Probe_die_usage(void);

/* Set up the Charmonizer environment.
 *
 * If the environment variable CHARM_VERBOSITY has been set, it will be
 * processed at this time:
 *
 *      0 - silent
 *      1 - normal
 *      2 - debugging
 */
void
chaz_Probe_init(struct chaz_CLIArgs *args);

/* Clean up the Charmonizer environment -- deleting tempfiles, etc.  This
 * should be called only after everything else finishes.
 */
void
chaz_Probe_clean_up(void);

/* Return an integer version of the GCC version number which is
 * (10000 * __GNU_C__ + 100 * __GNUC_MINOR__ + __GNUC_PATCHLEVEL__).
 */
int
chaz_Probe_gcc_version_num(void);

/* If the compiler is GCC (or claims compatibility), return an X.Y.Z string
 * version of the GCC version; otherwise, return NULL.
 */
const char*
chaz_Probe_gcc_version(void);

/* Returns true if the compiler is MSVC.
 */
int
chaz_Probe_compiler_is_msvc(void);

#ifdef __cplusplus
}
#endif

#endif /* Include guard. */


