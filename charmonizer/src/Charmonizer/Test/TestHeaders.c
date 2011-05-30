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
#include "Charmonizer/Test.h"

#ifdef HAS_ASSERT_H
  #include <assert.h>
#endif
#ifdef HAS_CTYPE_H
  #include <ctype.h>
#endif
#ifdef HAS_ERRNO_H
  #include <errno.h>
#endif
#ifdef HAS_FLOAT_H
  #include <float.h>
#endif
#ifdef HAS_LIMITS_H
  #include <limits.h>
#endif
#ifdef HAS_LOCALE_H
  #include <locale.h>
#endif
#ifdef HAS_MATH_H
  #include <math.h>
#endif
#ifdef HAS_SETJMP_H
  #include <setjmp.h>
#endif
#ifdef HAS_SIGNAL_H
  #include <signal.h>
#endif
#ifdef HAS_STDARG_H
  #include <stdarg.h>
#endif
#ifdef HAS_STDDEF_H
  #include <stddef.h>
#endif
#ifdef HAS_STDIO_H
  #include <stdio.h>
#endif
#ifdef HAS_STDLIB_H
  #include <stdlib.h>
#endif
#ifdef HAS_STRING_H
  #include <string.h>
#endif
#ifdef HAS_TIME_H
  #include <time.h>
#endif

#ifdef HAS_CPIO_H
  #include <cpio.h>
#endif
#ifdef HAS_DIRENT_H
  #include <dirent.h>
#endif
#ifdef HAS_FCNTL_H
  #include <fcntl.h>
#endif
#ifdef HAS_GRP_H
  #include <grp.h>
#endif
#ifdef HAS_PWD_H
  #include <pwd.h>
#endif
#ifdef HAS_SYS_STAT_H
  #include <sys/stat.h>
#endif
#ifdef HAS_SYS_TIMES_H
  #include <sys/times.h>
#endif
#ifdef HAS_SYS_TYPES_H
  #include <sys/types.h>
#endif
#ifdef HAS_SYS_UTSNAME_H
  #include <sys/utsname.h>
#endif
#ifdef HAS_WAIT_H
  #include <wait.h>
#endif
#ifdef HAS_TAR_H
  #include <tar.h>
#endif
#ifdef HAS_TERMIOS_H
  #include <termios.h>
#endif
#ifdef HAS_UNISTD_H
  #include <unistd.h>
#endif
#ifdef HAS_UTIME_H
  #include <utime.h>
#endif

#if defined(HAS_C89) || defined(HAS_C90)
  #include <assert.h>
  #include <ctype.h>
  #include <errno.h>
  #include <float.h>
  #include <limits.h>
  #include <locale.h>
  #include <math.h>
  #include <setjmp.h>
  #include <signal.h>
  #include <stdarg.h>
  #include <stddef.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>
#endif

#ifdef HAS_POSIX
  #include <cpio.h>
  #include <dirent.h>
  #include <fcntl.h>
  #include <grp.h>
  #include <pwd.h>
  #include <sys/stat.h>
  #include <sys/times.h>
  #include <sys/types.h>
  #include <sys/utsname.h>
  #include <sys/wait.h>
  #include <tar.h>
  #include <termios.h>
  #include <unistd.h>
  #include <utime.h>
#endif

static void
S_run_tests(TestBatch *batch) {
    PASS(batch, "Compiled successfully with all detected headers");

    /* Don't bother checking all -- just use stdio as an example. */
#ifdef HAS_STDIO_H
    PASS(batch, "stdio.h should have been detected");
#else
    FAIL(batch, "stdio.h should have been detected");
#endif
}


int main(int argc, char **argv) {
    TestBatch *batch;

    Test_init();
    batch = Test_new_batch("Headers", 2, S_run_tests);
    batch->run_test(batch);
    batch->destroy(batch);
    return 0;
}
