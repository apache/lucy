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

/* Charmonizer/Probe/LargeFiles.h
 */

#ifndef H_CHAZ_LARGE_FILES
#define H_CHAZ_LARGE_FILES

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* The LargeFiles module attempts to detect these symbols or alias them to
 * synonyms:
 *
 * off64_t
 * fopen64
 * ftello64
 * fseeko64
 *
 * If the attempt succeeds, this will be defined:
 *
 * HAS_LARGE_FILE_SUPPORT
 *
 * Additionally, 64-bit versions of lseek and pread may be detected/aliased:
 *
 * lseek64
 * pread64
 *
 * Use of the off64_t symbol may require sys/types.h.
 */
void chaz_LargeFiles_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define LargeFiles_run    chaz_LargeFiles_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_LARGE_FILES */


