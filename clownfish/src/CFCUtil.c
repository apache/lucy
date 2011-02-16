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

#include "CFCUtil.h"

void*
CFCUtil_make_perl_obj(void *ptr, const char *klass)
{
    SV *inner_obj = newSV(0);
    SvOBJECT_on(inner_obj);
    PL_sv_objcount++;
    SvUPGRADE(inner_obj, SVt_PVMG);
    sv_setiv(inner_obj, PTR2IV(ptr));

    // Connect class association.
    HV *stash = gv_stashpvn((char*)klass, strlen(klass), TRUE);
    SvSTASH_set(inner_obj, (HV*)SvREFCNT_inc(stash));

    return  inner_obj;
}

