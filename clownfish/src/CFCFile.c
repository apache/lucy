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

#include "CFCFile.h"
#include "CFCUtil.h"

struct CFCFile {
    void *perl_obj;
};

CFCFile*
CFCFile_new(void)
{
    CFCFile *self = (CFCFile*)malloc(sizeof(CFCFile));
    if (!self) { croak("malloc failed"); }
    return CFCFile_init(self);
}

CFCFile*
CFCFile_init(CFCFile *self) 
{
    self->perl_obj = CFCUtil_make_perl_obj(self, "Clownfish::File");
    return self;
}

void
CFCFile_destroy(CFCFile *self)
{
    free(self);
}

CFCFile*
CFCFile_incref(CFCFile *self)
{
    SvREFCNT_inc((SV*)self->perl_obj);
    return self;
}

unsigned
CFCFile_decref(CFCFile *self)
{
    unsigned modified_refcount = SvREFCNT((SV*)self->perl_obj) - 1;
    SvREFCNT_dec((SV*)self->perl_obj);
    return modified_refcount;
}

void*
CFCFile_get_perl_obj(CFCFile *self)
{
    return self->perl_obj;
}

