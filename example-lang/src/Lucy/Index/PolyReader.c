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

#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Object/Host.h"
#include "Lucy/Store/Folder.h"

Obj*
PolyReader_try_open_segreaders(PolyReader *self, VArray *segments) {
    return Host_callback_obj(self, "try_open_segreaders", 1,
                             ARG_OBJ("segments", segments));
}

CharBuf*
PolyReader_try_read_snapshot(Snapshot *snapshot, Folder *folder,
                             const CharBuf *path) {
    return (CharBuf*)Host_callback_obj(POLYREADER, "try_read_snapshot", 3,
                                       ARG_OBJ("snapshot", snapshot),
                                       ARG_OBJ("folder", folder),
                                       ARG_STR("path", path));
}


