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
#include "Charmonizer/Probe/RegularExpressions.h"

void
chaz_RegularExpressions_run(void) {
    int has_regex_h     = chaz_HeadCheck_check_header("regex.h");
    int has_pcre_h      = chaz_HeadCheck_check_header("pcre.h");
    int has_pcreposix_h = chaz_HeadCheck_check_header("pcreposix.h");

    chaz_ConfWriter_start_module("RegularExpressions");

    /* PCRE headers. */
    if (has_pcre_h) {
        chaz_ConfWriter_add_def("HAS_PCRE_H", NULL);
    }
    if (has_pcreposix_h) {
        chaz_ConfWriter_add_def("HAS_PCREPOSIX_H", NULL);
    }

    /* Check for OS X enhanced regexes. */
    if (has_regex_h) {
        const char *reg_enhanced_code =
            CHAZ_QUOTE(  #include <regex.h>                             )
            CHAZ_QUOTE(  int main(int argc, char **argv) {              )
            CHAZ_QUOTE(      regex_t re;                                )
            CHAZ_QUOTE(      if (regcomp(&re, "^", REG_ENHANCED)) {     )
            CHAZ_QUOTE(          return 1;                              )
            CHAZ_QUOTE(      }                                          )
            CHAZ_QUOTE(      return 0;                                  )
            CHAZ_QUOTE(  }                                              );

        if (chaz_CC_test_compile(reg_enhanced_code)) {
            chaz_ConfWriter_add_def("HAS_REG_ENHANCED", NULL);
        }
    }

    chaz_ConfWriter_end_module();
}


