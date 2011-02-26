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
#include "CFCDumpable.h"
#include "CFCUtil.h"

struct CFCDumpable {
    CFCBase base;
};

CFCDumpable*
CFCDumpable_new(void)
{
    CFCDumpable *self = (CFCDumpable*)CFCBase_allocate(sizeof(CFCDumpable),
        "Clownfish::Dumpable");
    return CFCDumpable_init(self);
}

CFCDumpable*
CFCDumpable_init(CFCDumpable *self)
{
    return self;
}

void
CFCDumpable_destroy(CFCDumpable *self)
{
    CFCBase_destroy((CFCBase*)self);
}

