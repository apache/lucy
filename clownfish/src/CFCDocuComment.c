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

#include "CFCDocuComment.h"

struct CFCDocuComment {
    const char *description;
    const char *brief;
    const char *long_des;
    void *param_names;
    void *param_docs;
    const char *retval;
};

CFCDocuComment*
CFCDocuComment_new(const char *description, const char *brief, 
                   const char *long_description, void *param_names, 
                   void *param_docs, const char *retval)
{
    CFCDocuComment *self = (CFCDocuComment*)malloc(sizeof(CFCDocuComment));
    if (!self) { croak("malloc failed"); }
    return CFCDocuComment_init(self, description, brief, long_description,
        param_names, param_docs, retval);
}

CFCDocuComment*
CFCDocuComment_init(CFCDocuComment *self, const char *description, 
                    const char *brief, const char *long_description, 
                    void *param_names, void *param_docs, const char *retval)
{
    self->description = savepv(description);
    self->brief       = savepv(brief);
    self->long_des    = savepv(long_description);
    self->param_names = newSVsv((SV*)param_names);
    self->param_docs  = newSVsv((SV*)param_docs);
    self->retval      = retval ? savepv(retval) : NULL;
    return self;
}

void
CFCDocuComment_destroy(CFCDocuComment *self)
{
    Safefree(self->description);
    Safefree(self->brief);
    Safefree(self->long_des);
    SvREFCNT_dec((SV*)self->param_names);
    SvREFCNT_dec((SV*)self->param_docs);
    Safefree(self->retval);
    free(self);
}


const char*
CFCDocuComment_get_description(CFCDocuComment *self)
{
    return self->description;
}

const char*
CFCDocuComment_get_brief(CFCDocuComment *self)
{
    return self->brief;
}

const char*
CFCDocuComment_get_long(CFCDocuComment *self)
{
    return self->long_des;
}

void*
CFCDocuComment_get_param_names(CFCDocuComment *self)
{
    return self->param_names;
}

void*
CFCDocuComment_get_param_docs(CFCDocuComment *self)
{
    return self->param_docs;
}

const char*
CFCDocuComment_get_retval(CFCDocuComment *self)
{
    return self->retval;
}

