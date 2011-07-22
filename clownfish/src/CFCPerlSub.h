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

#ifndef H_CFCPERLSUB
#define H_CFCPERLSUB

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCPerlSub CFCPerlSub;
struct CFCParamList;

CFCPerlSub*
CFCPerlSub_new(const char *klass, struct CFCParamList *param_list,
               const char *class_name, const char *alias,
               int use_labeled_params);

CFCPerlSub*
CFCPerlSub_init(CFCPerlSub *self, struct CFCParamList *param_list,
                const char *class_name, const char *alias,
                int use_labeled_params);

void
CFCPerlSub_destroy(CFCPerlSub *self);

char*
CFCPerlSub_params_hash_def(CFCPerlSub *self);

struct CFCParamList*
CFCPerlSub_get_param_list(CFCPerlSub *self);

const char*
CFCPerlSub_get_class_name(CFCPerlSub *self);

int
CFCPerlSub_use_labeled_params(CFCPerlSub *self);

const char*
CFCPerlSub_perl_name(CFCPerlSub *self);

const char*
CFCPerlSub_c_name(CFCPerlSub *self);

const char*
CFCPerlSub_c_name_list(CFCPerlSub *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLSUB */

