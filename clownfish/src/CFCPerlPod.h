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

/** Add pod for a method.
 * 
 * @param alias The name of the method, spelled as it will be used from
 * Perl-space.
 * @param method The name of the method from the Clownfish class.  If not
 * supplied, an attempt will be made to locate the correct method using
 * <code>alias</code>.
 * @param sample An optional Perl usage sample.
 * @param pod Optional verbatim POD, which will override all POD which would
 * otherwise have been generated.
 */
void
CFCPerlPod_add_method(CFCPerlPod *self, const char *alias, const char *method,
                      const char *sample, const char *pod);

/** Add pod for a constructor.
 * 
 * @param alias The name of the constructor, spelled as it will be used from
 * Perl-space.
 * @param initializer The name of the initialization routine from the
 * Clownfish class.  Defaults to "init".
 * @param sample An optional Perl usage sample.
 * @param pod Optional verbatim POD, which will override all POD which would
 * otherwise have been generated.
 */
void
CFCPerlPod_add_constructor(CFCPerlPod *self, const char *alias,
                           const char *initializer, const char *sample,
                           const char *pod);

char*
CFCPerlPod_methods_pod(CFCPerlPod *self, struct CFCClass *klass);

char*
CFCPerlPod_constructors_pod(CFCPerlPod *self, struct CFCClass *klass);

void
CFCPerlPod_set_synopsis(CFCPerlPod *self, const char *synopsis);

const char*
CFCPerlPod_get_synopsis(CFCPerlPod *self);

void
CFCPerlPod_set_description(CFCPerlPod *self, const char *description);

const char*
CFCPerlPod_get_description(CFCPerlPod *self);

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

