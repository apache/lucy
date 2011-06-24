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

#ifndef H_CFCBINDCLASS
#define H_CFCBINDCLASS

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCBindClass CFCBindClass;

struct CFCClass;

struct CFCBindClass*
CFCBindClass_new(struct CFCClass *client);

struct CFCBindClass*
CFCBindClass_init(struct CFCBindClass *self, struct CFCClass *client);

void
CFCBindClass_destroy(CFCBindClass *self);

char*
CFCBindClass_to_c(CFCBindClass *self);

char*
CFCBindClass_struct_definition(CFCBindClass *self);

char*
CFCBindClass_name_var_definition(CFCBindClass *self);

char*
CFCBindClass_vtable_definition(CFCBindClass *self);

struct CFCClass*
CFCBindClass_get_client(struct CFCBindClass *self);

const char*
CFCBindClass_full_callbacks_var(CFCBindClass *self);

const char*
CFCBindClass_full_name_var(CFCBindClass *self);

const char*
CFCBindClass_short_names_macro(CFCBindClass *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCBINDCLASS */

