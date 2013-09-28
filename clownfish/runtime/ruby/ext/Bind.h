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

/* XSBind.h -- Functions to help bind Clownfish to Perl XS api.
 */

#ifndef H_CFISH_XSBIND
#define H_CFISH_XSBIND 1

#ifdef __cplusplus
extern "C" {
#endif

#include "ruby.h"
#include "Clownfish/Obj.h"
#include "Clownfish/ByteBuf.h"
#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/Num.h"
#include "Clownfish/VArray.h"
#include "Clownfish/VTable.h"

VALUE Bind_cfish_to_ruby(cfish_Obj *obj);
VALUE Bind_str_to_ruby(cfish_String *str);
static VALUE S_cfish_array_to_ruby_array(cfish_VArray *varray);

#ifdef __cplusplus
}
#endif

#endif // H_CFISH_XSBIND
