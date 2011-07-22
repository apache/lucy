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

#define CFC_NEED_PERLSUB_STRUCT_DEF 1
#include "CFCPerlSub.h"
#include "CFCPerlMethod.h"
#include "CFCUtil.h"
#include "CFCMethod.h"
#include "CFCParamList.h"

struct CFCPerlMethod {
    CFCPerlSub  sub;
    CFCMethod  *method;
};

CFCPerlMethod*
CFCPerlMethod_new(CFCMethod *method, const char *alias) {
    CFCPerlMethod *self
        = (CFCPerlMethod*)CFCBase_allocate(sizeof(CFCPerlMethod),
                                          "Clownfish::Binding::Perl::Method");
    return CFCPerlMethod_init(self, method, alias);
}

CFCPerlMethod*
CFCPerlMethod_init(CFCPerlMethod *self, CFCMethod *method,
                   const char *alias) {
    CFCParamList *param_list = CFCMethod_get_param_list(method);
    const char *class_name = CFCMethod_get_class_name(method);
    int use_labeled_params = CFCParamList_num_vars(param_list) > 2
                             ? 1 : 0;
    if (!alias) {
        alias = CFCMethod_micro_sym(method);
    }
    CFCPerlSub_init((CFCPerlSub*)self, param_list, class_name, alias,
                    use_labeled_params);
    self->method = (CFCMethod*)CFCBase_incref((CFCBase*)method);
    return self;
}

void
CFCPerlMethod_destroy(CFCPerlMethod *self) {
    CFCBase_decref((CFCBase*)self->method);
    CFCPerlSub_destroy((CFCPerlSub*)self);
}

CFCMethod*
CFCPerlMethod_get_method(CFCPerlMethod *self) {
    return self->method;
}

