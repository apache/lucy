/* XSBind.h -- Functions to help bind KinoSearch to Perl XS api.
 */

#ifndef H_KINO_XSBIND
#define H_KINO_XSBIND 1

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

/** Given either a class name or a perl object, manufacture a new KS
 * object suitable for supplying to a kino_Foo_init() function.
 */
kino_Obj*
kino_XSBind_new_blank_obj(SV *either_sv);

/** Test whether an SV is defined.  Handles "get" magic, unlike SvOK on its
 * own.
 */
static CHY_INLINE chy_bool_t
kino_XSBind_sv_defined(SV *sv)
{
    if (!sv || !SvANY(sv)) { return false; }
    if (SvGMAGICAL(sv)) { mg_get(sv); }
    return SvOK(sv);
}

/** If the SV contains a KS object which passes an "isa" test against the
 * passed-in VTable, return a pointer to it.  If not, but
 * <code>allocation</code> is non-NULL and a ZombieCharBuf would satisfy the
 * "isa" test, stringify the SV, create a ZombieCharBuf using
 * <code>allocation</code>, assign the SV's string to it, and return that
 * instead.  If all else fails, throw an exception.
 */
kino_Obj*
kino_XSBind_sv_to_kino_obj(SV *sv, kino_VTable *vtable, void *allocation);

/** As XSBind_sv_to_kino_obj above, but returns NULL instead of throwing an
 * exception.
 */
kino_Obj*
kino_XSBind_maybe_sv_to_kino_obj(SV *sv, kino_VTable *vtable,
                                 void *allocation);


/** Derive an SV from a KinoSearch object.  If the KS object is NULL, the SV
 * will be undef.
 *
 * The new SV has single refcount for which the caller must take
 * responsibility.
 */
static CHY_INLINE SV*
kino_XSBind_kino_obj_to_sv(kino_Obj *obj)
{
    return obj ? (SV*)Kino_Obj_To_Host(obj) : newSV(0);
}

/** XSBind_kino_obj_to_sv, with a cast. 
 */
#define KINO_OBJ_TO_SV(_obj) kino_XSBind_kino_obj_to_sv((kino_Obj*)_obj)

/** As XSBind_kino_obj_to_sv above, except decrements the object's refcount
 * after creating the SV. This is useful when the KS expression creates a new
 * refcount, e.g.  a call to a constructor.
 */
static CHY_INLINE SV*
kino_XSBind_kino_obj_to_sv_noinc(kino_Obj *obj)
{
    SV *retval;
    if (obj) {
        retval = (SV*)Kino_Obj_To_Host(obj);
        Kino_Obj_Dec_RefCount(obj);
    }
    else {
        retval = newSV(0);
    }
    return retval;
}

/** XSBind_kino_obj_to_sv_noinc, with a cast. 
 */
#define KINO_OBJ_TO_SV_NOINC(_obj) \
    kino_XSBind_kino_obj_to_sv_noinc((kino_Obj*)_obj)

/** Deep conversion of KS objects to Perl objects -- CharBufs to UTF-8 SVs,
 * ByteBufs to SVs, VArrays to Perl array refs, Hashes to Perl hashrefs, and
 * any other object to a Perl object wrapping the KS Obj.
 */
SV*
kino_XSBind_kino_to_perl(kino_Obj *obj);

/** Deep conversion of Perl data structures to KS objects -- Perl hash to
 * Hash*, Perl array to VArray*, KinoSearch objects stripped of their
 * wrappers, and everything else stringified and turned to a CharBuf.
 */
kino_Obj*
kino_XSBind_perl_to_kino(SV *sv);

/** Convert a ByteBuf into a new string SV.
 */
SV*
kino_XSBind_bb_to_sv(const kino_ByteBuf *bb);

/** Convert a CharBuf into a new UTF-8 string SV.
 */
SV*
kino_XSBind_cb_to_sv(const kino_CharBuf *cb);

/** Turn on overloading for the supplied Perl object and its class.
 */
void
kino_XSBind_enable_overload(void *pobj);

/** Process hash-style params passed to an XS subroutine.  The varargs must
 * come batched in groups of three: an SV**, the name of the parameter, and
 * length of the paramter name.  A NULL pointer terminates the list:
 *
 *     kino_XSBind_allot_params(stack, start, num_stack_elems, 
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
kino_XSBind_allot_params(SV** stack, int32_t start, 
                         int32_t num_stack_elems, 
                         char* params_hash_name, ...);

/* Define short names for all the functions in this file.  Note that these
 * short names are ALWAYS in effect, since they are only used for Perl and we
 * can be confident they don't conflict with anything.  (It's prudent to use
 * full symbols nevertheless in case someone else defines e.g. a function
 * named "XSBind_sv_defined".)
 */
#define XSBind_new_blank_obj        kino_XSBind_new_blank_obj
#define XSBind_sv_defined           kino_XSBind_sv_defined
#define XSBind_sv_to_kino_obj       kino_XSBind_sv_to_kino_obj
#define XSBind_maybe_sv_to_kino_obj kino_XSBind_maybe_sv_to_kino_obj
#define XSBind_kino_obj_to_sv       kino_XSBind_kino_obj_to_sv
#define XSBind_kino_obj_to_sv_noinc kino_XSBind_kino_obj_to_sv_noinc
#define XSBind_kino_to_perl         kino_XSBind_kino_to_perl
#define XSBind_perl_to_kino         kino_XSBind_perl_to_kino
#define XSBind_bb_to_sv             kino_XSBind_bb_to_sv
#define XSBind_cb_to_sv             kino_XSBind_cb_to_sv
#define XSBind_enable_overload      kino_XSBind_enable_overload
#define XSBind_allot_params         kino_XSBind_allot_params

/* Strip the prefix from some common kino_ symbols where we know there's no
 * conflict with Perl.  It's a little inconsistent to do this rather than
 * leave all symbols at full size, but the succinctness is worth it.
 */
#define THROW            KINO_THROW
#define WARN             KINO_WARN

#ifdef __cplusplus
}
#endif

#endif // H_KINO_XSBIND 

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

