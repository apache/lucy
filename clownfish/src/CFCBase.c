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
#include "CFCUtil.h"

CFCBase*
CFCBase_allocate(size_t size, const char *klass)
{
    CFCBase *self = (CFCBase*)CALLOCATE(size, 1);
    self->perl_obj = CFCUtil_make_perl_obj(self, klass);
    return self;
}

void
CFCBase_destroy(CFCBase *self)
{
    FREEMEM(self);
}

CFCBase*
CFCBase_incref(CFCBase *self)
{
    if (self) {
        SvREFCNT_inc((SV*)self->perl_obj);
    }
    return self;
}

unsigned
CFCBase_decref(CFCBase *self)
{
    if (!self) { return 0; }
    unsigned modified_refcount = SvREFCNT((SV*)self->perl_obj) - 1;
    /* When the SvREFCNT for this Perl object falls to zero, DESTROY will be
     * invoked from Perl space for the class that the Perl object was blessed
     * into.  Thus even though the very simple CFC object model does not
     * generally support polymorphism, we get it for object destruction. */
    SvREFCNT_dec((SV*)self->perl_obj);
    return modified_refcount;
}

void*
CFCBase_get_perl_obj(CFCBase *self)
{
    return self->perl_obj;
}

const char*
CFCBase_get_cfc_class(CFCBase *self)
{
    return HvNAME(SvSTASH((SV*)self->perl_obj));
}


