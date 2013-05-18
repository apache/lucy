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

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Probe/Booleans.h"

void
chaz_Booleans_run(void) {
    int has_stdbool = chaz_HeadCheck_check_header("stdbool.h");

    chaz_ConfWriter_start_module("Booleans");

    if (has_stdbool) {
        chaz_ConfWriter_add_def("HAS_STDBOOL_H", NULL);
        chaz_ConfWriter_add_sys_include("stdbool.h");
    }
    else {
        chaz_ConfWriter_append_conf(
            "#if (defined(CHY_EMPLOY_BOOLEANS) && !defined(__cplusplus))\n"
            "  typedef int bool;\n"
            "  #ifndef true\n"
            "    #define true 1\n"
            "  #endif\n"
            "  #ifndef false\n"
            "    #define false 0\n"
            "  #endif\n"
            "#endif\n");
    }

    chaz_ConfWriter_end_module();
}


