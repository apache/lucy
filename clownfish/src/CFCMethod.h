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

typedef struct CFCMethod CFCMethod;

CFCMethod*
CFCMethod_new(void *parcel, const char *exposure, const char *class_name, 
              const char *class_cnick, const char *micro_sym, 
              void *return_type, void *param_list, void *docucomment, 
              int is_inline, const char *macro_sym, int is_final, 
              int is_abstract);

CFCMethod*
CFCMethod_init(CFCMethod *self, void *parcel, const char *exposure, 
               const char *class_name, const char *class_cnick, 
               const char *micro_sym, void *return_type, void *param_list, 
               void *docucomment, int is_inline, const char *macro_sym, 
               int is_final, int is_abstract);

void
CFCMethod_destroy(CFCMethod *self);


const char*
CFCMethod_get_macro_sym(CFCMethod *self);

void
CFCMethod_set_short_typedef(CFCMethod *self, const char *short_typedef);

const char*
CFCMethod_short_typedef(CFCMethod *self);

int
CFCMethod_final(CFCMethod *self);

int
CFCMethod_abstract(CFCMethod *self);

void
CFCMethod_set_novel(CFCMethod *self, int is_novel);

int
CFCMethod_novel(CFCMethod *self);

#endif /* H_CFCMETHOD */

