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

#define C_LUCY_INDEXFILENAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Util/StringHelper.h"

CharBuf*
IxFileNames_latest_snapshot(Folder *folder) {
    DirHandle *dh = Folder_Open_Dir(folder, NULL);
    CharBuf   *entry = dh ? DH_Get_Entry(dh) : NULL;
    CharBuf   *retval   = NULL;
    uint64_t   latest_gen = 0;

    if (!dh) { RETHROW(INCREF(Err_get_error())); }

    while (DH_Next(dh)) {
        if (CB_Starts_With_Str(entry, "snapshot_", 9)
            && CB_Ends_With_Str(entry, ".json", 5)
           ) {
            uint64_t gen = IxFileNames_extract_gen(entry);
            if (gen > latest_gen) {
                latest_gen = gen;
                if (!retval) { retval = CB_Clone(entry); }
                else         { CB_Mimic(retval, (Obj*)entry); }
            }
        }
    }

    DECREF(dh);
    return retval;
}

uint64_t
IxFileNames_extract_gen(const CharBuf *name) {
    ZombieCharBuf *num_string = ZCB_WRAP(name);

    // Advance past first underscore.  Bail if we run out of string or if we
    // encounter a NULL.
    while (1) {
        uint32_t code_point = ZCB_Nip_One(num_string);
        if (code_point == 0) { return 0; }
        else if (code_point == '_') { break; }
    }

    return (uint64_t)ZCB_BaseX_To_I64(num_string, 36);
}

ZombieCharBuf*
IxFileNames_local_part(const CharBuf *path, ZombieCharBuf *target) {
    ZombieCharBuf *scratch = ZCB_WRAP(path);
    size_t local_part_start = CB_Length(path);
    uint32_t code_point;

    ZCB_Assign(target, path);

    // Trim trailing slash.
    while (ZCB_Code_Point_From(target, 1) == '/') {
        ZCB_Chop(target, 1);
        ZCB_Chop(scratch, 1);
        local_part_start--;
    }

    // Substring should start after last slash.
    while (0 != (code_point = ZCB_Code_Point_From(scratch, 1))) {
        if (code_point == '/') {
            ZCB_Nip(target, local_part_start);
            break;
        }
        ZCB_Chop(scratch, 1);
        local_part_start--;
    }

    return target;
}


