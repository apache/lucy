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
#include "Charmonizer/Probe/Strings.h"

/* Check for C99-compatible snprintf and possible replacements.
 */
static void
chaz_Strings_probe_c99_snprintf(void);

void
chaz_Strings_run(void) {
    chaz_ConfWriter_start_module("Strings");

    /* Check for C99 snprintf. */
    chaz_Strings_probe_c99_snprintf();

    chaz_ConfWriter_end_module();
}

static void
chaz_Strings_probe_c99_snprintf(void) {
    static const char snprintf_code[] =
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  int main() {                                   )
        CHAZ_QUOTE(      char buf[4];                               )
        CHAZ_QUOTE(      int  result;                               )
        CHAZ_QUOTE(      result = snprintf(buf, 4, "%s", "12345");  )
        CHAZ_QUOTE(      printf("%d", result);                      )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    static const char detect__scprintf_code[] =
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  int main() {                                   )
        CHAZ_QUOTE(      int  result;                               )
        CHAZ_QUOTE(      result = _scprintf("%s", "12345");         )
        CHAZ_QUOTE(      printf("%d", result);                      )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    static const char detect__snprintf_code[] =
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  int main() {                                   )
        CHAZ_QUOTE(      char buf[6];                               )
        CHAZ_QUOTE(      int  result;                               )
        CHAZ_QUOTE(      result = _snprintf(buf, 6, "%s", "12345"); )
        CHAZ_QUOTE(      printf("%d", result);                      )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    char   *output = NULL;
    size_t  output_len;

    /* If the buffer passed to snprintf is too small, verify that snprintf
     * returns the length of the untruncated string which would have been
     * written to a large enough buffer.
     */
    output = chaz_CC_capture_output(snprintf_code, &output_len);
    if (output != NULL) {
        long result = strtol(output, NULL, 10);
        if (result == 5) {
            chaz_ConfWriter_add_def("HAS_C99_SNPRINTF", NULL);
        }
        free(output);
    }

    /* Test for _scprintf and _snprintf found in the MSVCRT.
     */
    output = chaz_CC_capture_output(detect__scprintf_code, &output_len);
    if (output != NULL) {
        chaz_ConfWriter_add_def("HAS__SCPRINTF", NULL);
        free(output);
    }
    output = chaz_CC_capture_output(detect__snprintf_code, &output_len);
    if (output != NULL) {
        chaz_ConfWriter_add_def("HAS__SNPRINTF", NULL);
        free(output);
    }
}

