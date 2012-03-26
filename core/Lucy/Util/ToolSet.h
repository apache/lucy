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

#ifndef H_CFISH_TOOLSET
#define H_CFISH_TOOLSET 1

#ifdef __cplusplus
extern "C" {
#endif

/** ToolSet groups together several commonly used header files, so that only
 * one pound-include directive is needed for them.
 *
 * It should only be used internally, and only included in C files rather than
 * header files, so that the header files remain as sparse as possible.
 */

#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "charmony.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "Lucy/Object/Obj.h"
#include "Lucy/Object/BitVector.h"
#include "Lucy/Object/ByteBuf.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/Hash.h"
#include "Lucy/Object/I32Array.h"
#include "Lucy/Object/Num.h"
#include "Lucy/Object/VArray.h"
#include "Lucy/Object/VTable.h"
#include "Lucy/Util/NumberUtils.h"
#include "Lucy/Util/Memory.h"
#include "Lucy/Util/StringHelper.h"

#ifdef __cplusplus
}
#endif

#endif // H_CFISH_TOOLSET


