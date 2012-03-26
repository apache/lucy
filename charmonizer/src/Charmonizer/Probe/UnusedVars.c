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

#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/UnusedVars.h"
#include <string.h>
#include <stdio.h>


void
UnusedVars_run(void) {
    ConfWriter_start_module("UnusedVars");

    /* Write the macros (no test, these are the same everywhere). */
    ConfWriter_append_conf("#define CHY_UNUSED_VAR(x) ((void)x)\n");
    ConfWriter_append_conf("#define CHY_UNREACHABLE_RETURN(type) return (type)0\n");

    /* Shorten. */
    ConfWriter_start_short_names();
    ConfWriter_shorten_macro("UNUSED_VAR");
    ConfWriter_shorten_macro("UNREACHABLE_RETURN");
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}



