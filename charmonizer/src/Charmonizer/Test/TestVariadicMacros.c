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

#include "charmony.h"
#include <string.h>
#include <stdio.h>
#include "Charmonizer/Test.h"

static void
S_run_tests(void) {
    char buf[10];
    chaz_bool_t really_has_var_macs = false;

#if defined(HAS_ISO_VARIADIC_MACROS) || defined(HAS_GNUC_VARIADIC_MACROS)
  #ifdef HAS_VARIADIC_MACROS
    PASS("#defines agree");
  #else
    FAIL(0, "#defines agree");
  #endif
#else
    SKIP_REMAINING("No variadic macro support");
    return;
#endif


#ifdef HAS_ISO_VARIADIC_MACROS
 #define ISO_TEST(buffer, fmt, ...) \
    sprintf(buffer, fmt, __VA_ARGS__)
    really_has_var_macs = true;
    ISO_TEST(buf, "%s", "iso");
    STR_EQ(buf, "iso", "ISO variadic macros work");
#else
    SKIP("No ISO variadic macros");
#endif

#ifdef HAS_GNUC_VARIADIC_MACROS
 #define GNU_TEST(buffer, fmt, args...) \
    sprintf(buffer, fmt, ##args )
    really_has_var_macs = true;
    GNU_TEST(buf, "%s", "gnu");
    STR_EQ(buf, "gnu", "GNUC variadic macros work");
#else
    SKIP("No GNUC variadic macros");
#endif

    OK(really_has_var_macs, "either ISO or GNUC");
}

int main(int argc, char **argv) {
    Test_start(4);
    S_run_tests();
    return !Test_finish();
}

