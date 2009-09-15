#define C_LUCY_UNDEFINED
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Lucy/Object/VTable.h"
#include "Lucy/Object/Undefined.h"

static Undefined the_undef_object = { UNDEFINED, {1} };
Undefined *UNDEF = &the_undef_object;

u32_t
Undefined_get_refcount(Undefined* self)
{
    CHY_UNUSED_VAR(self);
    return 1;
}

Undefined*
Undefined_inc_refcount(Undefined* self)
{
    return self;
}

u32_t
Undefined_dec_refcount(Undefined* self)
{
    UNUSED_VAR(self);
    return 1;
}

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

