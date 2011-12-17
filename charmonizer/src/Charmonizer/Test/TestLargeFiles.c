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
#include <errno.h>

#ifdef CHAZ_HAS_SYS_TYPES_H
  #include <sys/types.h>
#endif
#ifdef CHAZ_HAS_FCNTL_H
  #include <fcntl.h> /* open, fcntl flags */
#endif
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h> /* close */
#endif
#ifdef CHAZ_HAS_IO_H
  #include <io.h>
#endif

#include <stdio.h>
#include "Charmonizer/Test.h"

#if (defined(HAS_64BIT_STDIO) \
     && defined(CHAZ_HAS_STAT_ST_SIZE) \
     && defined(CHAZ_HAS_STAT_ST_BLOCKS))
#define STAT_TESTS_ENABLED

#include <sys/stat.h>

/* Determine whether we can use sparse files.
 */
static chaz_bool_t
S_check_sparse_files(void);

/* Helper for check_sparse_files().
 */
static chaz_bool_t
S_test_sparse_file(long offset, struct stat *st);

/* See if trying to write a 5 GB file in a subprocess bombs out.  If it
 * doesn't, then the test suite can safely verify large file support.
 */
static chaz_bool_t
S_can_create_big_files(void);

#endif /* criteria for defining STAT_TESTS_ENABLED */

#ifdef O_LARGEFILE
  #define LARGEFILE_OPEN_FLAG O_LARGEFILE
#else
  #define LARGEFILE_OPEN_FLAG 0
#endif

static void
S_run_tests(void) {
    FILE *fh;
    off64_t offset;
    int check_val;
    char check_char;
    int fd;

    /* A little over 4 GB, and a little over 2 GB. */
    off64_t gb4_plus = ((off64_t)0x7FFFFFFF << 1) + 100;
    off64_t gb2_plus = (off64_t)0x7FFFFFFF + 200;

    LONG_EQ(sizeof(off64_t), 8, "off64_t type has 8 bytes");

#ifndef HAS_64BIT_STDIO
    SKIP_REMAINING("No stdio large file support");
    return;
#endif
#ifndef STAT_TESTS_ENABLED
    SKIP_REMAINING("Need stat with st_size and st_blocks");
    return;
#else
    /* Check for sparse files. */
    if (!S_check_sparse_files()) {
        SKIP_REMAINING("Can't verify large file support "
                       "without sparse files");
        return;
    }
    if (!S_can_create_big_files()) {
        SKIP_REMAINING("Unsafe to create 5GB sparse files on this system");
        return;
    }

    fh = fopen64("_charm_large_file_test", "w+");
    if (fh == NULL) {
        SKIP_REMAINING("Failed to open file");
        return;
    }

    check_val = fseeko64(fh, gb4_plus, SEEK_SET);
    LONG_EQ(check_val, 0, "fseeko64 above 4 GB");

    offset = ftello64(fh);
    OK((offset == gb4_plus), "ftello64 above 4 GB");

    check_val = fprintf(fh, "X");
    LONG_EQ(check_val, 1, "print above 4 GB");

    check_val = fseeko64(fh, gb2_plus, SEEK_SET);
    LONG_EQ(check_val, 0, "fseeko64 above 2 GB");

    offset = ftello64(fh);
    OK((offset == gb2_plus), "ftello64 above 2 GB");

    check_val = fseeko64(fh, -1, SEEK_END);
    LONG_EQ(check_val, 0, "seek to near end");

    check_char = fgetc(fh);
    LONG_EQ(check_char, 'X', "read value after multiple seeks");

    check_val = fclose(fh);
    LONG_EQ(check_val, 0, "fclose succeeds after all that");

    /* Truncate, just in case the call to remove fails. */
    fh = fopen64("_charm_large_file_test", "w+");
    if (fh != NULL) {
        fclose(fh);
    }
    remove("_charm_large_file_test");

#ifndef HAS_64BIT_LSEEK
    SKIP_REMAINING("No 64-bit lseek");
    return;
#else
    fd = open("_charm_large_file_test",
              O_RDWR | O_CREAT | LARGEFILE_OPEN_FLAG, 0666);
    if (fd == -1) {
        FAIL("open failed");
        SKIP_REMAINING("open failed");
        return;
    }

    offset = lseek64(fd, gb4_plus, SEEK_SET);
    OK(offset == gb4_plus, "lseek64 above 4 GB");

    offset = lseek64(fd, 0, SEEK_CUR);
    OK(offset == gb4_plus, "lseek64 in place above 4 GB");

    check_val = write(fd, "X", 1);
    LONG_EQ(check_val, 1, "write() above 4 GB");

    offset = lseek64(fd, gb2_plus, SEEK_SET);
    OK(offset == gb2_plus, "lseek64 above 2 GB");

    offset = lseek64(fd, 0, SEEK_CUR);
    OK((offset == gb2_plus), "lseek64 in place above 2 GB");

    offset = lseek64(fd, -1, SEEK_END);
    OK(offset == gb4_plus, "seek to near end");

    check_val = read(fd, &check_char, 1);
    LONG_EQ(check_val, 1, "read() after multiple lseek64 calls");
    LONG_EQ(check_char, 'X',
            "read() correct data after multiple lseek64 calls");
#ifdef HAS_64BIT_PREAD
    check_char = 0;
    check_val = pread64(fd, &check_char, 1, gb4_plus);
    LONG_EQ(check_val, 1, "pread64");
    LONG_EQ(check_char, 'X', "pread64() correct data");
#else
    SKIP("no pread64");
    SKIP("no pread64");
#endif

    check_val = close(fd);
    LONG_EQ(check_val, 0, "close succeeds after all that");
#endif

    /* Truncate, just in case the call to remove fails. */
    fh = fopen64("_charm_large_file_test", "w+");
    if (fh != NULL) {
        fclose(fh);
    }
    remove("_charm_large_file_test");
#endif /* STAT_TESTS_ENABLED */
}

#ifdef STAT_TESTS_ENABLED

static chaz_bool_t
S_check_sparse_files(void) {
    struct stat st_a, st_b;

    /* Write and stat a 1 MB file and a 2 MB file, both of them sparse. */
    if (!S_test_sparse_file(1000000, &st_a)) { return false; }
    if (!S_test_sparse_file(2000000, &st_b)) { return false; }
    if (st_a.st_size != 1000001 || st_b.st_size != 2000001) {
        return false;
    }

    /* See if two files with very different lengths have the same block size. */
    return st_a.st_blocks == st_b.st_blocks ? true : false;
}

static chaz_bool_t
S_test_sparse_file(long offset, struct stat *st) {
    FILE *sparse_fh;
    chaz_bool_t result = false;

    /* Make sure the file's not there, then open. */
    remove("_charm_sparse");
    if ((sparse_fh = fopen64("_charm_sparse", "w+")) == NULL) {
        return false;
    }

    /* Seek fh to [offset], write a byte, close file. */
    if ((fseeko64(sparse_fh, offset, SEEK_SET)) != -1) {
        if ((fprintf(sparse_fh, "X")) == 1) {
            result = true;
        }
    }
    if (fclose(sparse_fh)) {
        result = false;
    }

    /* Stat the file. */
    stat("_charm_sparse", st);

    remove("_charm_sparse");
    return true;
}

static chaz_bool_t
S_can_create_big_files(void) {
    chy_bool_t result = 0;
    FILE *fh = fopen64("_charm_large_file_test", "w+");
    if (!fh) {
        return false;
    }
    else {
        /* Bail unless seek succeeds. */
        int64_t check_seek = fseeko64(fh, I64_C(5000000000), SEEK_SET);
        if (check_seek != -1) {
            /* Bail unless we write successfully. */
            if (fprintf(fh, "X") == 1) {
                result = true;
            }
        }
        if (fclose(fh)) {
            result = false;
        }
    }

    /* Truncate, just in case the call to remove fails. */
    fh = fopen64("_charm_large_file_test", "w");
    if (fh != NULL) {
        fclose(fh);
    }
    remove("_charm_large_file_test");

    return result;
}

#endif /* STAT_TESTS_ENABLED */

int main(int argc, char **argv) {
    Test_start(20);
    S_run_tests();
    return !Test_finish();
}

