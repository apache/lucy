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

#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/AtomicOps.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char osatomic_casptr_code[] =
    QUOTE(  #include <libkern/OSAtomic.h>                                  )
    QUOTE(  #include <libkern/OSAtomic.h>                                  )
    QUOTE(  int main() {                                                   )
    QUOTE(      int  foo = 1;                                              )
    QUOTE(      int *foo_ptr = &foo;                                       )
    QUOTE(      int *target = NULL;                                        )
    QUOTE(      OSAtomicCompareAndSwapPtr(NULL, foo_ptr, (void**)&target); )
    QUOTE(      return 0;                                                  )
    QUOTE(  }                                                              );

void
AtomicOps_run(void) {
    int has_libkern_osatomic_h = false;
    int has_osatomic_cas_ptr   = false;
    int has_sys_atomic_h       = false;
    int has_intrin_h           = false;

    ConfWriter_start_module("AtomicOps");

    if (HeadCheck_check_header("libkern/OSAtomic.h")) {
        has_libkern_osatomic_h = true;
        ConfWriter_add_def("HAS_LIBKERN_OSATOMIC_H", NULL);

        /* Check for OSAtomicCompareAndSwapPtr, introduced in later versions
         * of OSAtomic.h. */
        has_osatomic_cas_ptr = CC_test_compile(osatomic_casptr_code);
        if (has_osatomic_cas_ptr) {
            ConfWriter_add_def("HAS_OSATOMIC_CAS_PTR", NULL);
        }
    }
    if (HeadCheck_check_header("sys/atomic.h")) {
        has_sys_atomic_h = true;
        ConfWriter_add_def("HAS_SYS_ATOMIC_H", NULL);
    }
    if (HeadCheck_check_header("windows.h")
        && HeadCheck_check_header("intrin.h")
       ) {
        has_intrin_h = true;
        ConfWriter_add_def("HAS_INTRIN_H", NULL);
    }

    ConfWriter_end_module();
}



