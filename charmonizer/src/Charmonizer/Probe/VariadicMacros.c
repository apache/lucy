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
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/VariadicMacros.h"
#include <string.h>
#include <stdio.h>


/* Code for verifying ISO-style variadic macros. */
static char iso_code[] =
    QUOTE(  #include "_charm.h"                                   )
    QUOTE(  #define ISO_TEST(fmt, ...) \\                         )
    "           printf(fmt, __VA_ARGS__)                        \n"
    QUOTE(  int main() {                                          )
    QUOTE(      Charm_Setup;                                      )
    QUOTE(      ISO_TEST("%d %d", 1, 1);                          )
    QUOTE(      return 0;                                         )
    QUOTE(  }                                                     );

/* Code for verifying GNU-style variadic macros. */
static char gnuc_code[] =
    QUOTE(  #include "_charm.h"                                   )
    QUOTE(  #define GNU_TEST(fmt, args...) printf(fmt, ##args)    )
    QUOTE(  int main() {                                          )
    QUOTE(      Charm_Setup;                                      )
    QUOTE(      GNU_TEST("%d %d", 1, 1);                          )
    QUOTE(      return 0;                                         )
    QUOTE(  }                                                     );

void
VariadicMacros_run(void) {
    char *output;
    size_t output_len;
    chaz_bool_t has_varmacros      = false;
    chaz_bool_t has_iso_varmacros  = false;
    chaz_bool_t has_gnuc_varmacros = false;

    ConfWriter_start_module("VariadicMacros");

    /* Test for ISO-style variadic macros. */
    output = CC_capture_output(iso_code, strlen(iso_code), &output_len);
    if (output != NULL) {
        has_varmacros = true;
        has_iso_varmacros = true;
        ConfWriter_append_conf("#define CHY_HAS_VARIADIC_MACROS\n");
        ConfWriter_append_conf("#define CHY_HAS_ISO_VARIADIC_MACROS\n");
    }

    /* Test for GNU-style variadic macros. */
    output = CC_capture_output(gnuc_code, strlen(gnuc_code), &output_len);
    if (output != NULL) {
        has_gnuc_varmacros = true;
        if (has_varmacros == false) {
            has_varmacros = true;
            ConfWriter_append_conf("#define CHY_HAS_VARIADIC_MACROS\n");
        }
        ConfWriter_append_conf("#define CHY_HAS_GNUC_VARIADIC_MACROS\n");
    }

    /* Shorten. */
    ConfWriter_start_short_names();
    if (has_varmacros) {
        ConfWriter_shorten_macro("HAS_VARIADIC_MACROS");
    }
    if (has_iso_varmacros) {
        ConfWriter_shorten_macro("HAS_ISO_VARIADIC_MACROS");
    }
    if (has_gnuc_varmacros) {
        ConfWriter_shorten_macro("HAS_GNUC_VARIADIC_MACROS");
    }
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}



