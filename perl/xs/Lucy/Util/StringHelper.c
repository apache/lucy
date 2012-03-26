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

#include "XSBind.h"
#include "Lucy/Util/StringHelper.h"

// TODO: replace with code from ICU in common/ucnv_u8.c.
chy_bool_t
lucy_StrHelp_utf8_valid(const char *ptr, size_t size) {
    const U8 *uptr = (const U8*)ptr;
    return size == 0 ? true : !!is_utf8_string(uptr, size);
}


