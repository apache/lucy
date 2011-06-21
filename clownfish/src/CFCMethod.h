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

#ifndef H_CFCMETHOD
#define H_CFCMETHOD

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCMethod CFCMethod;
struct CFCParcel;
struct CFCType;
struct CFCParamList;
struct CFCDocuComment;

CFCMethod*
CFCMethod_new(struct CFCParcel *parcel, const char *exposure,
              const char *class_name, const char *class_cnick,
              const char *macro_sym, struct CFCType *return_type,
              struct CFCParamList *param_list,
              struct CFCDocuComment *docucomment, int is_final,
              int is_abstract);

CFCMethod*
CFCMethod_init(CFCMethod *self, struct CFCParcel *parcel,
               const char *exposure, const char *class_name,
               const char *class_cnick, const char *macro_sym,
               struct CFCType *return_type, struct CFCParamList *param_list,
               struct CFCDocuComment *docucomment, int is_final,
               int is_abstract);

void
CFCMethod_destroy(CFCMethod *self);

int
CFCMethod_compatible(CFCMethod *self, CFCMethod *other);

void
CFCMethod_override(CFCMethod *self, CFCMethod *orig);

CFCMethod*
CFCMethod_finalize(CFCMethod *self);

/**
 * @return the number of bytes which the symbol would occupy.
 */
size_t
CFCMethod_short_method_sym(CFCMethod *self, const char *invoker, char *buf,
                           size_t buf_size);

/**
 * @return the number of bytes which the symbol would occupy.
 */
size_t
CFCMethod_full_method_sym(CFCMethod *self, const char *invoker, char *buf,
                          size_t buf_size);

/**
 * @return the number of bytes which the symbol would occupy.
 */
size_t
CFCMethod_full_offset_sym(CFCMethod *self, const char *invoker, char *buf,
                          size_t buf_size);

const char*
CFCMethod_get_macro_sym(CFCMethod *self);

const char*
CFCMethod_short_typedef(CFCMethod *self);

const char*
CFCMethod_full_typedef(CFCMethod *self);

const char*
CFCMethod_full_callback_sym(CFCMethod *self);

const char*
CFCMethod_full_override_sym(CFCMethod *self);

int
CFCMethod_final(CFCMethod *self);

int
CFCMethod_abstract(CFCMethod *self);

int
CFCMethod_novel(CFCMethod *self);

struct CFCType*
CFCMethod_self_type(CFCMethod *self);

struct CFCParcel*
CFCMethod_get_parcel(CFCMethod *self);

const char*
CFCMethod_get_prefix(CFCMethod *self);

const char*
CFCMethod_get_Prefix(CFCMethod *self);

const char*
CFCMethod_get_exposure(CFCMethod *self);

const char*
CFCMethod_get_class_name(CFCMethod *self);

const char*
CFCMethod_get_class_cnick(CFCMethod *self);

int
CFCMethod_public(CFCMethod *self);

struct CFCType*
CFCMethod_get_return_type(CFCMethod *self);

struct CFCParamList*
CFCMethod_get_param_list(CFCMethod *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCMETHOD */

