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

#include "Charmonizer/Probe/SymbolVisibility.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char symbol_exporting_code[] =
    QUOTE(  %s int exported_function() {   )
    QUOTE(      return 42;                 )
    QUOTE(  }                              )
    QUOTE(  int main() {                   )
    QUOTE(      return 0;                  )
    QUOTE(  }                              );

void
SymbolVisibility_run(void) {
    int can_control_visibility = false;
    char code_buf[sizeof(symbol_exporting_code) + 100];

    ConfWriter_start_module("SymbolVisibility");
    CC_set_warnings_as_errors(1);

    /* Windows. */
    if (!can_control_visibility) {
        char export_win[] = "__declspec(dllexport)";
        sprintf(code_buf, symbol_exporting_code, export_win);
        if (CC_test_compile(code_buf, strlen(code_buf))) {
            can_control_visibility = true;
            ConfWriter_append_conf("#define CHY_EXPORT %s\n", export_win);
            ConfWriter_append_conf(
                "#define CHY_IMPORT __declspec(dllimport)\n"
            );
        }
    }

    /* GCC. */
    if (!can_control_visibility) {
        char export_gcc[] = "__attribute__ ((visibility (\"default\")))";
        sprintf(code_buf, symbol_exporting_code, export_gcc);
        if (CC_test_compile(code_buf, strlen(code_buf))) {
            can_control_visibility = true;
            ConfWriter_append_conf("#define CHY_EXPORT %s\n", export_gcc);
            ConfWriter_append_conf("#define CHY_IMPORT\n");
        }
    }
    CC_set_warnings_as_errors(0);

    /* Default. */
    if (!can_control_visibility) {
        ConfWriter_append_conf("#define CHY_EXPORT\n");
        ConfWriter_append_conf("#define CHY_IMPORT\n");
    }

    /* Shorten */
    ConfWriter_start_short_names();
    ConfWriter_shorten_macro("EXPORT");
    ConfWriter_shorten_macro("IMPORT");
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}


