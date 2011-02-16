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

#include "CFCParamList.h"

struct CFCParamList {
    void *variables;
    void *values;
    int   variadic;
};

CFCParamList*
CFCParamList_new(int variadic)
{
    CFCParamList *self = (CFCParamList*)malloc(sizeof(CFCParamList));
    if (!self) { croak("malloc failed"); }
    return CFCParamList_init(self, variadic);
}

CFCParamList*
CFCParamList_init(CFCParamList *self, int variadic)
{
    self->variables   = newAV();
    self->values      = newAV();
    self->variadic    = variadic;
    return self;
}

void
CFCParamList_add_param(CFCParamList *self, void *variable, void *value)
{
    av_push((AV*)self->variables, newSVsv((SV*)variable));
    av_push((AV*)self->values,    newSVsv((SV*)value));
}

void
CFCParamList_destroy(CFCParamList *self)
{
    SvREFCNT_dec((SV*)self->variables);
    SvREFCNT_dec((SV*)self->values);
    free(self);
}

void*
CFCParamList_get_variables(CFCParamList *self)
{
    return self->variables;
}

void*
CFCParamList_get_initial_values(CFCParamList *self)
{
    return self->values;
}

int
CFCParamList_variadic(CFCParamList *self)
{
    return self->variadic;
}

