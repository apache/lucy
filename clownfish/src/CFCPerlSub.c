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

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerlSub.h"
#include "CFCUtil.h"
#include "CFCParamList.h"

struct CFCPerlSub {
    CFCBase base;
    CFCParamList *param_list;
    char *class_name;
    char *alias;
    int use_labeled_params;
    char *perl_name;
};

CFCPerlSub*
CFCPerlSub_new(const char *klass, CFCParamList *param_list,
               const char *class_name, const char *alias,
               int use_labeled_params) {
    CFCPerlSub *self
        = (CFCPerlSub*)CFCBase_allocate(sizeof(CFCPerlSub), klass);
    return CFCPerlSub_init(self, param_list, class_name, alias,
                           use_labeled_params);
}

CFCPerlSub*
CFCPerlSub_init(CFCPerlSub *self, CFCParamList *param_list,
                const char *class_name, const char *alias,
                int use_labeled_params) {
    CFCUTIL_NULL_CHECK(param_list);
    CFCUTIL_NULL_CHECK(class_name);
    CFCUTIL_NULL_CHECK(alias);
    self->param_list  = (CFCParamList*)CFCBase_incref((CFCBase*)param_list);
    self->class_name  = CFCUtil_strdup(class_name);
    self->alias       = CFCUtil_strdup(alias);
    self->use_labeled_params = use_labeled_params;
    self->perl_name = CFCUtil_cat(CFCUtil_strdup(class_name), "::", alias,
                                  NULL);
    return self;
}

void
CFCPerlSub_destroy(CFCPerlSub *self) {
    CFCBase_decref((CFCBase*)self->param_list);
    FREEMEM(self->class_name);
    FREEMEM(self->alias);
    FREEMEM(self->perl_name);
    CFCBase_destroy((CFCBase*)self);
}

CFCParamList*
CFCPerlSub_get_param_list(CFCPerlSub *self) {
    return self->param_list;
}

const char*
CFCPerlSub_get_class_name(CFCPerlSub *self) {
    return self->class_name;
}

const char*
CFCPerlSub_get_alias(CFCPerlSub *self) {
    return self->alias;
}

int
CFCPerlSub_use_labeled_params(CFCPerlSub *self) {
    return self->use_labeled_params;
}

const char*
CFCPerlSub_perl_name(CFCPerlSub *self) {
    return self->perl_name;
}

