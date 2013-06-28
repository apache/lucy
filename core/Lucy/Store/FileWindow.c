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

#define C_LUCY_FILEWINDOW
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/FileWindow.h"

FileWindow*
FileWindow_new() {
    FileWindow *self = (FileWindow*)VTable_Make_Obj(FILEWINDOW);
    return FileWindow_init(self);
}

FileWindow*
FileWindow_init(FileWindow *self) {
    return self;
}

void
FileWindow_set_offset(FileWindow *self, int64_t offset) {
    FileWindowIVARS *const ivars = FileWindow_IVARS(self);
    if (ivars->buf != NULL) {
        if (offset != ivars->offset) {
            THROW(ERR, "Can't set offset to %i64 instead of %i64 unless buf "
                  "is NULL", offset, ivars->offset);
        }
    }
    ivars->offset = offset;
}

void
FileWindow_set_window(FileWindow *self, char *buf, int64_t offset,
                      int64_t len) {
    FileWindowIVARS *const ivars = FileWindow_IVARS(self);
    ivars->buf    = buf;
    ivars->offset = offset;
    ivars->len    = len;
}


