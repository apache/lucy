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

void
lucy_Host_callback(void *vobj, char *method, uint32_t num_args, ...) {
    THROW(LUCY_ERR, "TODO");
}

int64_t
lucy_Host_callback_i64(void *vobj, char *method, uint32_t num_args, ...) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(int64_t);
}

double
lucy_Host_callback_f64(void *vobj, char *method, uint32_t num_args, ...) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(double);
}

lucy_Obj*
lucy_Host_callback_obj(void *vobj, char *method, uint32_t num_args, ...) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_Obj*);
}

lucy_CharBuf*
lucy_Host_callback_str(void *vobj, char *method, uint32_t num_args, ...) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(lucy_CharBuf*);
}

void*
lucy_Host_callback_host(void *vobj, char *method, uint32_t num_args, ...) {
    THROW(LUCY_ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

