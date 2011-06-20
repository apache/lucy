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

#include <stdio.h>
#include <string.h>
#include "CFCBindMethod.h"
#include "CFCUtil.h"
#include "CFCMethod.h"
#include "CFCFunction.h"
#include "CFCParamList.h"
#include "CFCType.h"

char*
CFCBindMeth_typdef_dec(struct CFCMethod *method) {
    const char *params 
        = CFCParamList_to_c(CFCFunction_get_param_list((CFCFunction*)method));
    const char *ret_type
        = CFCType_to_c(CFCFunction_get_return_type((CFCFunction*)method));
    const char *full_typedef = CFCMethod_full_typedef(method);
    size_t size = strlen(params)
                  + strlen(ret_type)
                  + strlen(full_typedef)
                  + 20
                  + sizeof("\0");
    char *buf = (char*)MALLOCATE(size);
    sprintf(buf, "typedef %s\n(*%s)(%s);\n", ret_type, full_typedef, params);
    return buf;
}

