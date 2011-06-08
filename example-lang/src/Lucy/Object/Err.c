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

#include "CFBind.h"

lucy_Err*
lucy_Err_get_error() {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Err*);
}

void
lucy_Err_set_error(lucy_Err *error) {
    THROW(LUCY_ERR, "TODO");
}

void*
lucy_Err_to_host(lucy_Err *self) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

void
lucy_Err_throw_mess(lucy_VTable *vtable, lucy_CharBuf *message) {
    THROW(LUCY_ERR, "TODO");
}

void
lucy_Err_warn_mess(lucy_CharBuf *message) {
    THROW(LUCY_ERR, "TODO");
}


