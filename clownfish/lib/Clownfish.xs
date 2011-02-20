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
    RETVAL = newRV(CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
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
    RETVAL = newRV(CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
_destroy(self)
    CFCClass *self;
PPCODE:
    CFCClass_destroy(self);


MODULE = Clownfish    PACKAGE = Clownfish::DocuComment

SV*
parse(klass, text)
    const char *klass;
    const char *text;
CODE:
    CFCDocuComment *self = CFCDocuComment_parse(text);
    RETVAL = newRV(CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
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
        case 8: {
            AV *av = newAV();
            const char **names = CFCDocuComment_get_param_names(self);
            size_t i;
            for (i = 0; names[i] != NULL; i++) {
                SV *val_sv = newSVpvn(names[i], strlen(names[i]));
                av_store(av, i, val_sv);
            }
            retval = newRV((SV*)av);
            SvREFCNT_dec(av);
            break;
        }
        case 10: {
            AV *av = newAV();
            const char **docs = CFCDocuComment_get_param_docs(self);
            size_t i;
            for (i = 0; docs[i] != NULL; i++) {
                SV *val_sv = newSVpvn(docs[i], strlen(docs[i]));
                av_store(av, i, val_sv);
            }
            retval = newRV((SV*)av);
            SvREFCNT_dec(av);
            break;
        }
        case 12: {
                const char *rv = CFCDocuComment_get_retval(self);
                retval = rv ? newSVpvn(rv, strlen(rv)) : newSV(0);
            }
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::File

SV*
_new(klass, source_class)
    const char *klass;
    const char *source_class;
CODE:
    CFCFile *self = CFCFile_new(source_class);
    RETVAL = newRV(CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
_destroy(self)
    CFCFile *self;
PPCODE:
    CFCFile_destroy(self);

void
add_block(self, block)
    CFCFile *self;
    CFCBase *block;
PPCODE:
    CFCFile_add_block(self, block);

void
_set_or_get(self, ...)
    CFCFile *self;
ALIAS:
    set_modified       = 1
    get_modified       = 2
    get_source_class   = 4
    guard_name         = 6
    guard_start        = 8
    guard_close        = 10 
    blocks             = 12
    classes            = 14
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 1: 
            CFCFile_set_modified(self, !!SvTRUE(ST(1)));
            break;
        case 2: 
            retval = newSViv(CFCFile_get_modified(self));
            break;
        case 4: {
                const char *value = CFCFile_get_source_class(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 6: {
                const char *value = CFCFile_guard_name(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 8: {
                const char *value = CFCFile_guard_start(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 10: {
                const char *value = CFCFile_guard_close(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 12: {
            AV *av = newAV();
            CFCBase **blocks = CFCFile_blocks(self);
            size_t i;
            for (i = 0; blocks[i] != NULL; i++) {
                SV *ref = newRV((SV*)CFCBase_get_perl_obj(blocks[i]));
                av_store(av, i, ref);
            }
            retval = newRV((SV*)av);
            SvREFCNT_dec(av);
            break;
        }
        case 14: {
            AV *av = newAV();
            CFCClass **classes = CFCFile_classes(self);
            size_t i;
            for (i = 0; classes[i] != NULL; i++) {
                SV *ref = newRV((SV*)CFCBase_get_perl_obj(
                    (CFCBase*)classes[i]));
                av_store(av, i, ref);
            }
            retval = newRV((SV*)av);
            SvREFCNT_dec(av);
            break;
        }
    END_SET_OR_GET_SWITCH
}

SV*
_gen_path(self, base_dir = NULL)
    CFCFile *self;
    const char *base_dir;
ALIAS:
    c_path       = 1
    h_path       = 2
    cfh_path     = 3
CODE:
{
    size_t buf_size = CFCFile_path_buf_size(self, base_dir);
    RETVAL = newSV(buf_size);
    SvPOK_on(RETVAL);
    char *buf = SvPVX(RETVAL);
    switch(ix) {
        case 1: 
            CFCFile_c_path(self, buf, buf_size, base_dir);
            break;
        case 2: 
            CFCFile_h_path(self, buf, buf_size, base_dir);
            break;
        case 3: 
            CFCFile_cfh_path(self, buf, buf_size, base_dir);
            break;
        default:
            croak("unexpected ix value: %d", ix);
    }
    SvCUR_set(RETVAL, strlen(buf));
}
OUTPUT: RETVAL


MODULE = Clownfish    PACKAGE = Clownfish::Function

SV*
_new(klass, parcel, exposure_sv, class_name_sv, class_cnick_sv, micro_sym_sv, return_type, param_list, docucomment, is_inline)
    const char *klass;
    CFCParcel *parcel;
    SV *exposure_sv;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
    CFCType *return_type;
    CFCParamList *param_list;
    CFCDocuComment *docucomment;
    int is_inline;
CODE:
    const char *exposure = SvOK(exposure_sv) 
                         ? SvPV_nolen(exposure_sv) : NULL;
    const char *class_name = SvOK(class_name_sv) 
                           ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick = SvOK(class_cnick_sv) 
                            ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym = SvOK(micro_sym_sv) 
                            ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCFunction *self = CFCFunction_new(parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, is_inline);
    RETVAL = newRV(CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
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
        case 2: {
                CFCType *type = CFCFunction_get_return_type(self);
                retval = type 
                       ? newRV((SV*)CFCBase_get_perl_obj((CFCBase*)type))
                       : newSV(0);
            }
            break;
        case 4: {
                CFCParamList *param_list = CFCFunction_get_param_list(self);
                retval = param_list 
                       ? newRV((SV*)CFCBase_get_perl_obj((CFCBase*)param_list))
                       : newSV(0);
            }
            break;
        case 6: {
                CFCDocuComment *docucomment 
                    = CFCFunction_get_docucomment(self);
                retval = docucomment 
                       ? newRV((SV*)CFCBase_get_perl_obj((CFCBase*)docucomment))
                       : newSV(0);
            }
            break;
        case 8:
            retval = newSViv(CFCFunction_inline(self));
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish    PACKAGE = Clownfish::Method

SV*
_new(klass, parcel, exposure_sv, class_name_sv, class_cnick_sv, micro_sym_sv, return_type, param_list, docucomment, macro_sym, is_final, is_abstract)
    const char *klass;
    CFCParcel *parcel;
    SV *exposure_sv;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
    CFCType *return_type;
    CFCParamList *param_list;
    CFCDocuComment *docucomment;
    const char *macro_sym;
    int is_final;
    int is_abstract;
CODE:
    const char *exposure = SvOK(exposure_sv) 
                         ? SvPV_nolen(exposure_sv) : NULL;
    const char *class_name = SvOK(class_name_sv) 
                           ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick = SvOK(class_cnick_sv) 
                            ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym = SvOK(micro_sym_sv) 
                            ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCMethod *self = CFCMethod_new(parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, macro_sym, 
        is_final, is_abstract);
    RETVAL = newRV(CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
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
    final              = 10
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
_new(klass, variadic)
    const char *klass;
    int variadic;
CODE:
    CFCParamList *self = CFCParamList_new(variadic);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
DESTROY(self)
    CFCParamList *self;
PPCODE:
    CFCParamList_destroy(self);

void
add_param(self, variable, value_sv)
    CFCParamList *self;
    CFCVariable  *variable;
    SV *value_sv;
PPCODE:
    const char *value = SvOK(value_sv) ? SvPV_nolen(value_sv) : NULL;
    CFCParamList_add_param(self, variable, value);

void
_set_or_get(self, ...)
    CFCParamList *self;
ALIAS:
    get_variables      = 2
    get_initial_values = 4
    variadic           = 6
    num_vars           = 8
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
            AV *av = newAV();
            CFCVariable **vars = CFCParamList_get_variables(self);
            size_t i;
            size_t num_vars = CFCParamList_num_vars(self);
            for (i = 0; i < num_vars; i++) {
                SV *ref = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)vars[i]));
                av_store(av, i, ref);
            }
            retval = newRV((SV*)av);
            SvREFCNT_dec(av);
            break;
        }
        case 4: {
            AV *av = newAV();
            const char **values = CFCParamList_get_initial_values(self);
            size_t i;
            size_t num_vars = CFCParamList_num_vars(self);
            for (i = 0; i < num_vars; i++) {
                if (values[i] != NULL) {
                    SV *val_sv = newSVpvn(values[i], strlen(values[i]));
                    av_store(av, i, val_sv);
                }
                else {
                    av_store(av, i, newSV(0));
                }
            }
            retval = newRV((SV*)av);
            SvREFCNT_dec(av);
            break;
        }
        case 6:
            retval = newSViv(CFCParamList_variadic(self));
            break;
        case 8:
            retval = newSViv(CFCParamList_num_vars(self));
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
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
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
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)default_parcel));
OUTPUT: RETVAL

void
reap_singletons(...)
PPCODE:
    CFCParcel_reap_singletons();

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
                retval = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)parcel));
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
_new(klass, flags, parcel, specifier, indirection, c_string)
    const char *klass;
    int flags;
    CFCParcel *parcel;
    const char *specifier;
    int indirection;
    const char *c_string;
CODE:
    CFCType *self = CFCType_new(flags, parcel, specifier, indirection, 
        c_string);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_integer(klass, flags, specifier)
    const char *klass;
    int flags;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_integer(flags, specifier);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_float(klass, flags, specifier)
    const char *klass;
    int flags;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_float(flags, specifier);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_object(klass, flags, parcel, specifier, indirection)
    const char *klass;
    int flags;
    CFCParcel *parcel;
    const char *specifier;
    int indirection;
CODE:
    CFCType *self = CFCType_new_object(flags, parcel, specifier, indirection);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
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
    }
    else {
        croak("Param 'child' not a Clownfish::Type");
    }
    CFCType *self = CFCType_new_composite(flags, child, indirection, array);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_void(klass, is_const)
    const char *klass;
    int is_const;
CODE:
    CFCType *self = CFCType_new_void(is_const);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_va_list(klass)
    const char *klass;
CODE:
    CFCType *self = CFCType_new_va_list();
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_arbitrary(klass, parcel, specifier)
    const char *klass;
    CFCParcel *parcel;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_arbitrary(parcel, specifier);
    RETVAL = newRV((SV*)CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
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
                       ? newRV((SV*)CFCBase_get_perl_obj((CFCBase*)parcel))
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


MODULE = Clownfish   PACKAGE = Clownfish::Util

SV*
trim_whitespace(text)
    SV *text;
CODE:
    RETVAL = newSVsv(text);
    STRLEN len;
    char *ptr = SvPV(RETVAL, len);
    CFCUtil_trim_whitespace(ptr);
    SvCUR_set(RETVAL, strlen(ptr));
OUTPUT: RETVAL


MODULE = Clownfish   PACKAGE = Clownfish::Variable

SV*
_new(klass, parcel, exposure, class_name_sv, class_cnick_sv, micro_sym_sv, type_sv)
    const char *klass;
    CFCParcel *parcel;
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
    CFCType *type = NULL;
    if (SvOK(type_sv) && sv_derived_from(type_sv, "Clownfish::Type")) {
        IV objint = SvIV((SV*)SvRV(type_sv));
        type = INT2PTR(CFCType*, objint);
    }
    else {
        croak("Param 'type' is not a Clownfish::Type");
    }
    CFCVariable *self = CFCVariable_new(parcel, exposure, class_name,
        class_cnick, micro_sym, type);
    RETVAL = newRV(CFCBase_get_perl_obj((CFCBase*)self));
    CFCBase_decref((CFCBase*)self);
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
                CFCType *type = CFCVariable_get_type(self);
                retval = newRV(CFCBase_get_perl_obj((CFCBase*)type));
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

