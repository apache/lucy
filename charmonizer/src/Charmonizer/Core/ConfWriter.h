/* Chaz/Core/ConfWriter.h -- Write to a config file.
 */

#ifndef H_CHAZ_CONFWRITER
#define H_CHAZ_CONFWRITER 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>
#include "Charmonizer/Core/Defines.h"

/* Temporary files used by Charmonizer. 
 */
#define CHAZ_CONFWRITER_TRY_SOURCE_PATH  "_charmonizer_try.c"
#define CHAZ_CONFWRITER_TRY_APP_BASENAME "_charmonizer_try"
#define CHAZ_CONFWRITER_TARGET_PATH      "_charmonizer_target"

/* Global variables.
 */
extern chaz_bool_t chaz_ConfWriter_charm_run_available;
extern FILE* chaz_ConfWriter_charmony_fh;

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

/* Close the include guard on charmony.h, then close the file.  Delete temp
 * files and perform any other needed cleanup.
 */
void
chaz_ConfWriter_clean_up(void);

/* Attempt to compile the supplied source code and return true if the
 * effort succeeds.
 */
chaz_bool_t
chaz_ConfWriter_test_compile(char *source, size_t source_len);

/* Attempt to compile the supplied source code.  If successful, capture the 
 * output of the program and return a pointer to a newly allocated buffer.
 * If the compilation fails, return NULL.  The length of the captured 
 * output will be placed into the integer pointed to by [output_len].
 */
char*
chaz_ConfWriter_capture_output(char *source, size_t source_len, 
                            size_t *output_len);

/* Print output to charmony.h.
 */
void
chaz_ConfWriter_append_conf(const char *fmt, ...);

/* Print bookends delimiting a short names block.
 */
#define CHAZ_CONFWRITER_START_SHORT_NAMES \
  chaz_ConfWriter_append_conf( \
    "\n#if defined(CHY_USE_SHORT_NAMES) || defined(CHAZ_USE_SHORT_NAMES)\n")

#define CHAZ_CONFWRITER_END_SHORT_NAMES \
    chaz_ConfWriter_append_conf("#endif /* USE_SHORT_NAMES */\n")

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

/* Print a "chapter heading" when starting a module. 
 */
#define CHAZ_CONFWRITER_START_RUN(module_name) \
    do { \
        chaz_ConfWriter_append_conf("\n/* %s */\n", module_name); \
        if (chaz_Util_verbosity > 0) { \
            printf("Running %s module...\n", module_name); \
        } \
    } while (0)

/* Leave a little whitespace at the end of each module.
 */
#define CHAZ_CONFWRITER_END_RUN chaz_ConfWriter_append_conf("\n")

#ifdef   CHAZ_USE_SHORT_NAMES
  #define TRY_SOURCE_PATH                   CHAZ_CONFWRITER_TRY_SOURCE_PATH
  #define TRY_APP_BASENAME                  CHAZ_CONFWRITER_TRY_APP_BASENAME
  #define TARGET_PATH                       CHAZ_CONFWRITER_TARGET_PATH
  #define ConfWriter_charm_run_available    chaz_ConfWriter_charm_run_available
  #define ConfWriter_charmony_fh            chaz_ConfWriter_charmony_fh
  #define ConfWriter_init                   chaz_ConfWriter_init
  #define ConfWriter_open_charmony_h        chaz_ConfWriter_open_charmony_h
  #define ConfWriter_clean_up               chaz_ConfWriter_clean_up
  #define ConfWriter_write_charm_h          chaz_ConfWriter_write_charm_h
  #define ConfWriter_build_charm_run        chaz_ConfWriter_build_charm_run
  #define START_SHORT_NAMES                 CHAZ_CONFWRITER_START_SHORT_NAMES
  #define END_SHORT_NAMES                   CHAZ_CONFWRITER_END_SHORT_NAMES
  #define ConfWriter_test_compile           chaz_ConfWriter_test_compile
  #define ConfWriter_capture_output         chaz_ConfWriter_capture_output 
  #define ConfWriter_append_conf            chaz_ConfWriter_append_conf
  #define ConfWriter_shorten_macro          chaz_ConfWriter_shorten_macro
  #define ConfWriter_shorten_typedef        chaz_ConfWriter_shorten_typedef
  #define ConfWriter_shorten_function       chaz_ConfWriter_shorten_function
  #define START_RUN                         CHAZ_CONFWRITER_START_RUN
  #define END_RUN                           CHAZ_CONFWRITER_END_RUN
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_CONFWRITER */

/**
 * Copyright 2006 The Apache Software Foundation
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

