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

#ifndef H_CFCVARIABLE
#define H_CFCVARIABLE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCVariable CFCVariable;
struct CFCParcel;
struct CFCType;

CFCVariable*
CFCVariable_new(struct CFCParcel *parcel, const char *exposure,
                const char *class_name, const char *class_cnick,
                const char *micro_sym, struct CFCType *type);

CFCVariable*
CFCVariable_init(CFCVariable *self, struct CFCParcel *parcel,
                 const char *exposure, const char *class_name,
                 const char *class_cnick, const char *micro_sym,
                 struct CFCType *type);

void
CFCVariable_destroy(CFCVariable *self);

int
CFCVariable_equals(CFCVariable *self, CFCVariable *other);

struct CFCType*
CFCVariable_get_type(CFCVariable *self);

const char*
CFCVariable_local_c(CFCVariable *self);

const char*
CFCVariable_global_c(CFCVariable *self);

const char*
CFCVariable_local_declaration(CFCVariable *self);

const char*
CFCVariable_micro_sym(CFCVariable *self);

const char*
CFCVariable_short_sym(CFCVariable *self);

const char*
CFCVariable_full_sym(CFCVariable *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCVARIABLE */

