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
#include "CFCBindFunction.h"
#include "CFCUtil.h"
#include "CFCFunction.h"
#include "CFCParamList.h"
#include "CFCType.h"

char*
CFCBindFunc_func_declaration(CFCFunction *func) {
    CFCType      *return_type    = CFCFunction_get_return_type(func);
    CFCParamList *param_list     = CFCFunction_get_param_list(func);
    const char   *ret_type_str   = CFCType_to_c(return_type);
    const char   *full_func_sym  = CFCFunction_full_func_sym(func);
    const char   *param_list_str = CFCParamList_to_c(param_list);
    const char   *inline_prop    = CFCFunction_inline(func)
                                   ? "static CFISH_INLINE "
                                   : "";
    char *buf = CFCUtil_sprintf("%s%s\n%s(%s);", inline_prop, ret_type_str,
                                full_func_sym, param_list_str);
    return buf;
}

