#define C_LUCY_ERR
#define C_LUCY_OBJ
#define C_LUCY_VTABLE
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Lucy/Object/Err.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/VTable.h"

Err*
Err_new(CharBuf *mess)
{
    Err *self = (Err*)VTable_Make_Obj(ERR);
    return Err_init(self, mess);
}

Err*
Err_init(Err *self, CharBuf *mess)
{
    self->mess = mess;
    return self;
}

void
Err_destroy(Err *self)
{
    DECREF(self->mess);
    SUPER_DESTROY(self, ERR);
}

Err*
Err_make(Err *self)
{
    UNUSED_VAR(self);
    return Err_new(CB_new(0));
}

CharBuf*
Err_to_string(Err *self)
{
    return (CharBuf*)INCREF(self->mess);
}

void
Err_cat_mess(Err *self, const CharBuf *mess)
{
    CB_Cat(self->mess, mess);
}

/* Fallbacks in case variadic macros aren't available. */
#ifndef CHY_HAS_VARIADIC_MACROS
void
THROW(VTable *vtable, char *pattern, ...)
{
    va_list args;
    Err_make_t make 
        = (Err_make_t)METHOD(ASSERT_IS_A(vtable, VTABLE), Err, Make);
    Err *err = ASSERT_IS_A(make(NULL), ERR);
    CharBuf *mess = Err_Get_Mess(err);

    va_start(args, pattern);
    CB_VCatF(message, pattern, args);
    va_end(args);

    Err_do_throw(err);
}
void
LUCY_WARN(char *pattern, ...)
{
    va_list args;
    CharBuf *const message = CB_new(strlen(pattern) + 10);

    va_start(args, pattern);
    CB_VCatF(message, pattern, args);
    va_end(args);

    Err_warn_mess(message);
}
CharBuf*
LUCY_MAKE_MESS(char *pattern, ...)
{
    va_list args;
    CharBuf *const message = CB_new(strlen(pattern) + 10);

    va_start(args, pattern);
    CB_VCatF(message, pattern, args);
    va_end(args);

    return message;
}
#endif


static void
S_vcat_mess(CharBuf *message, const char *file, int line, const char *func, 
            const char *pattern, va_list args)
{
    size_t guess_len = strlen(file) 
                     + func ? strlen(func) : 0 
                     + strlen(pattern) 
                     + 30;
    CB_Grow(message, guess_len);
    CB_VCatF(message, pattern, args);
    if (func != NULL)
        CB_catf(message, ", %s at %s line %i32\n", func, file, (i32_t)line);
    else 
        CB_catf(message, " at %s line %i32\n", file, (i32_t)line);
}

CharBuf*
Err_make_mess(const char *file, int line, const char *func, 
               const char *pattern, ...)
{
    va_list args;
    size_t guess_len = strlen(pattern) + strlen(file) + 20;
    CharBuf *message = CB_new(guess_len);
    va_start(args, pattern);
    S_vcat_mess(message, file, line, func, pattern, args);
    va_end(args);
    return message;
}

void
Err_warn_at(const char *file, int line, const char *func, 
             const char *pattern, ...)
{
    va_list args;
    CharBuf *message = CB_new(0);
    va_start(args, pattern);
    S_vcat_mess(message, file, line, func, pattern, args);
    va_end(args);
    Err_warn_mess(message);
}

CharBuf*
Err_get_mess(Err *self) { return self->mess; }

void
lucy_Err_throw_at(VTable *vtable, const char *file, int line,
                  const char *func, const char *pattern, ...)
{
    va_list args;
    Err_make_t make 
        = (Err_make_t)METHOD(ASSERT_IS_A(vtable, VTABLE), Err, Make);
    Err *err = (Err*)ASSERT_IS_A(make(NULL), ERR);
    CharBuf *mess = Err_Get_Mess(err);

    va_start(args, pattern);
    S_vcat_mess(mess, file, line, func, pattern, args);
    va_end(args);

    Err_do_throw(err);
}

/* Inlined, slightly optimized version of Obj_is_a. */
static INLINE bool_t
SI_obj_is_a(Obj *obj, VTable *target_vtable)
{
    VTable *vtable = obj->vtable;

    while (vtable != NULL) {
        if (vtable == target_vtable)
            return true;
        vtable = vtable->parent;
    }

    return false;
}

Obj*
lucy_Err_assert_is_a(Obj *obj, VTable *vtable, const char *file, int line, 
                      const char *func)
{
    if (!obj) {
        Err_throw_at(ERR, file, line, func, "Object isn't a %o, it's NULL",
            VTable_Get_Name(vtable));
    }
    else if ( !SI_obj_is_a(obj, vtable) ) {
        Err_throw_at(ERR, file, line, func, "Object isn't a %o, it's a %o",
            VTable_Get_Name(vtable), Obj_Get_Class_Name(obj));
    }
    return obj;
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

