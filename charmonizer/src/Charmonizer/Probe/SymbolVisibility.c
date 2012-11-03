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

#include "Charmonizer/Probe/SymbolVisibility.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char chaz_SymbolVisibility_symbol_exporting_code[] =
    CHAZ_QUOTE(  %s int exported_function() {   )
    CHAZ_QUOTE(      return 42;                 )
    CHAZ_QUOTE(  }                              )
    CHAZ_QUOTE(  int main() {                   )
    CHAZ_QUOTE(      return 0;                  )
    CHAZ_QUOTE(  }                              );

void
chaz_SymbolVisibility_run(void) {
    int can_control_visibility = false;
    char code_buf[sizeof(chaz_SymbolVisibility_symbol_exporting_code) + 100];

    chaz_ConfWriter_start_module("SymbolVisibility");
    chaz_CC_set_warnings_as_errors(1);

    /* Windows. */
    if (!can_control_visibility) {
        char export_win[] = "__declspec(dllexport)";
        sprintf(code_buf, chaz_SymbolVisibility_symbol_exporting_code,
                export_win);
        if (chaz_CC_test_compile(code_buf)) {
            can_control_visibility = true;
            chaz_ConfWriter_add_def("EXPORT", export_win);
            chaz_ConfWriter_add_def("IMPORT", "__declspec(dllimport)");
        }
    }

    /* GCC. */
    if (!can_control_visibility) {
        char export_gcc[] = "__attribute__ ((visibility (\"default\")))";
        sprintf(code_buf, chaz_SymbolVisibility_symbol_exporting_code,
                export_gcc);
        if (chaz_CC_test_compile(code_buf)) {
            can_control_visibility = true;
            chaz_ConfWriter_add_def("EXPORT", export_gcc);
            chaz_ConfWriter_add_def("IMPORT", NULL);
        }
    }
    chaz_CC_set_warnings_as_errors(0);

    /* Default. */
    if (!can_control_visibility) {
        chaz_ConfWriter_add_def("EXPORT", NULL);
        chaz_ConfWriter_add_def("IMPORT", NULL);
    }

    chaz_ConfWriter_end_module();
}


