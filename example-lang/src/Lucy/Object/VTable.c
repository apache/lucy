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

#define C_LUCY_OBJ
#define C_LUCY_VTABLE

#include "Lucy/Object/VTable.h"
#include "Lucy/Object/Host.h"
#include "Lucy/Object/CharBuf.h"

lucy_Obj*
lucy_VTable_foster_obj(lucy_VTable *self, void *host_obj) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Obj*);
}

void
lucy_VTable_register_with_host(lucy_VTable *singleton, lucy_VTable *parent) {
    THROW(LUCY_ERR, "TODO");
}

lucy_VArray*
lucy_VTable_novel_host_methods(const lucy_CharBuf *class_name) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_VArray*);
}

lucy_CharBuf*
lucy_VTable_find_parent_class(const lucy_CharBuf *class_name) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_CharBuf*);
}

void*
lucy_VTable_to_host(lucy_VTable *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

