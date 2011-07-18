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

/* Set up the Charmonizer environment.  This should be called before anything
 * else.
 *
 * If the environment variable CHARM_VERBOSITY has been set, it will be
 * processed at this time:
 *
 *      0 - silent
 *      1 - normal
 *      2 - debugging
 *
 * @param cc_command the string used to invoke the C compiler via system()
 * @param cc_flags flags which will be passed on to the C compiler
 * @param charmony_start Code to prepend onto the front of charmony.h
 */
void
chaz_Probe_init(const char *cc_command, const char *cc_flags,
                const char *charmony_start);

/* Clean up the Charmonizer environment -- deleting tempfiles, etc.  This
 * should be called only after everything else finishes.
 */
void
chaz_Probe_clean_up(void);

/* Access the FILE* used to write charmony.h, so that you can write your own
 * content to it.  Should not be called before chaz_Probe_init() or after
 * chaz_Probe_clean_up().
 */
FILE*
chaz_Probe_get_charmony_fh(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Probe_init            chaz_Probe_init
  #define Probe_clean_up        chaz_Probe_clean_up
  #define Probe_get_charmony_fh chaz_Probe_get_charmony_fh
#endif

#ifdef __cplusplus
}
#endif

#endif /* Include guard. */


