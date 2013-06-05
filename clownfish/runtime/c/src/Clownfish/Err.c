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

#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "Clownfish/Err.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/VTable.h"

/* TODO: Thread safety */
static Err *current_error;
static Err *thrown_error;
static jmp_buf  *current_env;

void
Err_init_class(void) {
}

Err*
Err_get_error() {
    return current_error;
}

void
Err_set_error(Err *error) {
    if (current_error) {
        DECREF(current_error);
    }
    current_error = error;
}

void
Err_do_throw(Err *error) {
    if (current_env) {
        thrown_error = error;
        longjmp(*current_env, 1);
    }
    else {
        CharBuf *message = Err_get_mess(error);
        fprintf(stderr, "%s", CB_Get_Ptr8(message));
        exit(EXIT_FAILURE);
    }
}

void*
Err_to_host(Err *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

void
Err_throw_mess(VTable *vtable, CharBuf *message) {
    Err_Make_t make
        = METHOD_PTR(CERTIFY(vtable, VTABLE), Cfish_Err_Make);
    Err *err = (Err*)CERTIFY(make(NULL), ERR);
    Err_Cat_Mess(err, message);
    DECREF(message);
    Err_do_throw(err);
}

void
Err_warn_mess(CharBuf *message) {
    fprintf(stderr, "%s", CB_Get_Ptr8(message));
    DECREF(message);
}

Err*
Err_trap(Err_Attempt_t routine, void *context) {
    jmp_buf  env;
    jmp_buf *prev_env = current_env;
    current_env = &env;

    if (!setjmp(env)) {
        routine(context);
    }

    current_env = prev_env;

    Err *error = thrown_error;
    thrown_error = NULL;
    return error;
}

