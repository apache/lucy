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
    chaz_bool_t  has_libkern_osatomic_h = false;
    chaz_bool_t  has_osatomic_cas_ptr   = false;
    chaz_bool_t  has_sys_atomic_h       = false;
    chaz_bool_t  has_intrin_h           = false;

    ConfWriter_start_module("AtomicOps");

    if (HeadCheck_check_header("libkern/OSAtomic.h")) {
        has_libkern_osatomic_h = true;
        ConfWriter_append_conf("#define CHY_HAS_LIBKERN_OSATOMIC_H\n");

        /* Check for OSAtomicCompareAndSwapPtr, introduced in later versions
         * of OSAtomic.h. */
        has_osatomic_cas_ptr = CC_test_compile(osatomic_casptr_code,
                                               strlen(osatomic_casptr_code));
        if (has_osatomic_cas_ptr) {
            ConfWriter_append_conf("#define CHY_HAS_OSATOMIC_CAS_PTR\n");
        }
    }
    if (HeadCheck_check_header("sys/atomic.h")) {
        has_sys_atomic_h = true;
        ConfWriter_append_conf("#define CHY_HAS_SYS_ATOMIC_H\n");
    }
    if (HeadCheck_check_header("windows.h")
        && HeadCheck_check_header("intrin.h")
       ) {
        has_intrin_h = true;
        ConfWriter_append_conf("#define CHY_HAS_INTRIN_H\n");
    }

    /* Shorten */
    ConfWriter_start_short_names();
    if (has_libkern_osatomic_h) {
        ConfWriter_shorten_macro("HAS_LIBKERN_OSATOMIC_H");
        if (has_osatomic_cas_ptr) {
            ConfWriter_shorten_macro("HAS_OSATOMIC_CAS_PTR");
        }
    }
    if (has_sys_atomic_h) {
        ConfWriter_shorten_macro("HAS_SYS_ATOMIC_H");
    }
    if (has_intrin_h) {
        ConfWriter_shorten_macro("HAS_INTRIN_H");
    }
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}



