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

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/LargeFiles.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char* off64_options[] = {
    "off64_t",
    "off_t",
    "__int64",
    "long"
};

#define NUM_OFF64_OPTIONS (sizeof(off64_options) / sizeof(off64_options[0]))

/* Sets of symbols which might provide large file support. */
typedef struct stdio64_combo {
    const char *includes;
    const char *fopen_command;
    const char *ftell_command;
    const char *fseek_command;
} stdio64_combo;
static stdio64_combo stdio64_combos[] = {
    { "#include <sys/types.h>\n", "fopen64",   "ftello64",  "fseeko64"  },
    { "#include <sys/types.h>\n", "fopen",     "ftello64",  "fseeko64"  },
    { "#include <sys/types.h>\n", "fopen",     "ftello",    "fseeko"    },
    { "",                         "fopen",     "ftell",     "fseek"     },
    { "",                         "fopen",     "_ftelli64", "_fseeki64" },
    { "",                         "fopen",     "ftell",     "fseek"     },
    { NULL, NULL, NULL, NULL }
};

typedef struct unbuff_combo {
    const char *includes;
    const char *lseek_command;
    const char *pread64_command;
} unbuff_combo;
static unbuff_combo unbuff_combos[] = {
    { "#include <unistd.h>\n#include <fcntl.h>\n", "lseek64",   "pread64" },
    { "#include <unistd.h>\n#include <fcntl.h>\n", "lseek",     "pread"      },
    { "#include <io.h>\n#include <fcntl.h>\n",     "_lseeki64", "NO_PREAD64" },
    { NULL, NULL, NULL }
};

/* Check for a 64-bit file pointer type.
 */
static const chaz_bool_t
S_probe_off64(void);

/* Check what name 64-bit ftell, fseek go by.
 */
static chaz_bool_t
S_probe_stdio64(stdio64_combo *combo);

/* Check for a 64-bit lseek.
 */
static chaz_bool_t
S_probe_lseek(unbuff_combo *combo);

/* Check for a 64-bit pread.
 */
static chaz_bool_t
S_probe_pread64(unbuff_combo *combo);

/* Vars for holding lfs commands, once they're discovered. */
static char fopen_command[10];
static char fseek_command[10];
static char ftell_command[10];
static char lseek_command[10];
static char pread64_command[10];
static char off64_type[10];

void
LargeFiles_run(void) {
    chaz_bool_t found_off64_t = false;
    chaz_bool_t found_stdio64 = false;
    chaz_bool_t found_lseek = false;
    chaz_bool_t found_pread64 = false;
    unsigned i;
    const char *stat_includes = "#include <stdio.h>\n#include <sys/stat.h>";

    ConfWriter_start_module("LargeFiles");

    /* Find off64_t or equivalent. */
    found_off64_t = S_probe_off64();
    if (found_off64_t) {
        ConfWriter_append_conf("#define CHY_HAS_64BIT_OFFSET_TYPE\n");
        ConfWriter_append_conf("#define chy_off64_t %s\n",  off64_type);
    }

    /* See if stdio variants with 64-bit support exist. */
    for (i = 0; stdio64_combos[i].includes != NULL; i++) {
        stdio64_combo combo = stdio64_combos[i];
        if (S_probe_stdio64(&combo)) {
            found_stdio64 = true;
            ConfWriter_append_conf("#define CHY_HAS_64BIT_STDIO\n");
            strcpy(fopen_command, combo.fopen_command);
            strcpy(fseek_command, combo.fseek_command);
            strcpy(ftell_command, combo.ftell_command);
            ConfWriter_append_conf("#define chy_fopen64 %s\n",  fopen_command);
            ConfWriter_append_conf("#define chy_ftello64 %s\n", ftell_command);
            ConfWriter_append_conf("#define chy_fseeko64 %s\n", fseek_command);
            break;
        }
    }

    /* Probe for 64-bit versions of lseek and pread (if we have an off64_t). */
    if (found_off64_t) {
        for (i = 0; unbuff_combos[i].lseek_command != NULL; i++) {
            unbuff_combo combo = unbuff_combos[i];
            found_lseek = S_probe_lseek(&combo);
            if (found_lseek) {
                ConfWriter_append_conf("#define CHY_HAS_64BIT_LSEEK\n");
                strcpy(lseek_command, combo.lseek_command);
                ConfWriter_append_conf("#define chy_lseek64 %s\n",
                                       lseek_command);
                break;
            }
        }
        for (i = 0; unbuff_combos[i].pread64_command != NULL; i++) {
            unbuff_combo combo = unbuff_combos[i];
            found_pread64 = S_probe_pread64(&combo);
            if (found_pread64) {
                ConfWriter_append_conf("#define CHY_HAS_64BIT_PREAD\n");
                strcpy(pread64_command, combo.pread64_command);
                ConfWriter_append_conf("#define chy_pread64 %s\n",
                                       pread64_command);
                found_pread64 = true;
                break;
            }
        }
    }

    /* Make checks needed for testing. */
    if (HeadCheck_check_header("sys/stat.h")) {
        ConfWriter_append_conf("#define CHAZ_HAS_SYS_STAT_H\n");
    }
    if (HeadCheck_check_header("io.h")) {
        ConfWriter_append_conf("#define CHAZ_HAS_IO_H\n");
    }
    if (HeadCheck_check_header("fcntl.h")) {
        ConfWriter_append_conf("#define CHAZ_HAS_FCNTL_H\n");
    }
    if (HeadCheck_contains_member("struct stat", "st_size", stat_includes)) {
        ConfWriter_append_conf("#define CHAZ_HAS_STAT_ST_SIZE\n");
    }
    if (HeadCheck_contains_member("struct stat", "st_blocks", stat_includes)) {
        ConfWriter_append_conf("#define CHAZ_HAS_STAT_ST_BLOCKS\n");
    }

    /* Short names. */
    ConfWriter_start_short_names();
    if (found_off64_t) {
        ConfWriter_shorten_macro("HAS_64BIT_OFFSET_TYPE");
        if (strcmp(off64_type, "off64_t") != 0) {
            ConfWriter_shorten_typedef("off64_t");
        }
    }
    if (found_stdio64) {
        ConfWriter_shorten_macro("HAS_64BIT_STDIO");
        if (strcmp(fopen_command, "fopen64") != 0) {
            ConfWriter_shorten_function("fopen64");
        }
        if (strcmp(fopen_command, "ftello64") != 0) {
            ConfWriter_shorten_function("ftello64");
        }
        if (strcmp(fopen_command, "fseeko64") != 0) {
            ConfWriter_shorten_function("fseeko64");
        }
    }
    if (found_lseek) {
        ConfWriter_shorten_macro("HAS_64BIT_LSEEK");
        if (strcmp(lseek_command, "lseek64") != 0) {
            ConfWriter_shorten_function("lseek64");
        }
    }
    if (found_pread64) {
        ConfWriter_shorten_macro("HAS_64BIT_PREAD");
        if (strcmp(pread64_command, "pread64") != 0) {
            ConfWriter_shorten_function("pread64");
        }
    }
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}

/* Code for finding an off64_t or some other 64-bit signed type. */
static const char off64_code[] =
    QUOTE(  %s                                        )
    QUOTE(  #include "_charm.h"                       )
    QUOTE(  int main()                                )
    QUOTE(  {                                         )
    QUOTE(      Charm_Setup;                          )
    QUOTE(      printf("%%d", (int)sizeof(%s));       )
    QUOTE(      return 0;                             )
    QUOTE(  }                                         );

static const chaz_bool_t
S_probe_off64(void) {
    size_t needed = sizeof(off64_code) + 100;
    char *code_buf = (char*)malloc(needed);
    int i;
    chaz_bool_t success = false;
    for (i = 0; i < NUM_OFF64_OPTIONS; i++) {
        const char *candidate = off64_options[i];
        char *output;
        size_t output_len;
        chaz_bool_t has_sys_types_h = HeadCheck_check_header("sys/types.h");
        const char *sys_types_include = has_sys_types_h
                                        ? "#include <sys/types.h>"
                                        : "";

        /* Execute the probe. */
        sprintf(code_buf, off64_code, sys_types_include, candidate);
        output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
        if (output != NULL) {
            long sizeof_candidate = strtol(output, NULL, 10);
            free(output);
            if (sizeof_candidate == 8) {
                strcpy(off64_type, candidate);
                success = true;
                break;
            }
        }
    }
    free(code_buf);
    return success;
}

/* Code for checking ftello64 and friends. */
static const char stdio64_code[] =
    QUOTE(  %s                                         )
    QUOTE(  #include "_charm.h"                        )
    QUOTE(  int main() {                               )
    QUOTE(      %s pos;                                )
    QUOTE(      FILE *f;                               )
    QUOTE(      Charm_Setup;                           )
    QUOTE(      f = %s("_charm_stdio64", "w");         )
    QUOTE(      if (f == NULL) return -1;              )
    QUOTE(      printf("%%d", (int)sizeof(%s));        )
    QUOTE(      pos = %s(stdout);                      )
    QUOTE(      %s(stdout, 0, SEEK_SET);               )
    QUOTE(      return 0;                              )
    QUOTE(  }                                          );


static chaz_bool_t
S_probe_stdio64(stdio64_combo *combo) {
    char *output = NULL;
    size_t output_len;
    size_t needed = sizeof(stdio64_code)
                    + (2 * strlen(off64_type))
                    + strlen(combo->fopen_command)
                    + strlen(combo->ftell_command)
                    + strlen(combo->fseek_command)
                    + 20;
    char *code_buf = (char*)malloc(needed);
    chaz_bool_t success = false;

    /* Prepare the source code. */
    sprintf(code_buf, stdio64_code, combo->includes, off64_type,
            combo->fopen_command, off64_type, combo->ftell_command,
            combo->fseek_command);

    /* Verify compilation and that the offset type has 8 bytes. */
    output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
    if (output != NULL) {
        long size = strtol(output, NULL, 10);
        if (size == 8) {
            success = true;
        }
        free(output);
    }

    if (!Util_remove_and_verify("_charm_stdio64")) {
        Util_die("Failed to remove '_charm_stdio64'");
    }

    return success;
}

/* Code for checking 64-bit lseek. */
static const char lseek_code[] =
    QUOTE(  %s                                                        )
    QUOTE(  #include "_charm.h"                                       )
    QUOTE(  int main() {                                              )
    QUOTE(      int fd;                                               )
    QUOTE(      Charm_Setup;                                          )
    QUOTE(      fd = open("_charm_lseek", O_WRONLY | O_CREAT, 0666);  )
    QUOTE(      if (fd == -1) { return -1; }                          )
    QUOTE(      %s(fd, 0, SEEK_SET);                                  )
    QUOTE(      printf("%%d", 1);                                     )
    QUOTE(      if (close(fd)) { return -1; }                         )
    QUOTE(      return 0;                                             )
    QUOTE(  }                                                         );

static chaz_bool_t
S_probe_lseek(unbuff_combo *combo) {
    char *output = NULL;
    size_t output_len;
    size_t needed = sizeof(lseek_code)
                    + strlen(combo->includes)
                    + strlen(combo->lseek_command)
                    + 20;
    char *code_buf = (char*)malloc(needed);
    chaz_bool_t success = false;

    /* Verify compilation. */
    sprintf(code_buf, lseek_code, combo->includes, combo->lseek_command);
    output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
    if (output != NULL) {
        success = true;
        free(output);
    }

    if (!Util_remove_and_verify("_charm_lseek")) {
        Util_die("Failed to remove '_charm_lseek'");
    }

    free(code_buf);
    return success;
}

/* Code for checking 64-bit pread.  The pread call will fail, but that's fine
 * as long as it compiles. */
static const char pread64_code[] =
    QUOTE(  %s                                     )
    QUOTE(  #include "_charm.h"                    )
    QUOTE(  int main() {                           )
    QUOTE(      int fd = 20;                       )
    QUOTE(      char buf[1];                       )
    QUOTE(      Charm_Setup;                       )
    QUOTE(      printf("1");                       )
    QUOTE(      %s(fd, buf, 1, 1);                 )
    QUOTE(      return 0;                          )
    QUOTE(  }                                      );

static chaz_bool_t
S_probe_pread64(unbuff_combo *combo) {
    char *output = NULL;
    size_t output_len;
    size_t needed = sizeof(pread64_code)
                    + strlen(combo->includes)
                    + strlen(combo->pread64_command)
                    + 20;
    char *code_buf = (char*)malloc(needed);
    chaz_bool_t success = false;

    /* Verify compilation. */
    sprintf(code_buf, pread64_code, combo->includes, combo->pread64_command);
    output = CC_capture_output(code_buf, strlen(code_buf), &output_len);
    if (output != NULL) {
        success = true;
        free(output);
    }

    free(code_buf);
    return success;
}

