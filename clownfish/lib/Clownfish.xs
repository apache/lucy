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

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "CFC.h"

/* Rather than provide an XSUB for each accessor, we can use one multipath
 * accessor function per class, with several Perl-space aliases.  All set
 * functions have odd-numbered aliases, and all get functions have
 * even-numbered aliases.  These two macros serve as bookends for the switch
 * function.
 */
#define START_SET_OR_GET_SWITCH \
    SV *retval = &PL_sv_undef; \
    /* If called as a setter, make sure the extra arg is there. */ \
    if (ix % 2 == 1) { \
        if (items != 2) { croak("usage: $object->set_xxxxxx($val)"); } \
    } \
    else { \
        if (items != 1) { croak("usage: $object->get_xxxxx()"); } \
    } \
    switch (ix) {

#define END_SET_OR_GET_SWITCH \
        default: croak("Internal error. ix: %d", ix); \
    } \
    if (ix % 2 == 0) { \
        XPUSHs( sv_2mortal(retval) ); \
        XSRETURN(1); \
    } \
    else { \
        XSRETURN(0); \
    } 

MODULE = Clownfish    PACKAGE = Clownfish::CBlock

SV*
_new(klass, contents)
    const char *klass;
    const char *contents;
CODE:
    CFCCBlock *self = CFCCBlock_new(contents);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCCBlock *self;
PPCODE:
    CFCCBlock_destroy(self);

void
_set_or_get(self, ...)
    CFCCBlock *self;
ALIAS:
    get_contents = 2
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *contents = CFCCBlock_get_contents(self);
                retval = newSVpvn(contents, strlen(contents));
            }
            break;
    END_SET_OR_GET_SWITCH
}

MODULE = Clownfish    PACKAGE = Clownfish::Class

SV*
_new(klass, parcel, exposure, class_name_sv, class_cnick_sv, micro_sym_sv)
    const char *klass;
    CFCParcel *parcel;
    const char *exposure;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
CODE:
    const char *class_name = SvOK(class_name_sv) 
                           ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick = SvOK(class_cnick_sv) 
                            ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym = SvOK(micro_sym_sv) 
                            ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCClass *self = CFCClass_new(parcel, exposure, class_name, class_cnick,
        micro_sym);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
_destroy(self)
    CFCClass *self;
PPCODE:
    CFCClass_destroy(self);


MODULE = Clownfish    PACKAGE = Clownfish::DocuComment

SV*
_new(klass, description, brief, long_description, param_names, param_docs, retval_sv)
    const char *klass;
    const char *description;
    const char *brief;
    const char *long_description;
    SV *param_names;
    SV *param_docs;
    SV *retval_sv;
CODE:
    const char *retval = SvOK(retval_sv) ? SvPV_nolen(retval_sv) : NULL;
    CFCDocuComment *self = CFCDocuComment_new(description, brief,
        long_description, param_names, param_docs, retval);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCDocuComment *self;
PPCODE:
    CFCDocuComment_destroy(self);

void
_set_or_get(self, ...)
    CFCDocuComment *self;
ALIAS:
    get_description = 2
    get_brief       = 4
    get_long        = 6
    get_param_names = 8
    get_param_docs  = 10
    get_retval      = 12
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *description = CFCDocuComment_get_description(self);
                retval = newSVpvn(description, strlen(description));
            }
            break;
        case 4: {
                const char *brief = CFCDocuComment_get_brief(self);
                retval = newSVpvn(brief, strlen(brief));
            }
            break;
        case 6: {
                const char *long_description = CFCDocuComment_get_long(self);
                retval = newSVpvn(long_description, strlen(long_description));
            }
            break;
        case 8:
            retval = newSVsv((SV*)CFCDocuComment_get_param_names(self));
            break;
        case 10:
            retval = newSVsv((SV*)CFCDocuComment_get_param_docs(self));
            break;
        case 12: {
                const char *rv = CFCDocuComment_get_retval(self);
                retval = rv ? newSVpvn(rv, strlen(rv)) : newSV(0);
            }
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::Function

SV*
_new(klass, parcel, exposure, class_name_sv, class_cnick_sv, micro_sym_sv, return_type, param_list, docucomment, is_inline)
    const char *klass;
    CFCParcel *parcel;
    const char *exposure;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
    SV *return_type;
    SV *param_list;
    SV *docucomment;
    int is_inline;
CODE:
    const char *class_name = SvOK(class_name_sv) 
                           ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick = SvOK(class_cnick_sv) 
                            ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym = SvOK(micro_sym_sv) 
                            ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCFunction *self = CFCFunction_new(parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, is_inline);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCFunction *self;
PPCODE:
    CFCFunction_destroy(self);

void
_set_or_get(self, ...)
    CFCFunction *self;
ALIAS:
    get_return_type    = 2
    get_param_list     = 4
    get_docucomment    = 6
    inline             = 8
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2:
            retval = newSVsv((SV*)CFCFunction_get_return_type(self));
            break;
        case 4:
            retval = newSVsv((SV*)CFCFunction_get_param_list(self));
            break;
        case 6:
            retval = newSVsv((SV*)CFCFunction_get_docucomment(self));
            break;
        case 8:
            retval = newSViv(CFCFunction_inline(self));
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::Method

SV*
_new(klass, parcel, exposure, class_name_sv, class_cnick_sv, micro_sym_sv, return_type, param_list, docucomment, is_inline, macro_sym, is_final, is_abstract)
    const char *klass;
    CFCParcel *parcel;
    const char *exposure;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
    SV *return_type;
    SV *param_list;
    SV *docucomment;
    int is_inline;
    const char *macro_sym;
    int is_final;
    int is_abstract;
CODE:
    const char *class_name = SvOK(class_name_sv) 
                           ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick = SvOK(class_cnick_sv) 
                            ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym = SvOK(micro_sym_sv) 
                            ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCMethod *self = CFCMethod_new(parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, is_inline, macro_sym, 
        is_final, is_abstract);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCMethod *self;
PPCODE:
    CFCMethod_destroy(self);

void
_set_or_get(self, ...)
    CFCMethod *self;
ALIAS:
    get_macro_sym      = 2
    _set_short_typedef = 3
    short_typedef      = 4
    abstract           = 6
    _set_novel         = 7
    novel              = 8
    final              = 10;
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *macro_sym = CFCMethod_get_macro_sym(self);
                retval = newSVpvn(macro_sym, strlen(macro_sym));
            }
            break;
        case 3:
            CFCMethod_set_short_typedef(self, SvPV_nolen(ST(1)));
            break;
        case 4: {
                const char *short_typedef = CFCMethod_short_typedef(self);
                retval = newSVpvn(short_typedef, strlen(short_typedef));
            }
            break;
        case 6:
            retval = newSViv(CFCMethod_abstract(self));
            break;
        case 7:
            CFCMethod_set_novel(self, !!SvIV(ST(1)));
            break;
        case 8:
            retval = newSViv(CFCMethod_novel(self));
            break;
        case 10:
            retval = newSViv(CFCMethod_final(self));
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::ParamList

SV*
_new(klass, variables, values, variadic)
    const char *klass;
    SV *variables;
    SV *values;
    int variadic;
CODE:
    CFCParamList *self = CFCParamList_new(variables, values, variadic);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCParamList *self;
PPCODE:
    CFCParamList_destroy(self);

void
_set_or_get(self, ...)
    CFCParamList *self;
ALIAS:
    get_variables      = 2
    get_initial_values = 4
    variadic           = 6
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2:
            retval = newSVsv((SV*)CFCParamList_get_variables(self));
            break;
        case 4:
            retval = newSVsv((SV*)CFCParamList_get_initial_values(self));
            break;
        case 6:
            retval = newSViv(CFCParamList_variadic(self));
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::Parcel

SV*
_singleton(klass, name_sv, cnick_sv)
    const char *klass;
    SV *name_sv;
    SV *cnick_sv;
CODE:
    const char *name  = SvOK(name_sv)  ? SvPV_nolen(name_sv)  : NULL;
    const char *cnick = SvOK(cnick_sv) ? SvPV_nolen(cnick_sv) : NULL;
    CFCParcel *self = CFCParcel_singleton(name, cnick);
    SV *inner_object = SvRV((SV*)CFCParcel_get_perl_object(self));
    RETVAL = newRV(inner_object);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCParcel *self;
PPCODE:
    CFCParcel_destroy(self);

int
equals(self, other)
    CFCParcel *self;
    CFCParcel *other;
CODE:
    RETVAL = CFCParcel_equals(self, other);
OUTPUT: RETVAL

SV*
default_parcel(...)
CODE:
    CFCParcel *default_parcel = CFCParcel_default_parcel();
    SV *inner_obj = SvRV((SV*)CFCParcel_get_perl_object(default_parcel));
    RETVAL = newRV(inner_obj);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCParcel *self;
ALIAS:
    get_name   = 2
    get_cnick  = 4
    get_prefix = 6
    get_Prefix = 8
    get_PREFIX = 10
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *name = CFCParcel_get_name(self);
                retval = newSVpvn(name, strlen(name));
            }
            break;
        case 4: {
                const char *cnick = CFCParcel_get_cnick(self);
                retval = newSVpvn(cnick, strlen(cnick));
            }
            break;
        case 6: {
                const char *value = CFCParcel_get_prefix(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 8: {
                const char *value = CFCParcel_get_Prefix(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 10: {
                const char *value = CFCParcel_get_PREFIX(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::Symbol

SV*
_new(klass, parcel, exposure, class_name_sv, class_cnick_sv, micro_sym_sv)
    const char *klass;
    CFCParcel *parcel;
    const char *exposure;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
CODE:
    const char *class_name = SvOK(class_name_sv) 
                           ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick = SvOK(class_cnick_sv) 
                            ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym = SvOK(micro_sym_sv) 
                            ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCSymbol *self = CFCSymbol_new(parcel, exposure, class_name, class_cnick,
        micro_sym);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

int
equals(self, other)
    CFCSymbol *self;
    CFCSymbol *other;
CODE:
    RETVAL = CFCSymbol_equals(self, other);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCSymbol *self;
PPCODE:
    CFCSymbol_destroy(self);

void
_set_or_get(self, ...)
    CFCSymbol *self;
ALIAS:
    get_parcel      = 2
    get_class_name  = 4
    get_class_cnick = 6
    get_exposure    = 8
    micro_sym       = 10
    get_prefix      = 12
    get_Prefix      = 14
    get_PREFIX      = 16
    public          = 18
    private         = 20
    parcel          = 22
    local           = 24
    short_sym       = 26
    full_sym        = 28
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                struct CFCParcel *parcel = CFCSymbol_get_parcel(self);
                retval = newSVsv((SV*)CFCParcel_get_perl_object(parcel));
            }
            break;
        case 4: {
                const char *class_name = CFCSymbol_get_class_name(self);
                retval = class_name 
                       ? newSVpvn(class_name, strlen(class_name))
                       : newSV(0);
            }
            break;
        case 6: {
                const char *class_cnick = CFCSymbol_get_class_cnick(self);
                retval = class_cnick 
                       ? newSVpvn(class_cnick, strlen(class_cnick))
                       : newSV(0);
            }
            break;
        case 8: {
                const char *exposure = CFCSymbol_get_exposure(self);
                retval = newSVpvn(exposure, strlen(exposure));
            }
            break;
        case 10: {
                const char *micro_sym = CFCSymbol_micro_sym(self);
                retval = newSVpvn(micro_sym, strlen(micro_sym));
            }
            break;
        case 12: {
                const char *value = CFCSymbol_get_prefix(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 14: {
                const char *value = CFCSymbol_get_Prefix(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 16: {
                const char *value = CFCSymbol_get_PREFIX(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 18:
            retval = newSViv(CFCSymbol_public(self));
            break;
        case 20:
            retval = newSViv(CFCSymbol_private(self));
            break;
        case 22:
            retval = newSViv(CFCSymbol_parcel(self));
            break;
        case 24:
            retval = newSViv(CFCSymbol_local(self));
            break;
        case 26: {
                const char *short_sym = CFCSymbol_short_sym(self);
                retval = newSVpvn(short_sym, strlen(short_sym));
            }
            break;
        case 28: {
                const char *full_sym = CFCSymbol_full_sym(self);
                retval = newSVpvn(full_sym, strlen(full_sym));
            }
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::Type

SV*
_new(klass, flags, parcel_sv, specifier, indirection, c_string)
    const char *klass;
    int flags;
    SV *parcel_sv;
    const char *specifier;
    int indirection;
    const char *c_string;
CODE:
    CFCParcel *parcel = NULL;
    if (SvOK(parcel_sv) && sv_derived_from(parcel_sv, "Clownfish::Parcel")) {
        IV objint = SvIV((SV*)SvRV(parcel_sv));
        parcel = INT2PTR(CFCParcel*, objint);
    }   
    CFCType *self = CFCType_new(flags, parcel, specifier, indirection, 
        c_string);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

SV*
_new_integer(klass, flags, specifier)
    const char *klass;
    int flags;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_integer(flags, specifier);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

SV*
_new_float(klass, flags, specifier)
    const char *klass;
    int flags;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_float(flags, specifier);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

SV*
_new_object(klass, flags, parcel_sv, specifier, indirection)
    const char *klass;
    int flags;
    SV *parcel_sv;
    const char *specifier;
    int indirection;
CODE:
    CFCParcel *parcel = NULL;
    if (SvOK(parcel_sv) && sv_derived_from(parcel_sv, "Clownfish::Parcel")) {
        IV objint = SvIV((SV*)SvRV(parcel_sv));
        parcel = INT2PTR(CFCParcel*, objint);
    }   
    CFCType *self = CFCType_new_object(flags, parcel, specifier, indirection);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

SV*
_new_composite(klass, flags, child_sv, indirection, array)
    const char *klass;
    int flags;
    SV *child_sv;
    int indirection;
    const char *array;
CODE:
    CFCType *child = NULL;
    if (SvOK(child_sv) && sv_derived_from(child_sv, "Clownfish::Type")) {
        IV objint = SvIV((SV*)SvRV(child_sv));
        child = INT2PTR(CFCType*, objint);
        // Intentionally leak a refcount for child type so that it does not
        // get destroyed.
        SvREFCNT_inc(SvRV(child_sv));
    }
    else {
        croak("Param 'child' not a Clownfish::Type");
    }
    CFCType *self = CFCType_new_composite(flags, child, indirection, array);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

SV*
_new_void(klass, is_const)
    const char *klass;
    int is_const;
CODE:
    CFCType *self = CFCType_new_void(is_const);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

SV*
_new_va_list(klass)
    const char *klass;
CODE:
    CFCType *self = CFCType_new_va_list();
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

SV*
_new_arbitrary(klass, parcel_sv, specifier)
    const char *klass;
    SV *parcel_sv;
    const char *specifier;
CODE:
    CFCParcel *parcel = NULL;
    if (SvOK(parcel_sv) && sv_derived_from(parcel_sv, "Clownfish::Parcel")) {
        IV objint = SvIV((SV*)SvRV(parcel_sv));
        parcel = INT2PTR(CFCParcel*, objint);
    }   
    CFCType *self = CFCType_new_arbitrary(parcel, specifier);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCType *self;
PPCODE:
    CFCType_destroy(self);

int
equals(self, other)
    CFCType *self;
    CFCType *other;
CODE:
    RETVAL = CFCType_equals(self, other);
OUTPUT: RETVAL

int
similar(self, other)
    CFCType *self;
    CFCType *other;
CODE:
    RETVAL = CFCType_similar(self, other);
OUTPUT: RETVAL

unsigned
CONST(...)
CODE:
    RETVAL = CFCTYPE_CONST;
OUTPUT: RETVAL

unsigned
NULLABLE(...)
CODE:
    RETVAL = CFCTYPE_NULLABLE;
OUTPUT: RETVAL

unsigned
INCREMENTED(...)
CODE:
    RETVAL = CFCTYPE_INCREMENTED;
OUTPUT: RETVAL

unsigned
DECREMENTED(...)
CODE:
    RETVAL = CFCTYPE_DECREMENTED;
OUTPUT: RETVAL

unsigned
VOID(...)
CODE:
    RETVAL = CFCTYPE_VOID;
OUTPUT: RETVAL

unsigned
OBJECT(...)
CODE:
    RETVAL = CFCTYPE_OBJECT;
OUTPUT: RETVAL

unsigned
PRIMITIVE(...)
CODE:
    RETVAL = CFCTYPE_PRIMITIVE;
OUTPUT: RETVAL

unsigned
INTEGER(...)
CODE:
    RETVAL = CFCTYPE_INTEGER;
OUTPUT: RETVAL

unsigned
FLOATING(...)
CODE:
    RETVAL = CFCTYPE_FLOATING;
OUTPUT: RETVAL

unsigned
STRING_TYPE(...)
CODE:
    RETVAL = CFCTYPE_STRING_TYPE;
OUTPUT: RETVAL

unsigned
VA_LIST(...)
CODE:
    RETVAL = CFCTYPE_VA_LIST;
OUTPUT: RETVAL

unsigned
ARBITRARY(...)
CODE:
    RETVAL = CFCTYPE_ARBITRARY;
OUTPUT: RETVAL

unsigned
COMPOSITE(...)
CODE:
    RETVAL = CFCTYPE_COMPOSITE;
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCType *self;
ALIAS:
    set_specifier   = 1
    get_specifier   = 2
    get_parcel      = 4
    get_indirection = 6
    set_c_string    = 7
    to_c            = 8
    const           = 10 
    set_nullable    = 11
    nullable        = 12
    is_void         = 14
    is_object       = 16
    is_primitive    = 18
    is_integer      = 20
    is_floating     = 22
    is_string_type  = 24
    is_va_list      = 26
    is_arbitrary    = 28
    is_composite    = 30
    get_width       = 32
    incremented     = 34
    decremented     = 36
    get_array       = 38
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 1:
            CFCType_set_specifier(self, SvPV_nolen(ST(1)));
            break;
        case 2: {
                const char *specifier = CFCType_get_specifier(self);
                retval = newSVpvn(specifier, strlen(specifier));
            }
            break;
        case 4: {
                CFCParcel *parcel = CFCType_get_parcel(self);
                retval = parcel
                       ? newSVsv((SV*)CFCParcel_get_perl_object(parcel))
                       : newSV(0);
            }
            break;
        case 6:
            retval = newSViv(CFCType_get_indirection(self));
            break;
        case 7:
            CFCType_set_c_string(self, SvPV_nolen(ST(1)));
        case 8: {
                const char *c_string = CFCType_to_c(self);
                retval = newSVpvn(c_string, strlen(c_string));
            }
            break;
        case 10:
            retval = newSViv(CFCType_const(self));
            break;
        case 11:
            CFCType_set_nullable(self, !!SvTRUE(ST(1)));
            break;
        case 12:
            retval = newSViv(CFCType_nullable(self));
            break;
        case 14:
            retval = newSViv(CFCType_is_void(self));
            break;
        case 16:
            retval = newSViv(CFCType_is_object(self));
            break;
        case 18:
            retval = newSViv(CFCType_is_primitive(self));
            break;
        case 20:
            retval = newSViv(CFCType_is_integer(self));
            break;
        case 22:
            retval = newSViv(CFCType_is_floating(self));
            break;
        case 24:
            retval = newSViv(CFCType_is_string_type(self));
            break;
        case 26:
            retval = newSViv(CFCType_is_va_list(self));
            break;
        case 28:
            retval = newSViv(CFCType_is_arbitrary(self));
            break;
        case 30:
            retval = newSViv(CFCType_is_composite(self));
            break;
        case 32:
            retval = newSVuv(CFCType_get_width(self));
            break;
        case 34:
            retval = newSVuv(CFCType_incremented(self));
            break;
        case 36:
            retval = newSVuv(CFCType_decremented(self));
            break;
        case 38: {
                const char *array = CFCType_get_array(self);
                retval = array 
                       ? newSVpvn(array, strlen(array))
                       : newSV(0);
            }
            break;
    END_SET_OR_GET_SWITCH
}

MODULE = Clownfish   PACKAGE = Clownfish::Variable

SV*
_new(klass, parcel_sv, exposure, class_name_sv, class_cnick_sv, micro_sym_sv, type_sv)
    const char *klass;
    SV *parcel_sv;
    const char *exposure;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
    SV *type_sv;
CODE:
    const char *class_name = SvOK(class_name_sv) 
                           ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick = SvOK(class_cnick_sv) 
                            ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym = SvOK(micro_sym_sv) 
                            ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCParcel *parcel = NULL;
    if (SvOK(parcel_sv) && sv_derived_from(parcel_sv, "Clownfish::Parcel")) {
        IV objint = SvIV((SV*)SvRV(parcel_sv));
        parcel = INT2PTR(CFCParcel*, objint);
    }
    CFCType *type = NULL;
    if (SvOK(type_sv) && sv_derived_from(type_sv, "Clownfish::Type")) {
        IV objint = SvIV((SV*)SvRV(type_sv));
        type = INT2PTR(CFCType*, objint);
    }
    else {
        croak("Param 'type' is not a Clownfish::Type");
    }
    CFCVariable *self = CFCVariable_new(parcel, exposure, class_name,
        class_cnick, micro_sym, type, type_sv);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCVariable *self;
PPCODE:
    CFCVariable_destroy(self);

int
equals(self, other)
    CFCVariable *self;
    CFCVariable *other;
CODE:
    RETVAL = CFCVariable_equals(self, other);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCVariable *self;
ALIAS:
    get_type          = 2
    local_c           = 4
    global_c          = 6
    local_declaration = 8
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                SV *type_perl_obj = (SV*)CFCVariable_type_perl_obj(self);
                retval = newSVsv(type_perl_obj);
            }
            break;
        case 4: {
                const char *local_c = CFCVariable_local_c(self);
                retval = newSVpvn(local_c, strlen(local_c));
            }
            break;
        case 6: {
                const char *global_c = CFCVariable_global_c(self);
                retval = newSVpvn(global_c, strlen(global_c));
            }
            break;
        case 8: {
                const char *local_dec = CFCVariable_local_declaration(self);
                retval = newSVpvn(local_dec, strlen(local_dec));
            }
            break;
    END_SET_OR_GET_SWITCH
}

