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
#include "Clownfish/Util/StringHelper.h"

String*
IxFileNames_latest_snapshot(Folder *folder) {
    DirHandle *dh = Folder_Open_Dir(folder, NULL);
    String    *retval   = NULL;
    uint64_t   latest_gen = 0;

    if (!dh) { RETHROW(INCREF(Err_get_error())); }

    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        if (Str_Starts_With_Utf8(entry, "snapshot_", 9)
            && Str_Ends_With_Utf8(entry, ".json", 5)
           ) {
            uint64_t gen = IxFileNames_extract_gen(entry);
            if (gen > latest_gen) {
                latest_gen = gen;
                DECREF(retval);
                retval = Str_Clone(entry);
            }
        }
        DECREF(entry);
    }

    DECREF(dh);
    return retval;
}

uint64_t
IxFileNames_extract_gen(String *name) {
    StringIterator *iter = Str_Top(name);

    // Advance past first underscore.  Bail if we run out of string or if we
    // encounter a NULL.
    while (1) {
        int32_t code_point = StrIter_Next(iter);
        if (code_point == STRITER_DONE) { return 0; }
        else if (code_point == '_') { break; }
    }

    String *num_string = StrIter_substring(iter, NULL);
    uint64_t retval = (uint64_t)Str_BaseX_To_I64(num_string, 36);

    DECREF(num_string);
    DECREF(iter);
    return retval;
}

String*
IxFileNames_local_part(String *path) {
    StringIterator *top = Str_Tail(path);
    int32_t code_point = StrIter_Prev(top);

    // Trim trailing slash.
    while (code_point == '/') {
        code_point = StrIter_Prev(top);
    }

    StringIterator *tail = StrIter_Clone(top);
    StrIter_Advance(tail, 1);

    // Substring should start after last slash.
    while (code_point != STRITER_DONE) {
        if (code_point == '/') {
            StrIter_Advance(top, 1);
            break;
        }
        code_point = StrIter_Prev(top);
    }

    String *retval = StrIter_substring(top, tail);

    DECREF(tail);
    DECREF(top);
    return retval;
}


