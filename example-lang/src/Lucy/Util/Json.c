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
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(bool_t);
}

Obj*
Json_slurp_json(Folder *folder, const CharBuf *path) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Obj*);
}

CharBuf*
Json_to_json(Obj *dump) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_CharBuf*);
}

Obj*
Json_from_json(CharBuf *json) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Obj*);
}

void
Json_set_tolerant(bool_t tolerant) {
    THROW(LUCY_ERR, "TODO");
}

