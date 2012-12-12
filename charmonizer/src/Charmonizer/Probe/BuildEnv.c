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
#include "Charmonizer/Probe/BuildEnv.h"

void
chaz_BuildEnv_run(void) {
    chaz_ConfWriter_start_module("BuildEnv");

    chaz_ConfWriter_add_def("CC", chaz_CC_get_cc());
    chaz_ConfWriter_add_def("CFLAGS", chaz_CC_get_cflags());
    chaz_ConfWriter_add_def("EXTRA_CFLAGS", chaz_CC_get_extra_cflags());

    chaz_ConfWriter_end_module();
}


