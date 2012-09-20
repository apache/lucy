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

#include "XSBind.h"

#include "Clownfish/VTable.h"

#include "Clownfish/Obj.h"
#include "Clownfish/Host.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Util/Memory.h"

static SV*
S_do_callback_sv(void *vobj, const char *method, uint32_t num_args,
                 va_list args);

// Convert all arguments to Perl and place them on the Perl stack.
static CHY_INLINE void
SI_push_args(void *vobj, va_list args, uint32_t num_args) {
    lucy_Obj *obj = (lucy_Obj*)vobj;
    SV *invoker;
    dSP;

    uint32_t stack_slots_needed = num_args < 2
                                  ? num_args + 1
                                  : (num_args * 2) + 1;
    EXTEND(SP, stack_slots_needed);

    if (Lucy_Obj_Is_A(obj, LUCY_VTABLE)) {
        lucy_VTable *vtable = (lucy_VTable*)obj;
        // TODO: Creating a new class name SV every time is wasteful.
        invoker = XSBind_cb_to_sv(Lucy_VTable_Get_Name(vtable));
    }
    else {
        invoker = (SV*)Lucy_Obj_To_Host(obj);
    }

    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    PUSHs(sv_2mortal(invoker));

    for (uint32_t i = 0; i < num_args; i++) {
        uint32_t arg_type = va_arg(args, uint32_t);
        char *label = va_arg(args, char*);
        if (num_args > 1) {
            PUSHs(sv_2mortal(newSVpvn(label, strlen(label))));
        }
        switch (arg_type & CFISH_HOST_ARGTYPE_MASK) {
            case CFISH_HOST_ARGTYPE_I32: {
                    int32_t value = va_arg(args, int32_t);
                    PUSHs(sv_2mortal(newSViv(value)));
                }
                break;
            case CFISH_HOST_ARGTYPE_I64: {
                    int64_t value = va_arg(args, int64_t);
                    if (sizeof(IV) == 8) {
                        PUSHs(sv_2mortal(newSViv((IV)value)));
                    }
                    else {
                        // lossy
                        PUSHs(sv_2mortal(newSVnv((double)value)));
                    }
                }
                break;
            case CFISH_HOST_ARGTYPE_F32:
            case CFISH_HOST_ARGTYPE_F64: {
                    // Floats are promoted to doubles by variadic calling.
                    double value = va_arg(args, double);
                    PUSHs(sv_2mortal(newSVnv(value)));
                }
                break;
            case CFISH_HOST_ARGTYPE_STR: {
                    lucy_CharBuf *string = va_arg(args, lucy_CharBuf*);
                    PUSHs(sv_2mortal(XSBind_cb_to_sv(string)));
                }
                break;
            case CFISH_HOST_ARGTYPE_OBJ: {
                    lucy_Obj* anObj = va_arg(args, lucy_Obj*);
                    SV *arg_sv = anObj == NULL
                                 ? newSV(0)
                                 : XSBind_cfish_to_perl(anObj);
                    PUSHs(sv_2mortal(arg_sv));
                }
                break;
            default:
                CFISH_THROW(LUCY_ERR, "Unrecognized arg type: %u32",
                            arg_type);
        }
    }

    PUTBACK;
}

void
lucy_Host_callback(void *vobj, const char *method, uint32_t num_args, ...) {
    va_list args;

    va_start(args, num_args);
    SI_push_args(vobj, args, num_args);
    va_end(args);

    int count = call_method(method, G_VOID | G_DISCARD);
    if (count != 0) {
        CFISH_THROW(LUCY_ERR, "callback '%s' returned too many values: %i32",
                    method, (int32_t)count);
    }
    FREETMPS;
    LEAVE;
}

int64_t
lucy_Host_callback_i64(void *vobj, const char *method, uint32_t num_args, ...) {
    va_list args;
    SV *return_sv;
    int64_t retval;

    va_start(args, num_args);
    return_sv = S_do_callback_sv(vobj, method, num_args, args);
    va_end(args);
    if (sizeof(IV) == 8) {
        retval = (int64_t)SvIV(return_sv);
    }
    else {
        if (SvIOK(return_sv)) {
            // It's already no more than 32 bits, so don't convert.
            retval = SvIV(return_sv);
        }
        else {
            // Maybe lossy.
            double temp = SvNV(return_sv);
            retval = (int64_t)temp;
        }
    }

    FREETMPS;
    LEAVE;

    return retval;
}

double
lucy_Host_callback_f64(void *vobj, const char *method, uint32_t num_args, ...) {
    va_list args;
    SV *return_sv;
    double retval;

    va_start(args, num_args);
    return_sv = S_do_callback_sv(vobj, method, num_args, args);
    va_end(args);
    retval = SvNV(return_sv);

    FREETMPS;
    LEAVE;

    return retval;
}

lucy_Obj*
lucy_Host_callback_obj(void *vobj, const char *method, uint32_t num_args, ...) {
    va_list args;
    SV *temp_retval;
    lucy_Obj *retval = NULL;

    va_start(args, num_args);
    temp_retval = S_do_callback_sv(vobj, method, num_args, args);
    va_end(args);

    retval = XSBind_perl_to_cfish(temp_retval);

    FREETMPS;
    LEAVE;

    return retval;
}

lucy_CharBuf*
lucy_Host_callback_str(void *vobj, const char *method, uint32_t num_args, ...) {
    va_list args;
    SV *temp_retval;
    lucy_CharBuf *retval = NULL;

    va_start(args, num_args);
    temp_retval = S_do_callback_sv(vobj, method, num_args, args);
    va_end(args);

    // Make a stringified copy.
    if (temp_retval && XSBind_sv_defined(temp_retval)) {
        STRLEN len;
        char *ptr = SvPVutf8(temp_retval, len);
        retval = lucy_CB_new_from_trusted_utf8(ptr, len);
    }

    FREETMPS;
    LEAVE;

    return retval;
}

void*
lucy_Host_callback_host(void *vobj, const char *method, uint32_t num_args, ...) {
    va_list args;
    SV *retval;

    va_start(args, num_args);
    retval = S_do_callback_sv(vobj, method, num_args, args);
    va_end(args);
    SvREFCNT_inc(retval);

    FREETMPS;
    LEAVE;

    return retval;
}

static SV*
S_do_callback_sv(void *vobj, const char *method, uint32_t num_args,
                 va_list args) {
    SV *return_val;
    SI_push_args(vobj, args, num_args);
    int num_returned = call_method(method, G_SCALAR);
    dSP;
    if (num_returned != 1) {
        CFISH_THROW(LUCY_ERR, "Bad number of return vals from %s: %i32",
                    method, (int32_t)num_returned);
    }
    return_val = POPs;
    PUTBACK;
    return return_val;
}


