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

#define CHAZ_CONFWRITER_INTERNAL
#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include <stdarg.h>
#include <stdio.h>

#define CW_MAX_WRITERS 10
static struct {
    ConfWriter *writers[CW_MAX_WRITERS];
    size_t num_writers;
} CW;

void
ConfWriter_init(void) {
    CW.num_writers = 0;
    return;
}

void
ConfWriter_clean_up(void) {
    size_t i;
    for (i = 0; i < CW.num_writers; i++) {
        CW.writers[i]->clean_up();
    }
}

void
ConfWriter_append_conf(const char *fmt, ...) {
    va_list args;
    size_t i;
    
    for (i = 0; i < CW.num_writers; i++) {
        va_start(args, fmt);
        CW.writers[i]->vappend_conf(fmt, args);
        va_end(args);
    }
}

void
ConfWriter_add_def(const char *sym, const char *value) {
    size_t i;
    for (i = 0; i < CW.num_writers; i++) {
        CW.writers[i]->add_def(sym, value);
    }
}

void
ConfWriter_add_typedef(const char *type, const char *alias) {
    size_t i;
    for (i = 0; i < CW.num_writers; i++) {
        CW.writers[i]->add_typedef(type, alias);
    }
}

void
ConfWriter_add_sys_include(const char *header) {
    size_t i;
    for (i = 0; i < CW.num_writers; i++) {
        CW.writers[i]->add_sys_include(header);
    }
}

void
ConfWriter_add_local_include(const char *header) {
    size_t i;
    for (i = 0; i < CW.num_writers; i++) {
        CW.writers[i]->add_local_include(header);
    }
}

void
ConfWriter_start_module(const char *module_name) {
    size_t i;
    if (chaz_Util_verbosity > 0) {
        printf("Running %s module...\n", module_name);
    }
    for (i = 0; i < CW.num_writers; i++) {
        CW.writers[i]->start_module(module_name);
    }
}

void
ConfWriter_end_module(void) {
    size_t i;
    for (i = 0; i < CW.num_writers; i++) {
        CW.writers[i]->end_module();
    }
}

void
ConfWriter_add_writer(ConfWriter *writer) {
    CW.writers[CW.num_writers] = writer;
    CW.num_writers++;
}

