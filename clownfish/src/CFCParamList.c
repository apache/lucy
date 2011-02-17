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

#include <stdlib.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCParamList.h"
#include "CFCVariable.h"
#include "CFCUtil.h"

struct CFCParamList {
    CFCBase       base;
    CFCVariable **variables;
    char        **values;
    int           variadic;
    size_t        num_vars;
};

CFCParamList*
CFCParamList_new(int variadic)
{
    CFCParamList *self = (CFCParamList*)CFCBase_allocate(sizeof(CFCParamList), 
        "Clownfish::ParamList");
    return CFCParamList_init(self, variadic);
}

CFCParamList*
CFCParamList_init(CFCParamList *self, int variadic)
{
    self->variadic  = variadic;
    self->num_vars  = 0;
    self->variables = (CFCVariable**)calloc(1, sizeof(void*));
    self->values    = (char**)calloc(1, sizeof(char*));
    return self;
}

void
CFCParamList_add_param(CFCParamList *self, CFCVariable *variable, 
                       const char *value)
{
    self->num_vars++;
    size_t amount = (self->num_vars + 1) * sizeof(void*);
    self->variables = (CFCVariable**)realloc(self->variables, amount);
    self->values    = (char**)realloc(self->values, amount);
    if (!self->variables || !self->values) {
        croak("realloc failed.");
    }
    self->variables[self->num_vars - 1] 
        = (CFCVariable*)CFCBase_incref((CFCBase*)variable);
    self->values[self->num_vars - 1] = value ? CFCUtil_strdup(value) : NULL;
    self->variables[self->num_vars] = NULL;
    self->values[self->num_vars] = NULL;
}

void
CFCParamList_destroy(CFCParamList *self)
{
    size_t i;
    for (i = 0; i < self->num_vars; i++) {
        CFCBase_decref((CFCBase*)self->variables[i]);
        free(self->values[i]);
    }
    free(self->variables);
    free(self->values);
    CFCBase_destroy((CFCBase*)self);
}

CFCVariable**
CFCParamList_get_variables(CFCParamList *self)
{
    return self->variables;
}

const char**
CFCParamList_get_initial_values(CFCParamList *self)
{
    return (const char**)self->values;
}

size_t
CFCParamList_num_vars(CFCParamList *self)
{
    return self->num_vars;
}

int
CFCParamList_variadic(CFCParamList *self)
{
    return self->variadic;
}

