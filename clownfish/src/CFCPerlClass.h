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

#ifndef H_CFCPERLCLASS
#define H_CFCPERLCLASS

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCPerlClass CFCPerlClass;
struct CFCParcel;
struct CFCClass;
struct CFCFunction;
struct CFCPerlPod;
struct CFCPerlMethod;
struct CFCPerlConstructor;

CFCPerlClass*
CFCPerlClass_new(struct CFCParcel *parcel, const char *class_name);

CFCPerlClass*
CFCPerlClass_init(CFCPerlClass *self, struct CFCParcel *parcel,
                  const char *class_name);

void
CFCPerlClass_destroy(CFCPerlClass *self);

void
CFCPerlClass_add_to_registry(CFCPerlClass *self);

CFCPerlClass*
CFCPerlClass_singleton(const char *class_name);

CFCPerlClass**
CFCPerlClass_registry();

void
CFCPerlClass_clear_registry(void);

void
CFCPerlClass_bind_method(CFCPerlClass *self, const char *alias,
                         const char *method);

void
CFCPerlClass_bind_constructor(CFCPerlClass *self, const char *alias,
                              const char *initializer);

/** Don't generate a binding for the specified method automatically.
 */
void
CFCPerlClass_exclude_method(CFCPerlClass *self, const char *method);

/** Don't generate a constructor named "new" from "init" automatically.
 */
void
CFCPerlClass_exclude_constructor(CFCPerlClass *self);

struct CFCPerlMethod**
CFCPerlClass_method_bindings(CFCPerlClass *self);

struct CFCPerlConstructor**
CFCPerlClass_constructor_bindings(CFCPerlClass *self);

char*
CFCPerlClass_create_pod(CFCPerlClass *self);

struct CFCClass*
CFCPerlClass_get_client(CFCPerlClass *self);

const char*
CFCPerlClass_get_class_name(CFCPerlClass *self);

/** Concatenate verbatim XS code.
 */
void
CFCPerlClass_append_xs(CFCPerlClass *self, const char *xs);

const char*
CFCPerlClass_get_xs_code(CFCPerlClass *self);

void
CFCPerlClass_set_pod_spec(CFCPerlClass *self, struct CFCPerlPod *pod_spec);

struct CFCPerlPod*
CFCPerlClass_get_pod_spec(CFCPerlClass *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLCLASS */

