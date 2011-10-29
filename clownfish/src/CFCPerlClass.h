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

CFCPerlClass*
CFCPerlClass_new(struct CFCParcel *parcel, const char *class_name,
                 struct CFCClass *client, const char *xs_code);

CFCPerlClass*
CFCPerlClass_init(CFCPerlClass *self, struct CFCParcel *parcel,
                  const char *class_name, struct CFCClass *client,
                  const char *xs_code);

void
CFCPerlClass_destroy(CFCPerlClass *self);

char*
CFCPerlClass_perlify_doc_text(CFCPerlClass *self, const char *source);

char*
CFCPerlClass_gen_subroutine_pod(CFCPerlClass *self, struct CFCFunction *func,
                                const char *sub_name, struct CFCClass *klass,
                                const char *code_sample,
                                const char *class_name, int is_constructor);

struct CFCClass*
CFCPerlClass_get_client(CFCPerlClass *self);

const char*
CFCPerlClass_get_class_name(CFCPerlClass *self);

const char*
CFCPerlClass_get_xs_code(CFCPerlClass *self);


#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLCLASS */

