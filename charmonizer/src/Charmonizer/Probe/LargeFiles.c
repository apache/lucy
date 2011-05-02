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
#include "Charmonizer/Core/Stat.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/LargeFiles.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Sets of symbols which might provide large file support. */
typedef struct off64_combo {
    const char *includes;
    const char *fopen_command;
    const char *ftell_command;
    const char *fseek_command;
    const char *offset64_type;
} off64_combo;
static off64_combo off64_combos[] = {
    { "#include <sys/types.h>\n", "fopen64",   "ftello64",  "fseeko64",  "off64_t" },
    { "#include <sys/types.h>\n", "fopen",     "ftello64",  "fseeko64",  "off64_t" },
    { "#include <sys/types.h>\n", "fopen",     "ftello",    "fseeko",    "off_t"   },
    { "",                         "fopen",     "ftell",     "fseek",     "off_t"   },
    { "",                         "fopen",     "_ftelli64", "_fseeki64", "__int64" },
    { "",                         "fopen",     "ftell",     "fseek",     "long"    },
    { NULL, NULL, NULL, NULL, NULL }
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

/* Check what name 64-bit ftell, fseek go by.
 */
static chaz_bool_t
S_probe_off64(off64_combo *combo);

/* Check for a 64-bit lseek.
 */
static chaz_bool_t
S_probe_lseek(unbuff_combo *combo);

/* Check for a 64-bit pread.
 */
static chaz_bool_t
S_probe_pread64(unbuff_combo *combo);

/* Determine whether we can use sparse files.
 */
static chaz_bool_t
S_check_sparse_files();

/* Helper for check_sparse_files().
 */
static void
S_test_sparse_file(long offset, Stat *st);

/* See if trying to write a 5 GB file in a subprocess bombs out.  If it
 * doesn't, then the test suite can safely verify large file support.
 */
static chaz_bool_t
S_can_create_big_files();

/* Vars for holding lfs commands, once they're discovered. */
static char fopen_command[10];
static char fseek_command[10];
static char ftell_command[10];
static char lseek_command[10];
static char pread64_command[10];
static char off64_type[10];

void
LargeFiles_run(void) {
    chaz_bool_t success = false;
    chaz_bool_t found_lseek = false;
    chaz_bool_t found_pread64 = false;
    unsigned i;

    ConfWriter_start_module("LargeFiles");

    /* See if off64_t and friends exist or have synonyms. */
    for (i = 0; off64_combos[i].includes != NULL; i++) {
        off64_combo combo = off64_combos[i];
        success = S_probe_off64(&combo);
        if (success) {
            strcpy(fopen_command, combo.fopen_command);
            strcpy(fseek_command, combo.fseek_command);
            strcpy(ftell_command, combo.ftell_command);
            strcpy(off64_type, combo.offset64_type);
            break;
        }
    }

    /* Write the affirmations/definitions. */
    if (success) {
        ConfWriter_append_conf("#define CHY_HAS_LARGE_FILE_SUPPORT\n");
        /* Alias these only if they're not already provided and correct. */
        if (strcmp(off64_type, "off64_t") != 0) {
            ConfWriter_append_conf("#define chy_off64_t %s\n",  off64_type);
            ConfWriter_append_conf("#define chy_fopen64 %s\n",  fopen_command);
            ConfWriter_append_conf("#define chy_ftello64 %s\n", ftell_command);
            ConfWriter_append_conf("#define chy_fseeko64 %s\n", fseek_command);
        }
    }

    /* Probe for 64-bit versions of lseek and pread (if we have an off64_t). */
    if (success) {
        for (i = 0; unbuff_combos[i].lseek_command != NULL; i++) {
            unbuff_combo combo = unbuff_combos[i];
            found_lseek = S_probe_lseek(&combo);
            if (found_lseek) {
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
                strcpy(pread64_command, combo.pread64_command);
                ConfWriter_append_conf("#define chy_pread64 %s\n",
                                       pread64_command);
                found_pread64 = true;
                break;
            }
        }
    }

    /* Check for sparse files. */
    if (S_check_sparse_files()) {
        ConfWriter_append_conf("#define CHAZ_HAS_SPARSE_FILES\n");
        /* See if we can create a 5 GB file without crashing. */
        if (success && S_can_create_big_files()) {
            ConfWriter_append_conf("#define CHAZ_CAN_CREATE_BIG_FILES\n");
        }
    }
    else {
        ConfWriter_append_conf("#define CHAZ_NO_SPARSE_FILES\n");
    }

    /* Short names. */
    if (success) {
        ConfWriter_start_short_names();
        ConfWriter_shorten_macro("HAS_LARGE_FILE_SUPPORT");

        /* Alias these only if they're not already provided and correct. */
        if (strcmp(off64_type, "off64_t") != 0) {
            ConfWriter_shorten_typedef("off64_t");
            ConfWriter_shorten_function("fopen64");
            ConfWriter_shorten_function("ftello64");
            ConfWriter_shorten_function("fseeko64");
        }
        if (found_lseek && strcmp(lseek_command, "lseek64") != 0) {
            ConfWriter_shorten_function("lseek64");
        }
        if (found_pread64 && strcmp(pread64_command, "pread64") != 0) {
            ConfWriter_shorten_function("pread64");
        }
        ConfWriter_end_short_names();
    }

    ConfWriter_end_module();
}

/* Code for checking ftello64 and friends. */
static char off64_code[] =
    QUOTE(  %s                                         )
    QUOTE(  #include "_charm.h"                        )
    QUOTE(  int main() {                               )
    QUOTE(      %s pos;                                )
    QUOTE(      FILE *f;                               )
    QUOTE(      Charm_Setup;                           )
    QUOTE(      f = %s("_charm_off64", "w");           )
    QUOTE(      if (f == NULL) return -1;              )
    QUOTE(      printf("%%d", (int)sizeof(%s));        )
    QUOTE(      pos = %s(stdout);                      )
    QUOTE(      %s(stdout, 0, SEEK_SET);               )
    QUOTE(      return 0;                              )
    QUOTE(  }                                          );


static chaz_bool_t
S_probe_off64(off64_combo *combo) {
    char *output = NULL;
    size_t output_len;
    size_t needed = sizeof(off64_code)
                    + (2 * strlen(combo->offset64_type))
                    + strlen(combo->fopen_command)
                    + strlen(combo->ftell_command)
                    + strlen(combo->fseek_command)
                    + 20;
    char *code_buf = (char*)malloc(needed);
    chaz_bool_t success = false;

    /* Prepare the source code. */
    sprintf(code_buf, off64_code, combo->includes, combo->offset64_type,
            combo->fopen_command, combo->offset64_type, combo->ftell_command,
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

    if (!Util_remove_and_verify("_charm_off64")) {
        Util_die("Failed to remove '_charm_off64'");
    }

    return success;
}

/* Code for checking 64-bit lseek. */
static char lseek_code[] =
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
static char pread64_code[] =
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

static chaz_bool_t
S_check_sparse_files() {
    Stat st_a, st_b;

    /* Bail out if we can't stat() a file. */
    if (!HeadCheck_check_header("sys/stat.h")) {
        return false;
    }

    /* Write and stat a 1 MB file and a 2 MB file, both of them sparse. */
    S_test_sparse_file(1000000, &st_a);
    S_test_sparse_file(2000000, &st_b);
    if (!(st_a.valid && st_b.valid)) {
        return false;
    }
    if (st_a.size != 1000001) {
        Util_die("Expected size of 1000001 but got %ld", (long)st_a.size);
    }
    if (st_b.size != 2000001) {
        Util_die("Expected size of 2000001 but got %ld", (long)st_b.size);
    }

    /* See if two files with very different lengths have the same block size. */
    if (st_a.blocks == st_b.blocks) {
        return true;
    }
    else {
        return false;
    }
}

static void
S_test_sparse_file(long offset, Stat *st) {
    FILE *sparse_fh;

    /* Make sure the file's not there, then open. */
    Util_remove_and_verify("_charm_sparse");
    if ((sparse_fh = fopen("_charm_sparse", "w+")) == NULL) {
        Util_die("Couldn't open file '_charm_sparse'");
    }

    /* Seek fh to [offset], write a byte, close file. */
    if ((fseek(sparse_fh, offset, SEEK_SET)) == -1) {
        Util_die("seek failed: %s", strerror(errno));
    }
    if ((fprintf(sparse_fh, "X")) != 1) {
        Util_die("fprintf failed");
    }
    if (fclose(sparse_fh)) {
        Util_die("Error closing file '_charm_sparse': %s", strerror(errno));
    }

    /* Stat the file. */
    Stat_stat("_charm_sparse", st);

    remove("_charm_sparse");
}

/* Open a file, seek to a loc, print a char, and communicate success. */
static char create_bigfile_code[] =
    QUOTE(  #include "_charm.h"                                      )
    QUOTE(  int main() {                                             )
    QUOTE(      FILE *fh = fopen("_charm_large_file_test", "w+");    )
    QUOTE(      int check_seek;                                      )
    QUOTE(      Charm_Setup;                                         )
    /* Bail unless seek succeeds. */
    QUOTE(      check_seek = %s(fh, 5000000000, SEEK_SET);           )
    QUOTE(      if (check_seek == -1)                                )
    QUOTE(          exit(1);                                         )
    /* Bail unless we write successfully. */
    QUOTE(      if (fprintf(fh, "X") != 1)                           )
    QUOTE(          exit(1);                                         )
    QUOTE(      if (fclose(fh))                                      )
    QUOTE(          exit(1);                                         )
    /* Communicate success to Charmonizer. */
    QUOTE(      printf("1");                                         )
    QUOTE(      return 0;                                            )
    QUOTE(  }                                                        );

static chaz_bool_t
S_can_create_big_files() {
    char *output;
    size_t output_len;
    FILE *truncating_fh;
    size_t needed = strlen(create_bigfile_code)
                    + strlen(fseek_command)
                    + 10;
    char *code_buf = (char*)malloc(needed);

    /* Concat the source strings, compile the file, capture output. */
    sprintf(code_buf, create_bigfile_code, fseek_command);
    output = CC_capture_output(code_buf, strlen(code_buf), &output_len);

    /* Truncate, just in case the call to remove fails. */
    truncating_fh = fopen("_charm_large_file_test", "w");
    if (truncating_fh != NULL) {
        fclose(truncating_fh);
    }
    Util_remove_and_verify("_charm_large_file_test");

    /* Return true if the test app made it to the finish line. */
    free(code_buf);
    return output == NULL ? false : true;
}



