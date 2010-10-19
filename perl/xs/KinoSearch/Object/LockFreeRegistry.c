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

#define C_KINO_OBJ
#define C_KINO_LOCKFREEREGISTRY
#include "xs/XSBind.h"

#include "KinoSearch/Object/LockFreeRegistry.h"
#include "KinoSearch/Object/Host.h"

void*
kino_LFReg_to_host(kino_LockFreeRegistry *self)
{
    chy_bool_t first_time = self->ref.count < 4 ? true : false;
    kino_LFReg_to_host_t to_host = (kino_LFReg_to_host_t)
        KINO_SUPER_METHOD(KINO_LOCKFREEREGISTRY, LFReg, To_Host);
    SV *host_obj = (SV*)to_host(self);
    if (first_time) {
        SvSHARE((SV*)self->ref.host_obj);
    }
    return host_obj;
}


