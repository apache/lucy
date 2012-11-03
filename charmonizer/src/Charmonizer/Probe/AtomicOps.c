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

#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/AtomicOps.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char osatomic_casptr_code[] =
    CHAZ_QUOTE(  #include <libkern/OSAtomic.h>                                  )
    CHAZ_QUOTE(  #include <libkern/OSAtomic.h>                                  )
    CHAZ_QUOTE(  int main() {                                                   )
    CHAZ_QUOTE(      int  foo = 1;                                              )
    CHAZ_QUOTE(      int *foo_ptr = &foo;                                       )
    CHAZ_QUOTE(      int *target = NULL;                                        )
    CHAZ_QUOTE(      OSAtomicCompareAndSwapPtr(NULL, foo_ptr, (void**)&target); )
    CHAZ_QUOTE(      return 0;                                                  )
    CHAZ_QUOTE(  }                                                              );

void
chaz_AtomicOps_run(void) {
    int has_libkern_osatomic_h = false;
    int has_osatomic_cas_ptr   = false;
    int has_sys_atomic_h       = false;
    int has_intrin_h           = false;

    chaz_ConfWriter_start_module("AtomicOps");

    if (chaz_HeadCheck_check_header("libkern/OSAtomic.h")) {
        has_libkern_osatomic_h = true;
        chaz_ConfWriter_add_def("HAS_LIBKERN_OSATOMIC_H", NULL);

        /* Check for OSAtomicCompareAndSwapPtr, introduced in later versions
         * of OSAtomic.h. */
        has_osatomic_cas_ptr = chaz_CC_test_compile(osatomic_casptr_code);
        if (has_osatomic_cas_ptr) {
            chaz_ConfWriter_add_def("HAS_OSATOMIC_CAS_PTR", NULL);
        }
    }
    if (chaz_HeadCheck_check_header("sys/atomic.h")) {
        has_sys_atomic_h = true;
        chaz_ConfWriter_add_def("HAS_SYS_ATOMIC_H", NULL);
    }
    if (chaz_HeadCheck_check_header("windows.h")
        && chaz_HeadCheck_check_header("intrin.h")
       ) {
        has_intrin_h = true;
        chaz_ConfWriter_add_def("HAS_INTRIN_H", NULL);
    }

    chaz_ConfWriter_end_module();
}



