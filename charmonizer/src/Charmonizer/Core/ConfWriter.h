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

#include <stddef.h>
#include <stdarg.h>
#include "Charmonizer/Core/Defines.h"

struct chaz_ConfWriter;

/* Initialize elements needed by ConfWriter.  Must be called before anything
 * else, but after os and compiler are initialized.
 */
void
chaz_ConfWriter_init(void);

/* Close the include guard on charmony.h, then close the file.  Delete temp
 * files and perform any other needed cleanup.
 */
void
chaz_ConfWriter_clean_up(void);

/* Print output to charmony.h.
 */
void
chaz_ConfWriter_append_conf(const char *fmt, ...);

/* Add a pound-define.
 */
void
chaz_ConfWriter_add_def(const char *sym, const char *value);

/* Add a typedef.
 */
void
chaz_ConfWriter_add_typedef(const char *type, const char *alias);

/* Add a globally scoped typedef.
 */
void
chaz_ConfWriter_add_global_typedef(const char *type, const char *alias);

/* Pound-include a system header (within angle brackets).
 */
void
chaz_ConfWriter_add_sys_include(const char *header);

/* Pound-include a locally created header (within quotes).
 */
void
chaz_ConfWriter_add_local_include(const char *header);

/* Print a "chapter heading" comment in the conf file when starting a module.
 */
void
chaz_ConfWriter_start_module(const char *module_name);

/* Leave a little whitespace at the end of each module.
 */
void
chaz_ConfWriter_end_module(void);

void
chaz_ConfWriter_add_writer(struct chaz_ConfWriter *writer);

typedef void
(*chaz_ConfWriter_clean_up_t)(void);
typedef void
(*chaz_ConfWriter_vappend_conf_t)(const char *fmt, va_list args); 
typedef void
(*chaz_ConfWriter_add_def_t)(const char *sym, const char *value);
typedef void
(*chaz_ConfWriter_add_typedef_t)(const char *type, const char *alias);
typedef void
(*chaz_ConfWriter_add_global_typedef_t)(const char *type, const char *alias);
typedef void
(*chaz_ConfWriter_add_sys_include_t)(const char *header);
typedef void
(*chaz_ConfWriter_add_local_include_t)(const char *header);
typedef void
(*chaz_ConfWriter_start_module_t)(const char *module_name);
typedef void
(*chaz_ConfWriter_end_module_t)(void);
typedef struct chaz_ConfWriter {
    chaz_ConfWriter_clean_up_t           clean_up;
    chaz_ConfWriter_vappend_conf_t       vappend_conf;
    chaz_ConfWriter_add_def_t            add_def;
    chaz_ConfWriter_add_typedef_t        add_typedef;
    chaz_ConfWriter_add_global_typedef_t add_global_typedef;
    chaz_ConfWriter_add_sys_include_t    add_sys_include;
    chaz_ConfWriter_add_local_include_t  add_local_include;
    chaz_ConfWriter_start_module_t       start_module;
    chaz_ConfWriter_end_module_t         end_module;
} chaz_ConfWriter;

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_CONFWRITER */


