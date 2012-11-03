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

#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include <stdarg.h>
#include <stdio.h>

#define CW_MAX_WRITERS 10
static struct {
    chaz_ConfWriter *writers[CW_MAX_WRITERS];
    size_t num_writers;
} chaz_CW;

void
chaz_ConfWriter_init(void) {
    chaz_CW.num_writers = 0;
    return;
}

void
chaz_ConfWriter_clean_up(void) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->clean_up();
    }
}

void
chaz_ConfWriter_append_conf(const char *fmt, ...) {
    va_list args;
    size_t i;
    
    for (i = 0; i < chaz_CW.num_writers; i++) {
        va_start(args, fmt);
        chaz_CW.writers[i]->vappend_conf(fmt, args);
        va_end(args);
    }
}

void
chaz_ConfWriter_add_def(const char *sym, const char *value) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_def(sym, value);
    }
}

void
chaz_ConfWriter_add_typedef(const char *type, const char *alias) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_typedef(type, alias);
    }
}

void
chaz_ConfWriter_add_sys_include(const char *header) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_sys_include(header);
    }
}

void
chaz_ConfWriter_add_local_include(const char *header) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_local_include(header);
    }
}

void
chaz_ConfWriter_start_module(const char *module_name) {
    size_t i;
    if (chaz_Util_verbosity > 0) {
        printf("Running %s module...\n", module_name);
    }
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->start_module(module_name);
    }
}

void
chaz_ConfWriter_end_module(void) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->end_module();
    }
}

void
chaz_ConfWriter_add_writer(chaz_ConfWriter *writer) {
    chaz_CW.writers[chaz_CW.num_writers] = writer;
    chaz_CW.num_writers++;
}

