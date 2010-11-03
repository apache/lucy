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

#ifdef __cplusplus
extern "C" {
#endif

#include "charmony.h"
#include "KinoSearch/Object/Obj.h"
#include "KinoSearch/Object/ByteBuf.h"
#include "KinoSearch/Object/CharBuf.h"
#include "KinoSearch/Object/Err.h"
#include "KinoSearch/Object/Hash.h"
#include "KinoSearch/Object/Num.h"
#include "KinoSearch/Object/VArray.h"
#include "KinoSearch/Object/VTable.h"

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define NEED_newRV_noinc_GLOBAL
#include "ppport.h"

/** Given either a class name or a perl object, manufacture a new Clownfish
 * object suitable for supplying to a cfish_Foo_init() function.
 */
cfish_Obj*
cfish_XSBind_new_blank_obj(SV *either_sv);

/** Test whether an SV is defined.  Handles "get" magic, unlike SvOK on its
 * own.
 */
static CHY_INLINE chy_bool_t
cfish_XSBind_sv_defined(SV *sv)
{
    if (!sv || !SvANY(sv)) { return false; }
    if (SvGMAGICAL(sv)) { mg_get(sv); }
    return SvOK(sv);
}

/** If the SV contains a Clownfish object which passes an "isa" test against the
 * passed-in VTable, return a pointer to it.  If not, but
 * <code>allocation</code> is non-NULL and a ZombieCharBuf would satisfy the
 * "isa" test, stringify the SV, create a ZombieCharBuf using
 * <code>allocation</code>, assign the SV's string to it, and return that
 * instead.  If all else fails, throw an exception.
 */
cfish_Obj*
cfish_XSBind_sv_to_cfish_obj(SV *sv, cfish_VTable *vtable, void *allocation);

/** As XSBind_sv_to_cfish_obj above, but returns NULL instead of throwing an
 * exception.
 */
cfish_Obj*
cfish_XSBind_maybe_sv_to_cfish_obj(SV *sv, cfish_VTable *vtable,
                                   void *allocation);


/** Derive an SV from a KinoSearch object.  If the Clownfish object is NULL, the SV
 * will be undef.
 *
 * The new SV has single refcount for which the caller must take
 * responsibility.
 */
static CHY_INLINE SV*
cfish_XSBind_cfish_obj_to_sv(cfish_Obj *obj)
{
    return obj ? (SV*)Cfish_Obj_To_Host(obj) : newSV(0);
}

/** XSBind_cfish_obj_to_sv, with a cast. 
 */
#define CFISH_OBJ_TO_SV(_obj) cfish_XSBind_cfish_obj_to_sv((cfish_Obj*)_obj)

/** As XSBind_cfish_obj_to_sv above, except decrements the object's refcount
 * after creating the SV. This is useful when the Clownfish expression creates a new
 * refcount, e.g.  a call to a constructor.
 */
static CHY_INLINE SV*
cfish_XSBind_cfish_obj_to_sv_noinc(cfish_Obj *obj)
{
    SV *retval;
    if (obj) {
        retval = (SV*)Cfish_Obj_To_Host(obj);
        Cfish_Obj_Dec_RefCount(obj);
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

/** Deep conversion of Clownfish objects to Perl objects -- CharBufs to UTF-8
 * SVs, ByteBufs to SVs, VArrays to Perl array refs, Hashes to Perl hashrefs,
 * and any other object to a Perl object wrapping the Clownfish Obj.
 */
SV*
cfish_XSBind_cfish_to_perl(cfish_Obj *obj);

/** Deep conversion of Perl data structures to Clownfish objects -- Perl hash
 * to Hash, Perl array to VArray, Clownfish objects stripped of their
 * wrappers, and everything else stringified and turned to a CharBuf.
 */
cfish_Obj*
cfish_XSBind_perl_to_cfish(SV *sv);

/** Convert a ByteBuf into a new string SV.
 */
SV*
cfish_XSBind_bb_to_sv(const cfish_ByteBuf *bb);

/** Convert a CharBuf into a new UTF-8 string SV.
 */
SV*
cfish_XSBind_cb_to_sv(const cfish_CharBuf *cb);

/** Turn on overloading for the supplied Perl object and its class.
 */
void
cfish_XSBind_enable_overload(void *pobj);

/** Process hash-style params passed to an XS subroutine.  The varargs must
 * come batched in groups of three: an SV**, the name of the parameter, and
 * length of the paramter name.  A NULL pointer terminates the list:
 *
 *     cfish_XSBind_allot_params(stack, start, num_stack_elems, 
 *         "KinoSearch::Search::TermQuery::new_PARAMS", 
 *          &field_sv, "field", 5,
 *          &term_sv, "term", 4,
 *          NULL);
 *
 * All labeled params found on the stack will be assigned to the appropriate
 * SV**.
 *
 * @param stack The Perl stack.
 * @param start Where on the Perl stack to start looking for params.  For
 * methods, this would typically be 1; for functions, most likely 0.
 * @param num_stack_elems The number of arguments passed to the Perl function
 * (generally, the XS variable "items").
 * @param params_hash_name The name of a package global hash.  Any param
 * labels which are not present in this hash will trigger an exception.
 */
void
cfish_XSBind_allot_params(SV** stack, int32_t start, 
                          int32_t num_stack_elems, 
                          char* params_hash_name, ...);

/* Define short names for all the functions in this file.  Note that these
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
#define XSBind_cb_to_sv                cfish_XSBind_cb_to_sv
#define XSBind_enable_overload         cfish_XSBind_enable_overload
#define XSBind_allot_params            cfish_XSBind_allot_params

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


