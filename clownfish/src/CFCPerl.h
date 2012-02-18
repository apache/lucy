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

#ifndef H_CFCPERL
#define H_CFCPERL

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCPerl CFCPerl;
struct CFCParcel;
struct CFCHierarchy;

CFCPerl*
CFCPerl_new(struct CFCParcel *parcel, struct CFCHierarchy *hierarchy,
            const char *lib_dir, const char *boot_class, const char *header,
            const char *footer);

CFCPerl*
CFCPerl_init(CFCPerl *self, struct CFCParcel *parcel,
             struct CFCHierarchy *hierarchy, const char *lib_dir,
             const char *boot_class, const char *header, const char *footer);

void
CFCPerl_destroy(CFCPerl *self);

struct CFCParcel*
CFCPerl_get_parcel(CFCPerl *self);

struct CFCHierarchy*
CFCPerl_get_hierarchy(CFCPerl *self);

const char*
CFCPerl_get_lib_dir(CFCPerl *self);

const char*
CFCPerl_get_boot_class(CFCPerl *self);

const char*
CFCPerl_get_header(CFCPerl *self);

const char*
CFCPerl_get_footer(CFCPerl *self);

const char*
CFCPerl_get_xs_path(CFCPerl *self);

const char*
CFCPerl_get_pm_path(CFCPerl *self);

const char*
CFCPerl_get_boot_h_file(CFCPerl *self);

const char*
CFCPerl_get_boot_c_file(CFCPerl *self);

const char*
CFCPerl_get_boot_h_path(CFCPerl *self);

const char*
CFCPerl_get_boot_c_path(CFCPerl *self);

const char*
CFCPerl_get_boot_func(CFCPerl *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERL */

