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
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/LargeFiles.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Module vars. */
static struct {
    char off64_type[10];
} chaz_LargeFiles = { "" };

/* Sets of symbols which might provide large file support for stdio. */
typedef struct chaz_LargeFiles_stdio64_combo {
    const char *includes;
    const char *fopen_command;
    const char *ftell_command;
    const char *fseek_command;
} chaz_LargeFiles_stdio64_combo;

/* Sets of symbols which might provide large file support for unbuffered i/o.
 */
typedef struct chaz_LargeFiles_unbuff_combo {
    const char *includes;
    const char *lseek_command;
    const char *pread64_command;
} chaz_LargeFiles_unbuff_combo;

/* Check for a 64-bit file pointer type.
 */
static const int
chaz_LargeFiles_probe_off64(void);

/* Check what name 64-bit ftell, fseek go by.
 */
static void
chaz_LargeFiles_probe_stdio64(void);
static int
chaz_LargeFiles_try_stdio64(chaz_LargeFiles_stdio64_combo *combo);

/* Probe for 64-bit unbuffered i/o.
 */
static void
chaz_LargeFiles_probe_unbuff(void);

/* Check for a 64-bit lseek.
 */
static int
chaz_LargeFiles_probe_lseek(chaz_LargeFiles_unbuff_combo *combo);

/* Check for a 64-bit pread.
 */
static int
chaz_LargeFiles_probe_pread64(chaz_LargeFiles_unbuff_combo *combo);

void
chaz_LargeFiles_run(void) {
    int found_off64_t = false;
    unsigned i;
    const char *stat_includes = "#include <stdio.h>\n#include <sys/stat.h>";

    chaz_ConfWriter_start_module("LargeFiles");

    /* Find off64_t or equivalent. */
    found_off64_t = chaz_LargeFiles_probe_off64();
    if (found_off64_t) {
        chaz_ConfWriter_add_def("HAS_64BIT_OFFSET_TYPE", NULL);
        chaz_ConfWriter_add_def("off64_t",  chaz_LargeFiles.off64_type);
    }

    /* See if stdio variants with 64-bit support exist. */
    chaz_LargeFiles_probe_stdio64();

    /* Probe for 64-bit versions of lseek and pread (if we have an off64_t). */
    if (found_off64_t) {
        chaz_LargeFiles_probe_unbuff();
    }

    /* Make checks needed for testing. */
    if (chaz_HeadCheck_check_header("sys/stat.h")) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_SYS_STAT_H\n");
    }
    if (chaz_HeadCheck_check_header("io.h")) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_IO_H\n");
    }
    if (chaz_HeadCheck_check_header("fcntl.h")) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_FCNTL_H\n");
    }
    if (chaz_HeadCheck_contains_member("struct stat", "st_size", stat_includes)) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_STAT_ST_SIZE\n");
    }
    if (chaz_HeadCheck_contains_member("struct stat", "st_blocks", stat_includes)) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_STAT_ST_BLOCKS\n");
    }

    chaz_ConfWriter_end_module();
}

static const int
chaz_LargeFiles_probe_off64(void) {
    static const char off64_code[] =
        CHAZ_QUOTE(  %s                                        )
        CHAZ_QUOTE(  #include <stdio.h>                        )
        CHAZ_QUOTE(  int main()                                )
        CHAZ_QUOTE(  {                                         )
        CHAZ_QUOTE(      printf("%%d", (int)sizeof(%s));       )
        CHAZ_QUOTE(      return 0;                             )
        CHAZ_QUOTE(  }                                         );
    char code_buf[sizeof(off64_code) + 100];
    int i;
    int success = false;
    static const char* off64_options[] = {
        "off64_t",
        "off_t",
        "__int64",
        "long"
    };
    int num_off64_options = sizeof(off64_options) / sizeof(off64_options[0]);

    for (i = 0; i < num_off64_options; i++) {
        const char *candidate = off64_options[i];
        char *output;
        size_t output_len;
        int has_sys_types_h = chaz_HeadCheck_check_header("sys/types.h");
        const char *sys_types_include = has_sys_types_h
                                        ? "#include <sys/types.h>"
                                        : "";

        /* Execute the probe. */
        sprintf(code_buf, off64_code, sys_types_include, candidate);
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            long sizeof_candidate = strtol(output, NULL, 10);
            free(output);
            if (sizeof_candidate == 8) {
                strcpy(chaz_LargeFiles.off64_type, candidate);
                success = true;
                break;
            }
        }
    }
    return success;
}

static int
chaz_LargeFiles_try_stdio64(chaz_LargeFiles_stdio64_combo *combo) {
    static const char stdio64_code[] =
        CHAZ_QUOTE(  %s                                         )
        CHAZ_QUOTE(  #include <stdio.h>                         )
        CHAZ_QUOTE(  int main() {                               )
        CHAZ_QUOTE(      %s pos;                                )
        CHAZ_QUOTE(      FILE *f;                               )
        CHAZ_QUOTE(      f = %s("_charm_stdio64", "w");         )
        CHAZ_QUOTE(      if (f == NULL) return -1;              )
        CHAZ_QUOTE(      printf("%%d", (int)sizeof(%s));        )
        CHAZ_QUOTE(      pos = %s(stdout);                      )
        CHAZ_QUOTE(      %s(stdout, 0, SEEK_SET);               )
        CHAZ_QUOTE(      return 0;                              )
        CHAZ_QUOTE(  }                                          );
    char *output = NULL;
    size_t output_len;
    char code_buf[sizeof(stdio64_code) + 200];
    int success = false;

    /* Prepare the source code. */
    sprintf(code_buf, stdio64_code, combo->includes,
            chaz_LargeFiles.off64_type, combo->fopen_command,
            chaz_LargeFiles.off64_type, combo->ftell_command,
            combo->fseek_command);

    /* Verify compilation and that the offset type has 8 bytes. */
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        long size = strtol(output, NULL, 10);
        if (size == 8) {
            success = true;
        }
        free(output);
    }

    if (!chaz_Util_remove_and_verify("_charm_stdio64")) {
        chaz_Util_die("Failed to remove '_charm_stdio64'");
    }

    return success;
}

static void
chaz_LargeFiles_probe_stdio64(void) {
    int i;
    static chaz_LargeFiles_stdio64_combo stdio64_combos[] = {
        { "#include <sys/types.h>\n", "fopen64",   "ftello64",  "fseeko64"  },
        { "#include <sys/types.h>\n", "fopen",     "ftello64",  "fseeko64"  },
        { "#include <sys/types.h>\n", "fopen",     "ftello",    "fseeko"    },
        { "",                         "fopen",     "ftell",     "fseek"     },
        { "",                         "fopen",     "_ftelli64", "_fseeki64" },
        { "",                         "fopen",     "ftell",     "fseek"     },
        { NULL, NULL, NULL, NULL }
    };

    for (i = 0; stdio64_combos[i].includes != NULL; i++) {
        chaz_LargeFiles_stdio64_combo combo = stdio64_combos[i];
        if (chaz_LargeFiles_try_stdio64(&combo)) {
            chaz_ConfWriter_add_def("HAS_64BIT_STDIO", NULL);
            chaz_ConfWriter_add_def("fopen64",  combo.fopen_command);
            chaz_ConfWriter_add_def("ftello64", combo.ftell_command);
            chaz_ConfWriter_add_def("fseeko64", combo.fseek_command);
            break;
        }
    }
}

static int
chaz_LargeFiles_probe_lseek(chaz_LargeFiles_unbuff_combo *combo) {
    static const char lseek_code[] =
        CHAZ_QUOTE( %s                                                       )
        CHAZ_QUOTE( #include <stdio.h>                                       )
        CHAZ_QUOTE( int main() {                                             )
        CHAZ_QUOTE(     int fd;                                              )
        CHAZ_QUOTE(     fd = open("_charm_lseek", O_WRONLY | O_CREAT, 0666); )
        CHAZ_QUOTE(     if (fd == -1) { return -1; }                         )
        CHAZ_QUOTE(     %s(fd, 0, SEEK_SET);                                 )
        CHAZ_QUOTE(     printf("%%d", 1);                                    )
        CHAZ_QUOTE(     if (close(fd)) { return -1; }                        )
        CHAZ_QUOTE(     return 0;                                            )
        CHAZ_QUOTE( }                                                        );
    char code_buf[sizeof(lseek_code) + 100];
    char *output = NULL;
    size_t output_len;
    int success = false;

    /* Verify compilation. */
    sprintf(code_buf, lseek_code, combo->includes, combo->lseek_command);
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        success = true;
        free(output);
    }

    if (!chaz_Util_remove_and_verify("_charm_lseek")) {
        chaz_Util_die("Failed to remove '_charm_lseek'");
    }

    return success;
}

static int
chaz_LargeFiles_probe_pread64(chaz_LargeFiles_unbuff_combo *combo) {
    /* Code for checking 64-bit pread.  The pread call will fail, but that's
     * fine as long as it compiles. */
    static const char pread64_code[] =
        CHAZ_QUOTE(  %s                                     )
        CHAZ_QUOTE(  #include <stdio.h>                     )
        CHAZ_QUOTE(  int main() {                           )
        CHAZ_QUOTE(      int fd = 20;                       )
        CHAZ_QUOTE(      char buf[1];                       )
        CHAZ_QUOTE(      printf("1");                       )
        CHAZ_QUOTE(      %s(fd, buf, 1, 1);                 )
        CHAZ_QUOTE(      return 0;                          )
        CHAZ_QUOTE(  }                                      );
    char code_buf[sizeof(pread64_code) + 100];
    char *output = NULL;
    size_t output_len;
    int success = false;

    /* Verify compilation. */
    sprintf(code_buf, pread64_code, combo->includes, combo->pread64_command);
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        success = true;
        free(output);
    }

    return success;
}

static void
chaz_LargeFiles_probe_unbuff(void) {
    static chaz_LargeFiles_unbuff_combo unbuff_combos[] = {
        { "#include <unistd.h>\n#include <fcntl.h>\n", "lseek64",   "pread64" },
        { "#include <unistd.h>\n#include <fcntl.h>\n", "lseek",     "pread"      },
        { "#include <io.h>\n#include <fcntl.h>\n",     "_lseeki64", "NO_PREAD64" },
        { NULL, NULL, NULL }
    };
    int i;

    for (i = 0; unbuff_combos[i].lseek_command != NULL; i++) {
        chaz_LargeFiles_unbuff_combo combo = unbuff_combos[i];
        if (chaz_LargeFiles_probe_lseek(&combo)) {
            chaz_ConfWriter_add_def("HAS_64BIT_LSEEK", NULL);
            chaz_ConfWriter_add_def("lseek64", combo.lseek_command);
            break;
        }
    }
    for (i = 0; unbuff_combos[i].pread64_command != NULL; i++) {
        chaz_LargeFiles_unbuff_combo combo = unbuff_combos[i];
        if (chaz_LargeFiles_probe_pread64(&combo)) {
            chaz_ConfWriter_add_def("HAS_64BIT_PREAD", NULL);
            chaz_ConfWriter_add_def("pread64", combo.pread64_command);
            break;
        }
    }
};

