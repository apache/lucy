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

/* XSBind.h -- Functions to help bind Clownfish to Perl XS api.
 */

#ifndef H_CFISH_XSBIND
#define H_CFISH_XSBIND 1

#include "Clownfish/Obj.h"
#include "Clownfish/ByteBuf.h"
#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/Num.h"
#include "Clownfish/VArray.h"
#include "Clownfish/VTable.h"

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define NEED_newRV_noinc_GLOBAL
#include "ppport.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Given either a class name or a perl object, manufacture a new Clownfish
 * object suitable for supplying to a cfish_Foo_init() function.
 */
CFISH_VISIBLE cfish_Obj*
cfish_XSBind_new_blank_obj(SV *either_sv);

/** Test whether an SV is defined.  Handles "get" magic, unlike SvOK on its
 * own.
 */
static CFISH_INLINE bool
cfish_XSBind_sv_defined(SV *sv) {
    if (!sv || !SvANY(sv)) { return false; }
    if (SvGMAGICAL(sv)) { mg_get(sv); }
    return SvOK(sv);
}

/** If the SV contains a Clownfish object which passes an "isa" test against the
 * passed-in VTable, return a pointer to it.  If not, but
 * <code>allocation</code> is non-NULL and a StackString would satisfy the
 * "isa" test, stringify the SV, create a StackString using
 * <code>allocation</code>, assign the SV's string to it, and return that
 * instead.  If all else fails, throw an exception.
 */
CFISH_VISIBLE cfish_Obj*
cfish_XSBind_sv_to_cfish_obj(SV *sv, cfish_VTable *vtable, void *allocation);

/** As XSBind_sv_to_cfish_obj above, but returns NULL instead of throwing an
 * exception.
 */
CFISH_VISIBLE cfish_Obj*
cfish_XSBind_maybe_sv_to_cfish_obj(SV *sv, cfish_VTable *vtable,
                                   void *allocation);


/** Derive an SV from a Clownfish object.  If the Clownfish object is NULL, the SV
 * will be undef.
 *
 * The new SV has single refcount for which the caller must take
 * responsibility.
 */
static CFISH_INLINE SV*
cfish_XSBind_cfish_obj_to_sv(cfish_Obj *obj) {
    return obj ? (SV*)CFISH_Obj_To_Host(obj) : newSV(0);
}

/** XSBind_cfish_obj_to_sv, with a cast.
 */
#define CFISH_OBJ_TO_SV(_obj) cfish_XSBind_cfish_obj_to_sv((cfish_Obj*)_obj)

/** As XSBind_cfish_obj_to_sv above, except decrements the object's refcount
 * after creating the SV. This is useful when the Clownfish expression creates a new
 * refcount, e.g.  a call to a constructor.
 */
static CFISH_INLINE SV*
cfish_XSBind_cfish_obj_to_sv_noinc(cfish_Obj *obj) {
    SV *retval;
    if (obj) {
        retval = (SV*)CFISH_Obj_To_Host(obj);
        CFISH_Obj_Dec_RefCount(obj);
    }
    else {
        retval = newSV(0);
    }
    return retval;
}

/** XSBind_cfish_obj_to_sv_noinc, with a cast.
 */
#define CFISH_OBJ_TO_SV_NOINC(_obj) \
    cfish_XSBind_cfish_obj_to_sv_noinc((cfish_Obj*)_obj)

/** Deep conversion of Clownfish objects to Perl objects -- Strings to UTF-8
 * SVs, ByteBufs to SVs, VArrays to Perl array refs, Hashes to Perl hashrefs,
 * and any other object to a Perl object wrapping the Clownfish Obj.
 */
CFISH_VISIBLE SV*
cfish_XSBind_cfish_to_perl(cfish_Obj *obj);

/** Deep conversion of Perl data structures to Clownfish objects -- Perl hash
 * to Hash, Perl array to VArray, Clownfish objects stripped of their
 * wrappers, and everything else stringified and turned to a String.
 */
CFISH_VISIBLE cfish_Obj*
cfish_XSBind_perl_to_cfish(SV *sv);

/** Convert a ByteBuf into a new string SV.
 */
CFISH_VISIBLE SV*
cfish_XSBind_bb_to_sv(const cfish_ByteBuf *bb);

/** Convert a String into a new UTF-8 string SV.
 */
CFISH_VISIBLE SV*
cfish_XSBind_str_to_sv(cfish_String *str);

/** Perl-specific wrapper for Err#trap.  The "routine" must be either a
 * subroutine reference or the name of a subroutine.
 */
cfish_Err*
cfish_XSBind_trap(SV *routine, SV *context);

/** Turn on overloading for the supplied Perl object and its class.
 */
CFISH_VISIBLE void
cfish_XSBind_enable_overload(void *pobj);

/** Process hash-style params passed to an XS subroutine.  The varargs must be
 * a NULL-terminated series of ALLOT_ macros.
 *
 *     cfish_XSBind_allot_params(stack, start, num_stack_elems,
 *          ALLOT_OBJ(&field, "field", 5, CFISH_STRING, true, alloca(cfish_SStr_size()),
 *          ALLOT_OBJ(&term, "term", 4, CFISH_STRING, true, alloca(cfish_SStr_size()),
 *          NULL);
 *
 * The following ALLOT_ macros are available for primitive types:
 *
 *     ALLOT_I8(ptr, key, keylen, required)
 *     ALLOT_I16(ptr, key, keylen, required)
 *     ALLOT_I32(ptr, key, keylen, required)
 *     ALLOT_I64(ptr, key, keylen, required)
 *     ALLOT_U8(ptr, key, keylen, required)
 *     ALLOT_U16(ptr, key, keylen, required)
 *     ALLOT_U32(ptr, key, keylen, required)
 *     ALLOT_U64(ptr, key, keylen, required)
 *     ALLOT_BOOL(ptr, key, keylen, required)
 *     ALLOT_CHAR(ptr, key, keylen, required)
 *     ALLOT_SHORT(ptr, key, keylen, required)
 *     ALLOT_INT(ptr, key, keylen, required)
 *     ALLOT_LONG(ptr, key, keylen, required)
 *     ALLOT_SIZE_T(ptr, key, keylen, required)
 *     ALLOT_F32(ptr, key, keylen, required)
 *     ALLOT_F64(ptr, key, keylen, required)
 *
 * The four arguments to these ALLOT_ macros have the following meanings:
 *
 *     ptr -- A pointer to the variable to be extracted.
 *     key -- The name of the parameter as a C string.
 *     keylen -- The length of the parameter name in bytes.
 *     required -- A boolean indicating whether the parameter is required.
 *
 * If a required parameter is not present, allot_params() will set Err_error
 * and return false.
 *
 * Use the following macro if a Clownfish object is desired:
 *
 *     ALLOT_OBJ(ptr, key, keylen, required, vtable, allocation)
 *
 * The "vtable" argument must be the VTable corresponding to the class of the
 * desired object.  The "allocation" argument must be a blob of memory
 * allocated on the stack sufficient to hold a StackString.  (Use
 * cfish_SStr_size() to find the allocation size.)
 *
 * To extract a Perl scalar, use the following ALLOT_ macro:
 *
 *     ALLOT_SV(ptr, key, keylen, required)
 *
 * All possible valid param names must be passed via the ALLOT_ macros; if a
 * user-supplied param cannot be matched up with an ALLOT_ macro,
 * allot_params() will set Err_error and return false.
 *
 * @param stack The Perl stack.
 * @param start Where on the Perl stack to start looking for params.  For
 * methods, this would typically be 1; for functions, most likely 0.
 * @param num_stack_elems The number of arguments passed to the Perl function
 * (generally, the XS variable "items").
 * @return true on success, false on failure (sets Err_error).
 */
CFISH_VISIBLE bool
cfish_XSBind_allot_params(SV** stack, int32_t start,
                          int32_t num_stack_elems, ...);

#define XSBIND_WANT_I8       0x1
#define XSBIND_WANT_I16      0x2
#define XSBIND_WANT_I32      0x3
#define XSBIND_WANT_I64      0x4
#define XSBIND_WANT_U8       0x5
#define XSBIND_WANT_U16      0x6
#define XSBIND_WANT_U32      0x7
#define XSBIND_WANT_U64      0x8
#define XSBIND_WANT_BOOL     0x9
#define XSBIND_WANT_F32      0xA
#define XSBIND_WANT_F64      0xB
#define XSBIND_WANT_OBJ      0xC
#define XSBIND_WANT_SV       0xD

#if (CFISH_SIZEOF_CHAR == 1)
  #define XSBIND_WANT_CHAR XSBIND_WANT_I8
#else
  #error "Can't build unless sizeof(char) == 1"
#endif

#if (CFISH_SIZEOF_SHORT == 2)
  #define XSBIND_WANT_SHORT XSBIND_WANT_I16
#else
  #error "Can't build unless sizeof(short) == 2"
#endif

#if (CFISH_SIZEOF_INT == 4)
  #define XSBIND_WANT_INT XSBIND_WANT_I32
#elif (CFISH_SIZEOF_INT == 8)
  #define XSBIND_WANT_INT XSBIND_WANT_I64
#else
  #error "Can't build unless sizeof(int) == 4 or sizeof(int) == 8"
#endif

#if (CFISH_SIZEOF_LONG == 4)
  #define XSBIND_WANT_LONG XSBIND_WANT_I32
#elif (CFISH_SIZEOF_LONG == 8)
  #define XSBIND_WANT_LONG XSBIND_WANT_I64
#else
  #error "Can't build unless sizeof(long) == 4 or sizeof(long) == 8"
#endif

#if (CFISH_SIZEOF_SIZE_T == 4)
  #define XSBIND_WANT_SIZE_T XSBIND_WANT_U32
#elif (CFISH_SIZEOF_SIZE_T == 8)
  #define XSBIND_WANT_SIZE_T XSBIND_WANT_U64
#else
  #error "Can't build unless sizeof(size_t) == 4 or sizeof(size_t) == 8"
#endif

#define XSBIND_ALLOT_I8(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_I8, NULL, NULL
#define XSBIND_ALLOT_I16(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_I16, NULL, NULL
#define XSBIND_ALLOT_I32(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_I32, NULL, NULL
#define XSBIND_ALLOT_I64(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_I64, NULL, NULL
#define XSBIND_ALLOT_U8(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_U8, NULL, NULL
#define XSBIND_ALLOT_U16(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_U16, NULL, NULL
#define XSBIND_ALLOT_U32(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_U32, NULL, NULL
#define XSBIND_ALLOT_U64(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_U64, NULL, NULL
#define XSBIND_ALLOT_BOOL(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_BOOL, NULL, NULL
#define XSBIND_ALLOT_CHAR(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_CHAR, NULL, NULL
#define XSBIND_ALLOT_SHORT(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_SHORT, NULL, NULL
#define XSBIND_ALLOT_INT(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_INT, NULL, NULL
#define XSBIND_ALLOT_LONG(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_LONG, NULL, NULL
#define XSBIND_ALLOT_SIZE_T(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_SIZE_T, NULL, NULL
#define XSBIND_ALLOT_F32(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_F32, NULL, NULL
#define XSBIND_ALLOT_F64(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_F64, NULL, NULL
#define XSBIND_ALLOT_OBJ(ptr, key, keylen, required, vtable, allocation) \
    ptr, key, keylen, required, XSBIND_WANT_OBJ, vtable, allocation
#define XSBIND_ALLOT_SV(ptr, key, keylen, required) \
    ptr, key, keylen, required, XSBIND_WANT_SV, NULL, NULL

/* Define short names for most of the symbols in this file.  Note that these
 * short names are ALWAYS in effect, since they are only used for Perl and we
 * can be confident they don't conflict with anything.  (It's prudent to use
 * full symbols nevertheless in case someone else defines e.g. a function
 * named "XSBind_sv_defined".)
 */
#define XSBind_new_blank_obj           cfish_XSBind_new_blank_obj
#define XSBind_sv_defined              cfish_XSBind_sv_defined
#define XSBind_sv_to_cfish_obj         cfish_XSBind_sv_to_cfish_obj
#define XSBind_maybe_sv_to_cfish_obj   cfish_XSBind_maybe_sv_to_cfish_obj
#define XSBind_cfish_obj_to_sv         cfish_XSBind_cfish_obj_to_sv
#define XSBind_cfish_obj_to_sv_noinc   cfish_XSBind_cfish_obj_to_sv_noinc
#define XSBind_cfish_to_perl           cfish_XSBind_cfish_to_perl
#define XSBind_perl_to_cfish           cfish_XSBind_perl_to_cfish
#define XSBind_bb_to_sv                cfish_XSBind_bb_to_sv
#define XSBind_str_to_sv               cfish_XSBind_str_to_sv
#define XSBind_trap                    cfish_XSBind_trap
#define XSBind_enable_overload         cfish_XSBind_enable_overload
#define XSBind_allot_params            cfish_XSBind_allot_params
#define ALLOT_I8                       XSBIND_ALLOT_I8
#define ALLOT_I16                      XSBIND_ALLOT_I16
#define ALLOT_I32                      XSBIND_ALLOT_I32
#define ALLOT_I64                      XSBIND_ALLOT_I64
#define ALLOT_U8                       XSBIND_ALLOT_U8
#define ALLOT_U16                      XSBIND_ALLOT_U16
#define ALLOT_U32                      XSBIND_ALLOT_U32
#define ALLOT_U64                      XSBIND_ALLOT_U64
#define ALLOT_BOOL                     XSBIND_ALLOT_BOOL
#define ALLOT_CHAR                     XSBIND_ALLOT_CHAR
#define ALLOT_SHORT                    XSBIND_ALLOT_SHORT
#define ALLOT_INT                      XSBIND_ALLOT_INT
#define ALLOT_LONG                     XSBIND_ALLOT_LONG
#define ALLOT_SIZE_T                   XSBIND_ALLOT_SIZE_T
#define ALLOT_F32                      XSBIND_ALLOT_F32
#define ALLOT_F64                      XSBIND_ALLOT_F64
#define ALLOT_OBJ                      XSBIND_ALLOT_OBJ
#define ALLOT_SV                       XSBIND_ALLOT_SV

/* Strip the prefix from some common ClownFish symbols where we know there's
 * no conflict with Perl.  It's a little inconsistent to do this rather than
 * leave all symbols at full size, but the succinctness is worth it.
 */
#define THROW            CFISH_THROW
#define WARN             CFISH_WARN

#ifdef __cplusplus
}
#endif

#endif // H_CFISH_XSBIND


