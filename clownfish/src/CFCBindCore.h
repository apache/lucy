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

#ifndef H_CFCBINDCORE
#define H_CFCBINDCORE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCBindCore CFCBindCore;
struct CFCHierarchy;

CFCBindCore*
CFCBindCore_new(struct CFCHierarchy *hierarchy, const char *dest,
                 const char *header, const char *footer);

CFCBindCore*
CFCBindCore_init(CFCBindCore *self, struct CFCHierarchy *hierarchy,
                 const char *dest, const char *header, const char *footer);

void
CFCBindCore_destroy(CFCBindCore *self);

int
CFCBindCore_write_all_modified(CFCBindCore *self, int modified);

char*
CFCBindCore_write_parcel_h(CFCBindCore *self);

char*
CFCBindCore_write_parcel_c(CFCBindCore *self);

struct CFCHierarchy*
CFCBindCore_get_hierarchy(CFCBindCore *self);

const char*
CFCBindCore_get_dest(CFCBindCore *self);

const char*
CFCBindCore_get_header(CFCBindCore *self);

const char*
CFCBindCore_get_footer(CFCBindCore *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCBINDCORE */


