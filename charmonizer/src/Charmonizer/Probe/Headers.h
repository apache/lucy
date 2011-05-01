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

/* Charmonizer/Probe/Headers.h
 */

#ifndef H_CHAZ_HEADERS
#define H_CHAZ_HEADERS

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "Charmonizer/Core/Defines.h"

/* Check whether a particular header file is available.  The test-compile is
 * only run the first time a given request is made.
 */
chaz_bool_t
chaz_Headers_check(const char *header_name);

/* Run the Headers module.
 *
 * Exported symbols:
 *
 * If HAS_C89 is declared, this system has all the header files described in
 * Ansi C 1989.  HAS_C90 is a synonym.  (It would be surprising if they are
 * not defined, because Charmonizer itself assumes C89.)
 *
 * HAS_C89
 * HAS_C90
 *
 * One symbol is exported for each C89 header file:
 *
 * HAS_ASSERT_H
 * HAS_CTYPE_H
 * HAS_ERRNO_H
 * HAS_FLOAT_H
 * HAS_LIMITS_H
 * HAS_LOCALE_H
 * HAS_MATH_H
 * HAS_SETJMP_H
 * HAS_SIGNAL_H
 * HAS_STDARG_H
 * HAS_STDDEF_H
 * HAS_STDIO_H
 * HAS_STDLIB_H
 * HAS_STRING_H
 * HAS_TIME_H
 *
 * One symbol is exported for every POSIX header present, and HAS_POSIX is
 * exported if they're all there.
 *
 * HAS_POSIX
 *
 * HAS_CPIO_H
 * HAS_DIRENT_H
 * HAS_FCNTL_H
 * HAS_GRP_H
 * HAS_PWD_H
 * HAS_SYS_STAT_H
 * HAS_SYS_TIMES_H
 * HAS_SYS_TYPES_H
 * HAS_SYS_UTSNAME_H
 * HAS_WAIT_H
 * HAS_TAR_H
 * HAS_TERMIOS_H
 * HAS_UNISTD_H
 * HAS_UTIME_H
 *
 * If pthread.h is available, this will be exported:
 *
 * HAS_PTHREAD_H
 */
void
chaz_Headers_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Headers_run        chaz_Headers_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_HEADERS */



