/* XSBind.h -- Functions to help bind Lucy to Perl XS api.
 */

#ifndef H_LUCY_XSBIND
#define H_LUCY_XSBIND 1

#include "charmony.h"
#include "Lucy/Obj.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/Hash.h"
#include "Lucy/Object/VArray.h"
#include "Lucy/Object/VTable.h"

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define NEED_newRV_noinc_GLOBAL
#include "ppport.h"

/** Given either a class name or a perl object, manufacture a new KS
 * object suitable for supplying to a lucy_Foo_init() function.
 */
lucy_Obj*
lucy_XSBind_new_blank_obj(SV *either_sv);

/** Test whether an SV is defined.  Handles "get" magic, unlike SvOK on its
 * own.
 */
static CHY_INLINE chy_bool_t
lucy_XSBind_sv_defined(SV *sv)
{
    if (!sv || !SvANY(sv)) { return false; }
    if (SvGMAGICAL(sv)) { mg_get(sv); }
    return SvOK(sv);
}

/** If the SV contains a KS object which passes an "isa" test against the
 * passed-in VTable, return a pointer to it.  If not, but <code>zcb</code> is
 * supplied and a ZombieCharBuf would satisfy the "isa" test, stringify the
 * SV, assign its string to <code>zcb</code> and return <code>zcb</code>
 * instead.  If all else fails, throw an exception.
 */
lucy_Obj*
lucy_XSBind_sv_to_lucy_obj(SV *sv, lucy_VTable *vtable, 
                           lucy_ZombieCharBuf *zcb);

/** As XSBind_sv_to_lucy_obj above, but returns NULL instead of throwing an
 * exception.
 */
lucy_Obj*
lucy_XSBind_maybe_sv_to_lucy_obj(SV *sv, lucy_VTable *vtable,
                                 lucy_ZombieCharBuf *zcb);

/** Derive an SV from a Lucy object.  If the KS object is NULL, the SV
 * will be undef.
 *
 * The new SV has single refcount for which the caller must take
 * responsibility.
 */
static CHY_INLINE SV*
lucy_XSBind_lucy_obj_to_sv(lucy_Obj *obj)
{
    return obj ? Lucy_Obj_To_Host(obj) : newSV(0);
}

/** XSBind_lucy_obj_to_sv, with a cast. 
 */
#define LUCY_OBJ_TO_SV(_obj) lucy_XSBind_lucy_obj_to_sv((lucy_Obj*)_obj)

/** As XSBind_lucy_obj_to_sv above, except decrements the object's refcount
 * after creating the SV. This is useful when the KS expression creates a new
 * refcount, e.g.  a call to a constructor.
 */
static CHY_INLINE SV*
lucy_XSBind_lucy_obj_to_sv_noinc(lucy_Obj *obj)
{
    SV *retval;
    if (obj) {
        retval = Lucy_Obj_To_Host(obj);
        Lucy_Obj_Dec_RefCount(obj);
    }
    else {
        retval = newSV(0);
    }
    return retval;
}

/** XSBind_lucy_obj_to_sv_noinc, with a cast. 
 */
#define LUCY_OBJ_TO_SV_NOINC(_obj) \
    lucy_XSBind_lucy_obj_to_sv_noinc((lucy_Obj*)_obj)

/** Deep conversion of KS objects to Perl objects -- CharBufs to UTF-8 SVs,
 * ByteBufs to SVs, VArrays to Perl array refs, Hashes to Perl hashrefs, and
 * any other object to a Perl object wrapping the KS Obj.
 */
SV*
lucy_XSBind_lucy_to_perl(lucy_Obj *obj);

/** Deep conversion of Perl data structures to KS objects -- Perl hash to
 * Hash*, Perl array to VArray*, Lucy objects stripped of their
 * wrappers, and everything else stringified and turned to a CharBuf.
 */
lucy_Obj*
lucy_XSBind_perl_to_lucy(SV *sv);

/** Convert a CharBuf into a new UTF-8 string SV.
 */
SV*
lucy_XSBind_cb_to_sv(const lucy_CharBuf *cb);

/** Process hash-style params passed to an XS subroutine.  The varargs must
 * come batched in groups of three: an SV**, the name of the parameter, and
 * length of the paramter name.  A NULL pointer terminates the list:
 *
 *     lucy_XSBind_allot_params(stack, start, num_stack_elems, 
 *         "Lucy::Search::TermQuery::new_PARAMS", 
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
lucy_XSBind_allot_params(SV** stack, chy_i32_t start, 
                         chy_i32_t num_stack_elems, 
                         char* params_hash_name, ...);

/* Define short names for all the functions in this file.  Note that these
 * short names are ALWAYS in effect, since they are only used for Perl and we
 * can be confident they don't conflict with anything.  (It's prudent to use
 * full symbols nevertheless in case someone else defines e.g. a function
 * named "XSBind_sv_defined".)
 */
#define XSBind_new_blank_obj        lucy_XSBind_new_blank_obj
#define XSBind_sv_defined           lucy_XSBind_sv_defined
#define XSBind_sv_to_lucy_obj       lucy_XSBind_sv_to_lucy_obj
#define XSBind_maybe_sv_to_lucy_obj lucy_XSBind_maybe_sv_to_lucy_obj
#define XSBind_lucy_obj_to_sv       lucy_XSBind_lucy_obj_to_sv
#define XSBind_lucy_obj_to_sv_noinc lucy_XSBind_lucy_obj_to_sv_noinc
#define XSBind_lucy_to_perl         lucy_XSBind_lucy_to_perl
#define XSBind_perl_to_lucy         lucy_XSBind_perl_to_lucy
#define XSBind_cb_to_sv             lucy_XSBind_cb_to_sv
#define XSBind_enable_overload      lucy_XSBind_enable_overload
#define XSBind_allot_params         lucy_XSBind_allot_params

/* Strip the prefix from some common lucy_ symbols where we know there's no
 * conflict with Perl.  It's a little inconsistent to do this rather than
 * leave all symbols at full size, but the succinctness is worth it.
 */
#define THROW            LUCY_THROW
#define WARN             LUCY_WARN
#define OVERRIDDEN       LUCY_OVERRIDDEN

#endif /* H_LUCY_XSBIND */

/**
 * Copyright 2009 The Apache Software Foundation
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

