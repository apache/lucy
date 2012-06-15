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

/* Charmonizer/Core/ConfWriterC.h -- Write to a C header file.
 */

#ifndef H_CHAZ_CONFWRITERC
#define H_CHAZ_CONFWRITERC 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>
#include "Charmonizer/Core/Defines.h"

/* Enable writing config to a C header file.
 */
void
chaz_ConfWriterC_enable(void);

/* Close the include guard on charmony.h, then close the file.  Delete temp
 * files and perform any other needed cleanup.
 */
void
chaz_ConfWriterC_clean_up(void);

/* Print output to charmony.h.
 */
void
chaz_ConfWriterC_append_conf(const char *fmt, ...);

void
chaz_ConfWriterC_vappend_conf(const char *fmt, va_list args);

/* Add a pound-define.
 */
void
chaz_ConfWriterC_add_def(const char *sym, const char *value);

/* Add a typedef.
 */
void
chaz_ConfWriterC_add_typedef(const char *type, const char *alias);

/* Pound-include a system header (within angle brackets).
 */
void
chaz_ConfWriterC_add_sys_include(const char *header);

/* Pound-include a locally created header (within quotes).
 */
void
chaz_ConfWriterC_add_local_include(const char *header);

/* Print a "chapter heading" comment in the conf file when starting a module.
 */
void
chaz_ConfWriterC_start_module(const char *module_name);

/* Leave a little whitespace at the end of each module.
 */
void
chaz_ConfWriterC_end_module(void);

#ifdef   CHAZ_USE_SHORT_NAMES
  #define ConfWriterC_enable                 chaz_ConfWriterC_enable
  #define ConfWriterC_clean_up               chaz_ConfWriterC_clean_up
  #define ConfWriterC_start_module           chaz_ConfWriterC_start_module
  #define ConfWriterC_end_module             chaz_ConfWriterC_end_module
  #define ConfWriterC_append_conf            chaz_ConfWriterC_append_conf
  #define ConfWriterC_vappend_conf           chaz_ConfWriterC_vappend_conf
  #define ConfWriterC_add_def                chaz_ConfWriterC_add_def
  #define ConfWriterC_add_typedef            chaz_ConfWriterC_add_typedef
  #define ConfWriterC_add_sys_include        chaz_ConfWriterC_add_sys_include
  #define ConfWriterC_add_local_include      chaz_ConfWriterC_add_local_include
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_CONFWRITERC */


