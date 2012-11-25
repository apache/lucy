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

/* CFBind.h -- Functions to help bind Clownfish to [LANGUAGE_X].
 */

#ifndef H_CFISH_CFBIND
#define H_CFISH_CFBIND 1

#ifdef __cplusplus
extern "C" {
#endif

#include "charmony.h"
#include "Clownfish/Obj.h"
#include "Clownfish/ByteBuf.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/Num.h"
#include "Clownfish/VArray.h"
#include "Clownfish/VTable.h"

/* Strip the prefix from some common symbols where we know there's no
 * conflict.  It's a little inconsistent to do this rather than leave all
 * symbols at full size, but the succinctness is worth it.
 */
#define THROW                CFISH_THROW
#define WARN                 CFISH_WARN
#define UNUSED_VAR           CHY_UNUSED_VAR
#define UNREACHABLE_RETURN   CHY_UNREACHABLE_RETURN

#ifdef __cplusplus
}
#endif

#endif // H_CFISH_CFBIND


