#define C_LUCY_INDEXFILENAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/StringHelper.h"

i32_t
IxFileNames_extract_gen(const CharBuf *name)
{
    ZombieCharBuf num_string = ZCB_make(name);

    /* Advance past first underscore.  Bail if we run out of string or if we
     * encounter a NULL. */
    while (1) {
        u32_t code_point = ZCB_Nip_One(&num_string);
        if (code_point == 0) { return 0; }
        else if (code_point == '_') { break; }
    }

    return (i32_t)ZCB_BaseX_To_I64(&num_string, 36);
}

ZombieCharBuf*
IxFileNames_local_part(const CharBuf *path, ZombieCharBuf *target)
{
    ZombieCharBuf scratch   = ZCB_make(path);
    size_t local_part_start = CB_Length(path);
    u32_t  code_point;

    ZCB_Assign(target, path);

    /* Trim trailing slash. */
    while (ZCB_Code_Point_From(target, 1) == '/') {
        ZCB_Chop(target, 1);
        ZCB_Chop(&scratch, 1);
        local_part_start--;
    }

    /* Substring should start after last slash. */
    while (0 != (code_point = ZCB_Code_Point_From(&scratch, 1))) {
        if (code_point == '/') {
            ZCB_Nip(target, local_part_start);
            break;
        }
        ZCB_Chop(&scratch, 1);
        local_part_start--;
    }

    return target;
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

