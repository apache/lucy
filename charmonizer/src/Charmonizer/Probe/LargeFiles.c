#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/HeadCheck.h"
#include "Charmonizer/Core/ModHandler.h"
#include "Charmonizer/Core/Stat.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/LargeFiles.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* sets of symbols which might provide large file support */
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
    { "",                         "fopen",     "_ftelli64", "_fseeki64", "__int64" },
    { "",                         "fopen",     "ftell",     "fseek",     "long"    },
    { NULL, NULL, NULL, NULL, NULL }
};
typedef struct unbuff_combo {
    const char *includes;
    const char *open_command;
    const char *tell_command;
    const char *seek_command;
} unbuff_combo;

/* Check what name 64-bit ftell, fseek go by.
 */
static chaz_bool_t
S_probe_off64(off64_combo *combo);

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

/* vars for holding lfs commands, once they're discovered */
static char fopen_command[10];
static char fseek_command[10];
static char ftell_command[10];
static char off64_type[10];

void
LargeFiles_run(void) 
{
    chaz_bool_t success = false;
    unsigned i;

    START_RUN("LargeFiles");

    /* see if off64_t and friends exist or have synonyms */
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

    /* write the affirmations/definitions */
    if (success) {
        ModHand_append_conf("#define CHY_HAS_LARGE_FILE_SUPPORT\n");
        /* alias these only if they're not already provided and correct */
        if (strcmp(off64_type, "off64_t") != 0) {
            ModHand_append_conf("#define chy_off64_t %s\n",  off64_type);
            ModHand_append_conf("#define chy_fopen64 %s\n",  fopen_command);
            ModHand_append_conf("#define chy_ftello64 %s\n", ftell_command);
            ModHand_append_conf("#define chy_fseeko64 %s\n", fseek_command);
        }
    }

    /* check for sparse files */
    if (S_check_sparse_files()) {
        ModHand_append_conf("#define CHAZ_HAS_SPARSE_FILES\n");
        /* see if we can create a 5 GB file without crashing */
        if (success && S_can_create_big_files())
            ModHand_append_conf("#define CHAZ_CAN_CREATE_BIG_FILES\n");
    }
    else {
        ModHand_append_conf("#define CHAZ_NO_SPARSE_FILES\n");
    }

    /* test for unbuffered LFS commands */
    if (success) {

    }

    /* short names */
    if (success) {
        START_SHORT_NAMES;
        ModHand_shorten_macro("HAS_LARGE_FILE_SUPPORT");

        /* alias these only if they're not already provided and correct */
        if (strcmp(off64_type, "off64_t") != 0) {
            ModHand_shorten_typedef("off64_t");
            ModHand_shorten_function("fopen64");
            ModHand_shorten_function("ftello64");
            ModHand_shorten_function("fseeko64");
        }
        END_SHORT_NAMES;
    }
    
    END_RUN;
}

/* code for checking ftello64 and friends */
static char off64_code[] = METAQUOTE
    %s
    #include "_charm.h"
    int main() {
        %s pos;
        FILE *f;
        Charm_Setup;
        f = %s("_charm_off64", "w");
        if (f == NULL) return -1;
        printf("%%d", (int)sizeof(%s));
        pos = %s(stdout);
        %s(stdout, 0, SEEK_SET);
        return 0;
    }
METAQUOTE;


static chaz_bool_t
S_probe_off64(off64_combo *combo)
{
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

    /* verify compilation and that the offset type has 8 bytes */
    output = ModHand_capture_output(code_buf, strlen(code_buf), 
        &output_len);
    if (output != NULL) {
        long size = strtol(output, NULL, 10);
        if (size == 8)
            success = true;
        free(output);
    }

    return success;
}

static chaz_bool_t 
S_check_sparse_files()
{
    Stat st_a, st_b;

    /* bail out if we can't stat() a file */
    if (!HeadCheck_check_header("sys/stat.h"))
        return false;

    /* write and stat a 1 MB file and a 2 MB file, both of them sparse */
    S_test_sparse_file(1000000, &st_a);
    S_test_sparse_file(2000000, &st_b);
    if (!(st_a.valid && st_b.valid))
        return false;
    if (st_a.size != 1000001)
        Util_die("Expected size of 1000001 but got %ld", (long)st_a.size);
    if (st_b.size != 2000001)
        Util_die("Expected size of 2000001 but got %ld", (long)st_b.size);

    /* see if two files with very different lengths have the same block size */
    if (st_a.blocks == st_b.blocks)
        return true;
    else
        return false;
} 

static void
S_test_sparse_file(long offset, Stat *st)
{
    FILE *sparse_fh;

    /* make sure the file's not there, then open */
    Util_remove_and_verify("_charm_sparse");
    if ( (sparse_fh = fopen("_charm_sparse", "w+")) == NULL )
        Util_die("Couldn't open file '_charm_sparse'");

    /* seek fh to [offset], write a byte, close file */
    if ( (fseek(sparse_fh, offset, SEEK_SET)) == -1)
        Util_die("seek failed: %s", strerror(errno));
    if ( (fprintf(sparse_fh, "X")) != 1 )
        Util_die("fprintf failed");
    if (fclose(sparse_fh))
        Util_die("Error closing file '_charm_sparse': %s", strerror(errno));

    /* stat the file */
    Stat_stat("_charm_sparse", st);

    remove("_charm_sparse");
}

/* open a file, seek to a loc, print a char, and communicate success */
static char create_bigfile_code_a[] = METAQUOTE
    #include "_charm.h"
    int main() {
        FILE *fh = fopen("_charm_large_file_test", "w+");
        int check_seek;
        Charm_Setup;
        /* bail unless seek succeeds */
        check_seek = 
METAQUOTE;

static char create_bigfile_code_b[] = METAQUOTE
            (fh, 5000000000, SEEK_SET);
        if (check_seek == -1)
            exit(1);
        /* bail unless we write successfully */
        if (fprintf(fh, "X") != 1)
            exit(1);
        if (fclose(fh))
            exit(1);
        /* communicate success to Charmonizer */
        printf("1");
        return 0;
    }
METAQUOTE;

static chaz_bool_t
S_can_create_big_files()
{
    char *output;
    size_t output_len;
    FILE *truncating_fh;
    size_t needed = strlen(create_bigfile_code_a)
                  + strlen(fseek_command)
                  + strlen(create_bigfile_code_b)
                  + 10;
    char *code_buf = (char*)malloc(needed);

    /* concat the source strings, compile the file, capture output */
    sprintf(code_buf, "%s%s%s", create_bigfile_code_a, fseek_command, 
        create_bigfile_code_b);
    output = ModHand_capture_output(code_buf, strlen(code_buf), &output_len);

    /* truncate, just in case the call to remove fails */
    truncating_fh = fopen("_charm_large_file_test", "w");
    if (truncating_fh != NULL)
        fclose(truncating_fh);
    Util_remove_and_verify("_charm_large_file_test");

    /* return true if the test app made it to the finish line */
    free(code_buf);
    return output == NULL ? false : true;
}


/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

