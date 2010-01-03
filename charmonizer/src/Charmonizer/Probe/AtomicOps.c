#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/AtomicOps.h"
#include <string.h>
#include <stdio.h>


void
AtomicOps_run(void) 
{
    chaz_bool_t has_libkern_osatomic_h = false;
    chaz_bool_t has_sys_atomic_h       = false;
    chaz_bool_t has_intrin_h           = false;

    START_RUN("AtomicOps");

    if (HeadCheck_check_header("libkern/OSAtomic.h")) {
        has_libkern_osatomic_h = true;
        ConfWriter_append_conf("#define CHY_HAS_LIBKERN_OSATOMIC_H\n");
    }
    if (HeadCheck_check_header("sys/atomic.h")) {
        has_sys_atomic_h = true;
        ConfWriter_append_conf("#define CHY_HAS_SYS_ATOMIC_H\n");
    }
    if (   HeadCheck_check_header("windows.h")
        && HeadCheck_check_header("intrin.h")
    ) {
        has_intrin_h = true;
        ConfWriter_append_conf("#define CHY_HAS_INTRIN_H\n");
    }
    
    /* Shorten */
    START_SHORT_NAMES;
    if (has_libkern_osatomic_h) {
        ConfWriter_shorten_macro("HAS_LIBKERN_OSATOMIC_H");
    }
    if (has_sys_atomic_h) {
        ConfWriter_shorten_macro("HAS_SYS_ATOMIC_H");
    }
    if (has_intrin_h) {
        ConfWriter_shorten_macro("HAS_INTRIN_H");
    }
    END_SHORT_NAMES;

    END_RUN;
}


/**
 * Copyright 2009 The Apache Software Foundation
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

