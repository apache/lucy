#define CHAZ_USE_SHORT_NAMES

#include "charmony.h"

#ifdef HAS_SYS_TYPES_H
  #include <sys/types.h>
#endif

#include <stdio.h>
#include "Charmonizer/Test.h"
#include "Charmonizer/Test/AllTests.h"

TestBatch*
chaz_TLargeFiles_prepare()
{
    return Test_new_batch("LargeFiles", 10, chaz_TLargeFiles_run);
}

void
chaz_TLargeFiles_run(TestBatch *batch)
{
    FILE *fh;
    off64_t offset;
    int check_val;
    char check_char;

    /* a little over 4 GB, and a little over 2 GB */
    off64_t gb4_plus = ((off64_t)0x7FFFFFFF << 1) + 100;
    off64_t gb2_plus = (off64_t)0x7FFFFFFF + 200;
    
    /* gb4_plus modulo 4 GB (wrap is intentional) */
    i32_t wrap_gb4 = (i32_t)gb4_plus;

    ASSERT_INT_EQ(batch, sizeof(off64_t), 8, "off64_t type has 8 bytes");

#ifndef HAS_LARGE_FILE_SUPPORT
    SKIP_REMAINING(batch, "No large file support");
#endif
#ifndef CHAZ_HAS_SPARSE_FILES
    SKIP_REMAINING(batch, "Can't verify large file support without sparse files");
#endif
#ifndef CHAZ_CAN_CREATE_BIG_FILES
    SKIP_REMAINING(batch, "Unsafe to create 5GB sparse files on this system");
#endif

    fh = fopen64("_charm_large_file_test", "w+");
    if (fh == NULL)
        SKIP_REMAINING(batch, "Failed to open file");

    check_val = fseeko64(fh, gb4_plus, SEEK_SET);
    ASSERT_INT_EQ(batch, check_val, 0, "fseeko64 above 4 GB");

    offset = ftello64(fh);
    ASSERT_TRUE(batch, (offset == gb4_plus), "ftello64 above 4 GB");

    check_val = fprintf(fh, "X");
    ASSERT_INT_EQ(batch, check_val, 1, "print above 4 GB");

    check_val = fseeko64(fh, gb2_plus, SEEK_SET);
    ASSERT_INT_EQ(batch, check_val, 0, "fseeko64 above 2 GB");

    offset = ftello64(fh);
    ASSERT_TRUE(batch, (offset == gb2_plus), "ftello64 above 2 GB");

    check_val = fseeko64(fh, -1, SEEK_END);
    ASSERT_INT_EQ(batch, check_val, 0, "seek to near end");

    check_char = fgetc(fh);
    ASSERT_INT_EQ(batch, check_char, 'X', "read value after multiple seeks");

    fseeko64(fh, wrap_gb4, SEEK_SET);
    check_char = fgetc(fh);
    ASSERT_INT_EQ(batch, check_char, '\0', "No wraparound");

    check_val = fclose(fh);
    ASSERT_INT_EQ(batch, check_val, 0, "fclose succeeds after all that");

    /* truncate, just in case the call to remove fails */
    fh = fopen64("_charm_large_file_test", "w+");
    if (fh != NULL)
        fclose(fh);
    remove("_charm_large_file_test");
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

