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

#define C_LUCY_MOCKFILEHANDLE
#define C_LUCY_FILEWINDOW
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/Store/MockFileHandle.h"
#include "Lucy/Store/FileWindow.h"

MockFileHandle*
MockFileHandle_new(const CharBuf *path, int64_t length) {
    MockFileHandle *self = (MockFileHandle*)VTable_Make_Obj(MOCKFILEHANDLE);
    return MockFileHandle_init(self, path, length);
}

MockFileHandle*
MockFileHandle_init(MockFileHandle *self, const CharBuf *path,
                    int64_t length) {
    FH_do_open((FileHandle*)self, path, 0);
    self->len = length;
    return self;
}

bool_t
MockFileHandle_window(MockFileHandle *self, FileWindow *window,
                      int64_t offset, int64_t len) {
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, NULL, offset, len);
    return true;
}

bool_t
MockFileHandle_release_window(MockFileHandle *self, FileWindow *window) {
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, NULL, 0, 0);
    return true;
}

int64_t
MockFileHandle_length(MockFileHandle *self) {
    return self->len;
}

bool_t
MockFileHandle_close(MockFileHandle *self) {
    UNUSED_VAR(self);
    return true;
}


