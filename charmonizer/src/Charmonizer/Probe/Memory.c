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

#include "Charmonizer/Probe/Memory.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char alloca_code[] =
    "#include <%s>\n"
    CHAZ_QUOTE(  int main() {                   )
    CHAZ_QUOTE(      void *foo = %s(1);         )
    CHAZ_QUOTE(      return 0;                  )
    CHAZ_QUOTE(  }                              );

void
chaz_Memory_run(void) {
    int has_sys_mman_h = false;
    int has_alloca_h   = false;
    int has_malloc_h   = false;
    int need_stdlib_h  = false;
    int has_alloca     = false;
    int has_builtin_alloca    = false;
    int has_underscore_alloca = false;
    char code_buf[sizeof(alloca_code) + 100];

    chaz_ConfWriter_start_module("Memory");

    {
        /* OpenBSD needs sys/types.h for sys/mman.h to work and mmap() to be
         * available. Everybody else that has sys/mman.h should have
         * sys/types.h as well. */
        const char *mman_headers[] = {
            "sys/types.h",
            "sys/mman.h",
            NULL
        };
        if (chaz_HeadCheck_check_many_headers((const char**)mman_headers)) {
            has_sys_mman_h = true;
            chaz_ConfWriter_add_def("HAS_SYS_MMAN_H", NULL);
        }
    }

    /* Unixen. */
    sprintf(code_buf, alloca_code, "alloca.h", "alloca");
    if (chaz_CC_test_compile(code_buf)) {
        has_alloca_h = true;
        has_alloca   = true;
        chaz_ConfWriter_add_def("HAS_ALLOCA_H", NULL);
        chaz_ConfWriter_add_def("alloca", "alloca");
    }
    if (!has_alloca) {
        sprintf(code_buf, alloca_code, "stdlib.h", "alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_alloca    = true;
            need_stdlib_h = true;
            chaz_ConfWriter_add_def("ALLOCA_IN_STDLIB_H", NULL);
            chaz_ConfWriter_add_def("alloca", "alloca");
        }
    }
    if (!has_alloca) {
        sprintf(code_buf, alloca_code, "stdio.h", /* stdio.h is filler */
                "__builtin_alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_builtin_alloca = true;
            chaz_ConfWriter_add_def("alloca", "__builtin_alloca");
        }
    }

    /* Windows. */
    if (!(has_alloca || has_builtin_alloca)) {
        sprintf(code_buf, alloca_code, "malloc.h", "alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_malloc_h = true;
            has_alloca   = true;
            chaz_ConfWriter_add_def("HAS_MALLOC_H", NULL);
            chaz_ConfWriter_add_def("alloca", "alloca");
        }
    }
    if (!(has_alloca || has_builtin_alloca)) {
        sprintf(code_buf, alloca_code, "malloc.h", "_alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_malloc_h = true;
            has_underscore_alloca = true;
            chaz_ConfWriter_add_def("HAS_MALLOC_H", NULL);
            chaz_ConfWriter_add_def("chy_alloca", "_alloca");
        }
    }

    chaz_ConfWriter_end_module();
}


