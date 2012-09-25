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


/** Clownfish::CFC::Model::Version - Version number.
 *
 * A version number, comprised of one or more unsigned integers.  Digits
 * beyond the first are treated as implicit zeros for the purposes of
 * comparision, so that v1.2 and v1.2.0 are equivalent.
 */
#ifndef H_CFCVERSION
#define H_CFCVERSION

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct CFCVersion CFCVersion;

/**
 * @param vstring - A version string consisting of a lower-case 'v' followed
 * by one or more unsigned integers separated by dots.
*/
CFCVersion*
CFCVersion_new(const char *vstring);

CFCVersion*
CFCVersion_init(CFCVersion *self, const char *vstring);

void
CFCVersion_destroy(CFCVersion *self);

int
CFCVersion_compare_to(CFCVersion *self, CFCVersion *other);

const char*
CFCVersion_get_vstring(CFCVersion *self);

uint32_t
CFCVersion_get_major(CFCVersion *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCVERSION */

