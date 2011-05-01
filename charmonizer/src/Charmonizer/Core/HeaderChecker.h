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

/* Charmonizer/Probe/HeaderChecker.h
 */

#ifndef H_CHAZ_HEAD_CHECK
#define H_CHAZ_HEAD_CHECK

#ifdef __cplusplus
extern "C" {
#endif

#include "Charmonizer/Core/Defines.h"

/* Bootstrap the HeadCheck.  Call this before anything else.
 */
void
chaz_HeadCheck_init();

/* Check for a particular header and return true if it's available.  The
 * test-compile is only run the first time a given request is made.
 */
chaz_bool_t
chaz_HeadCheck_check_header(const char *header_name);

/* Attempt to compile a file which pulls in all the headers specified by name
 * in a null-terminated array.  If the compile succeeds, add them all to the
 * internal register and return true.
 */
chaz_bool_t
chaz_HeadCheck_check_many_headers(const char **header_names);

/* Return true if the member is present in the struct. */
chaz_bool_t
chaz_HeadCheck_contains_member(const char *struct_name, const char *member,
                               const char *includes);

#ifdef CHAZ_USE_SHORT_NAMES
  #define HeadCheck_init                    chaz_HeadCheck_init
  #define HeadCheck_contains_member         chaz_HeadCheck_contains_member
  #define HeadCheck_check_header            chaz_HeadCheck_check_header
  #define HeadCheck_check_many_headers      chaz_HeadCheck_check_many_headers
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_HEAD_CHECK */


