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

#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/ConfWriterC.h"
#include <stdarg.h>
#include <stdio.h>

void
ConfWriter_init(void) {
    ConfWriterC_enable();
    return;
}

void
ConfWriter_clean_up(void) {
    ConfWriterC_clean_up();
}

void
ConfWriter_append_conf(const char *fmt, ...) {
    va_list args;
    
    va_start(args, fmt);
    ConfWriterC_vappend_conf(fmt, args);
    va_end(args);
}

void
ConfWriter_add_def(const char *sym, const char *value) {
    ConfWriterC_add_def(sym, value);
}

void
ConfWriter_add_typedef(const char *type, const char *alias) {
    ConfWriterC_add_typedef(type, alias);
}

void
ConfWriter_add_sys_include(const char *header) {
    ConfWriterC_add_sys_include(header);
}

void
ConfWriter_add_local_include(const char *header) {
    ConfWriterC_add_local_include(header);
}

void
ConfWriter_start_module(const char *module_name) {
    if (chaz_Util_verbosity > 0) {
        printf("Running %s module...\n", module_name);
    }
    ConfWriterC_start_module(module_name);
}

void
ConfWriter_end_module(void) {
    ConfWriterC_end_module();
}

