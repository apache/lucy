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
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/VariadicMacros.h"
#include <string.h>
#include <stdio.h>


/* Code for verifying ISO-style variadic macros. */
static const char chaz_VariadicMacros_iso_code[] =
    CHAZ_QUOTE(  #include <stdio.h>                                    )
    CHAZ_QUOTE(  #define ISO_TEST(fmt, ...) \\                         )
    "                printf(fmt, __VA_ARGS__)                        \n"
    CHAZ_QUOTE(  int main() {                                          )
    CHAZ_QUOTE(      ISO_TEST("%d %d", 1, 1);                          )
    CHAZ_QUOTE(      return 0;                                         )
    CHAZ_QUOTE(  }                                                     );

/* Code for verifying GNU-style variadic macros. */
static const char chaz_VariadicMacros_gnuc_code[] =
    CHAZ_QUOTE(  #include <stdio.h>                                    )
    CHAZ_QUOTE(  #define GNU_TEST(fmt, args...) printf(fmt, ##args)    )
    CHAZ_QUOTE(  int main() {                                          )
    CHAZ_QUOTE(      GNU_TEST("%d %d", 1, 1);                          )
    CHAZ_QUOTE(      return 0;                                         )
    CHAZ_QUOTE(  }                                                     );

void
chaz_VariadicMacros_run(void) {
    char *output;
    size_t output_len;
    int has_varmacros      = false;
    int has_iso_varmacros  = false;
    int has_gnuc_varmacros = false;

    chaz_ConfWriter_start_module("VariadicMacros");

    /* Test for ISO-style variadic macros. */
    output = chaz_CC_capture_output(chaz_VariadicMacros_iso_code, &output_len);
    if (output != NULL) {
        has_varmacros = true;
        has_iso_varmacros = true;
        chaz_ConfWriter_add_def("HAS_VARIADIC_MACROS", NULL);
        chaz_ConfWriter_add_def("HAS_ISO_VARIADIC_MACROS", NULL);
    }

    /* Test for GNU-style variadic macros. */
    output = chaz_CC_capture_output(chaz_VariadicMacros_gnuc_code, &output_len);
    if (output != NULL) {
        has_gnuc_varmacros = true;
        if (has_varmacros == false) {
            has_varmacros = true;
            chaz_ConfWriter_add_def("HAS_VARIADIC_MACROS", NULL);
        }
        chaz_ConfWriter_add_def("HAS_GNUC_VARIADIC_MACROS", NULL);
    }

    chaz_ConfWriter_end_module();
}



