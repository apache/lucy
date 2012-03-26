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

#include "Lucy/Util/Json.h"
#include "Lucy/Object/Host.h"
#include "Lucy/Store/Folder.h"

bool_t
Json_spew_json(Obj *dump, Folder *folder, const CharBuf *path) {
    bool_t result = (bool_t)Host_callback_i64(JSON, "spew_json", 3,
                                              ARG_OBJ("dump", dump),
                                              ARG_OBJ("folder", folder),
                                              ARG_STR("path", path));
    if (!result) { ERR_ADD_FRAME(Err_get_error()); }
    return result;
}

Obj*
Json_slurp_json(Folder *folder, const CharBuf *path) {
    Obj *dump = Host_callback_obj(JSON, "slurp_json", 2,
                                  ARG_OBJ("folder", folder),
                                  ARG_STR("path", path));
    if (!dump) { ERR_ADD_FRAME(Err_get_error()); }
    return dump;
}

CharBuf*
Json_to_json(Obj *dump) {
    return Host_callback_str(JSON, "to_json", 1,
                             ARG_OBJ("dump", dump));
}

Obj*
Json_from_json(CharBuf *json) {
    return Host_callback_obj(JSON, "from_json", 1,
                             ARG_STR("json", json));
}

void
Json_set_tolerant(bool_t tolerant) {
    Host_callback(JSON, "set_tolerant", 1,
                  ARG_I32("tolerant", tolerant));
}

