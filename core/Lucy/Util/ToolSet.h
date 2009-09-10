#ifndef H_LUCY_TOOLSET
#define H_LUCY_TOOLSET 1

/* ToolSet groups together several commonly used header files, so that only
 * one pound-include directive is needed for them.
 *
 * It should only be used internally, and only included in C files rather than
 * header files, so that the header files remain as sparse as possible.
 */

#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#define C_LUCY_ZOMBIECHARBUF

#include "charmony.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/* #include "Lucy/Obj.h" */
/* #include "Lucy/Obj/BitVector.h" */
/* #include "Lucy/Obj/ByteBuf.h" */
/* #include "Lucy/Obj/CharBuf.h" */
/* #include "Lucy/Obj/Err.h" */
/* #include "Lucy/Obj/Hash.h" */
/* #include "Lucy/Obj/Num.h" */
/* #include "Lucy/Obj/Undefined.h" */
/* #include "Lucy/Obj/VArray.h" */
/* #include "Lucy/Obj/VTable.h" */
/* #include "Lucy/Util/NumberUtils.h" */
#include "Lucy/Util/Memory.h"
#include "Lucy/Util/StringHelper.h"

#endif /* H_LUCY_TOOLSET */

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

