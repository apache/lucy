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

/* Charmonizer/Core/ConfWriter.h -- Write to a config file.
 */

#ifndef H_CHAZ_CONFWRITER
#define H_CHAZ_CONFWRITER 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>
#include "Charmonizer/Core/Defines.h"

/* Initialize elements needed by ConfWriter.  Must be called before anything
 * else, but after os and compiler are initialized.
 */
void
chaz_ConfWriter_init();

/* Open the charmony.h file handle.  Print supplied text to it, if non-null.
 * Print an explanatory comment and open the include guard.
 */
void
chaz_ConfWriter_open_charmony_h(const char *charmony_start);

/* Return the config file's file handle.
 */
FILE*
chaz_ConfWriter_get_charmony_fh(void);

/* Close the include guard on charmony.h, then close the file.  Delete temp
 * files and perform any other needed cleanup.
 */
void
chaz_ConfWriter_clean_up(void);

/* Print output to charmony.h.
 */
void
chaz_ConfWriter_append_conf(const char *fmt, ...);

/* Start a short names block.
 */
void
chaz_ConfWriter_start_short_names(void);

/* Close a short names block.
 */
void
chaz_ConfWriter_end_short_names(void);

/* Define a shortened version of a macro symbol (minus the "CHY_" prefix);
 */
void
chaz_ConfWriter_shorten_macro(const char *symbol);

/* Define a shortened version of a typedef symbol (minus the "chy_" prefix);
 */
void
chaz_ConfWriter_shorten_typedef(const char *symbol);

/* Define a shortened version of a function symbol (minus the "chy_" prefix);
 */
void
chaz_ConfWriter_shorten_function(const char *symbol);

/* Print a "chapter heading" comment in the conf file when starting a module.
 */
void
chaz_ConfWriter_start_module(const char *module_name);

/* Leave a little whitespace at the end of each module.
 */
void
chaz_ConfWriter_end_module(void);

#ifdef   CHAZ_USE_SHORT_NAMES
  #define ConfWriter_init                   chaz_ConfWriter_init
  #define ConfWriter_open_charmony_h        chaz_ConfWriter_open_charmony_h
  #define ConfWriter_get_charmony_fh        chaz_ConfWriter_get_charmony_fh
  #define ConfWriter_clean_up               chaz_ConfWriter_clean_up
  #define ConfWriter_build_charm_run        chaz_ConfWriter_build_charm_run
  #define ConfWriter_start_module           chaz_ConfWriter_start_module
  #define ConfWriter_end_module             chaz_ConfWriter_end_module
  #define ConfWriter_start_short_names      chaz_ConfWriter_start_short_names
  #define ConfWriter_end_short_names        chaz_ConfWriter_end_short_names
  #define ConfWriter_append_conf            chaz_ConfWriter_append_conf
  #define ConfWriter_shorten_macro          chaz_ConfWriter_shorten_macro
  #define ConfWriter_shorten_typedef        chaz_ConfWriter_shorten_typedef
  #define ConfWriter_shorten_function       chaz_ConfWriter_shorten_function
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_CONFWRITER */


