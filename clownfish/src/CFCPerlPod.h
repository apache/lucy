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

#ifndef H_CFCPERLPOD
#define H_CFCPERLPOD

#ifdef __cplusplus
extern "C" {
#endif

/** Spec for generating POD for a single class.
 */

typedef struct CFCPerlPod CFCPerlPod;
struct CFCFunction;
struct CFCClass;

CFCPerlPod*
CFCPerlPod_new(void);

CFCPerlPod*
CFCPerlPod_init(CFCPerlPod *self);

void
CFCPerlPod_destroy(CFCPerlPod *self);

char*
CFCPerlPod_perlify_doc_text(CFCPerlPod *self, const char *source);

char*
CFCPerlPod_gen_subroutine_pod(CFCPerlPod *self, struct CFCFunction *func,
                              const char *sub_name, struct CFCClass *klass,
                              const char *code_sample,
                              const char *class_name, int is_constructor);


#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLPOD */

