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


MODULE = Clownfish    PACKAGE = Clownfish::Type

SV*
_new(klass, flags, parcel, specifier, indirection, c_string)
    const char *klass;
    int flags;
    SV *parcel;
    const char *specifier;
    int indirection;
    const char *c_string;
CODE:
    CFCType *self = CFCType_new(flags, parcel, specifier, indirection, 
        c_string);
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCType *self;
PPCODE:
    CFCType_destroy(self);

bool
CONST(...)
CODE:
    RETVAL = CFCTYPE_CONST;
OUTPUT: RETVAL

bool
NULLABLE(...)
CODE:
    RETVAL = CFCTYPE_NULLABLE;
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
        case 4:
            retval = newSVsv((SV*)CFCType_get_parcel(self));
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
    END_SET_OR_GET_SWITCH
}


