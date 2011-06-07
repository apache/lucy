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

#include "Lucy/Object/Obj.h"
#include "Lucy/Object/Err.h"

uint32_t
lucy_Obj_get_refcount(lucy_Obj *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(uint32_t);
}

lucy_Obj*
lucy_Obj_inc_refcount(lucy_Obj *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Obj*);
}

uint32_t
lucy_Obj_dec_refcount(lucy_Obj *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(uint32_t);
}

void*
lucy_Obj_to_host(lucy_Obj *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}


