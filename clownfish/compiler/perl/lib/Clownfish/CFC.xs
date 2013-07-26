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
        default: croak("Internal error. ix: %d", (int)ix); \
    } \
    if (ix % 2 == 0) { \
        XPUSHs(sv_2mortal(retval)); \
        XSRETURN(1); \
    } \
    else { \
        XSRETURN(0); \
    }

static SV*
S_cfcbase_to_perlref(void *thing) {
    SV *ref = newSV(0);
    if (thing) {
        const char *klass = CFCBase_get_cfc_class((CFCBase*)thing);
        CFCBase_incref((CFCBase*)thing);
        sv_setref_pv(ref, klass, (void*)thing);
    }
    return ref;
}

// Transform a NULL-terminated array of char* into a Perl arrayref.
static SV*
S_string_array_to_av(const char **strings) {
    AV *av = newAV();
    for (size_t i = 0; strings[i] != NULL; i++) {
        SV *val = newSVpvn(strings[i], strlen(strings[i]));
        av_store(av, i, val);
    }
    SV *retval = newRV_noinc((SV*)av);
    return retval;
}

// Transform a NULL-terminated array of CFCBase* into a Perl arrayref.
static SV*
S_array_of_cfcbase_to_av(CFCBase **things) {
    AV *av = newAV();
    for (size_t i = 0; things[i] != NULL; i++) {
        SV *val = S_cfcbase_to_perlref(things[i]);
        av_store(av, i, val);
    }
    SV *retval = newRV((SV*)av);
    SvREFCNT_dec(av);
    return retval;
}

// Transform a Perl arrayref into a NULL-terminated array of CFCBase*.
static CFCBase**
S_av_to_array_of_cfcbase(SV *ref, const char *class_name) {
    if (!SvROK(ref)) { croak("Not an arrayref"); }
    SV *sv = SvRV(ref);
    if (SvTYPE(sv) != SVt_PVAV) { croak("Not an arrayref"); }
    AV *av = (AV*)sv;
    size_t size = av_len(av) + 1;
    CFCBase **retval = (CFCBase**)CALLOCATE(size + 1, sizeof(CFCBase*));
    for (size_t i = 0; i < size; i++) {
        SV **elem = av_fetch(av, i, 0);
        if (!*elem || !sv_derived_from(*elem, class_name)) {
            croak("Array element not of type %s", class_name);
        }
        IV objint = SvIV((SV*)SvRV(*elem));
        retval[i] = INT2PTR(CFCBase*, objint);
    }
    return retval;
}

static SV*
S_sv_eat_c_string(char *string) {
    if (string) {
        SV *sv = newSVpvn(string, strlen(string));
        FREEMEM(string);
        return sv;
    }
    else {
        return newSV(0);
    }
}

MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Base

void
DESTROY(self)
    CFCBase *self;
PPCODE:
    CFCBase_decref((CFCBase*)self);


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::CBlock

SV*
_new(contents)
    const char *contents;
CODE:
    CFCCBlock *self = CFCCBlock_new(contents);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

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

MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::Class

SV*
_create(parcel, exposure_sv, class_name_sv, cnick_sv, micro_sym_sv, docucomment, file_spec, parent_class_name_sv, is_final, is_inert)
    CFCParcel *parcel;
    SV *exposure_sv;
    SV *class_name_sv;
    SV *cnick_sv;
    SV *micro_sym_sv;
    CFCDocuComment *docucomment;
    CFCFileSpec *file_spec;
    SV *parent_class_name_sv;
    bool is_final;
    bool is_inert;
CODE:
    const char *exposure =
        SvOK(exposure_sv) ? SvPV_nolen(exposure_sv) : NULL;
    const char *class_name =
        SvOK(class_name_sv) ? SvPV_nolen(class_name_sv) : NULL;
    const char *cnick =
        SvOK(cnick_sv) ? SvPV_nolen(cnick_sv) : NULL;
    const char *micro_sym =
        SvOK(micro_sym_sv) ? SvPV_nolen(micro_sym_sv) : NULL;
    const char *parent_class_name =
        SvOK(parent_class_name_sv) ? SvPV_nolen(parent_class_name_sv) : NULL;
    CFCClass *self = CFCClass_create(parcel, exposure, class_name, cnick,
                                     micro_sym, docucomment, file_spec,
                                     parent_class_name, is_final, is_inert);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_fetch_singleton(parcel, class_name)
    CFCParcel *parcel;
    const char *class_name;
CODE:
    CFCClass *klass = CFCClass_fetch_singleton(parcel, class_name);
    RETVAL = S_cfcbase_to_perlref(klass);
OUTPUT: RETVAL

void
_clear_registry(...)
PPCODE:
    CHY_UNUSED_VAR(items);
    CFCClass_clear_registry();

void
append_autocode(self, autocode)
    CFCClass *self;
    const char *autocode;
PPCODE:
    CFCClass_append_autocode(self, autocode);

void
add_child(self, child)
    CFCClass *self;
    CFCClass *child;
PPCODE:
    CFCClass_add_child(self, child);

void
add_member_var(self, var)
    CFCClass *self;
    CFCVariable *var;
PPCODE:
    CFCClass_add_member_var(self, var);

void
add_function(self, func)
    CFCClass *self;
    CFCFunction *func;
PPCODE:
    CFCClass_add_function(self, func);

void
add_method(self, method)
    CFCClass *self;
    CFCMethod *method;
PPCODE:
    CFCClass_add_method(self, method);

void
add_attribute(self, name, value_sv)
    CFCClass *self;
    const char *name;
    SV *value_sv;
PPCODE:
    char *value = SvOK(value_sv) ? SvPV_nolen(value_sv) : NULL;
    CFCClass_add_attribute(self, name, value);

int
has_attribute(self, name)
    CFCClass *self;
    const char *name;
CODE:
    RETVAL = CFCClass_has_attribute(self, name);
OUTPUT: RETVAL

void
grow_tree(self)
    CFCClass *self;
PPCODE:
    CFCClass_grow_tree(self);

void
add_inert_var(self, var)
    CFCClass *self;
    CFCVariable *var;
PPCODE:
    CFCClass_add_inert_var(self, var);

SV*
function(self, sym)
    CFCClass *self;
    const char *sym;
CODE:
    CFCFunction *func = CFCClass_function(self, sym);
    RETVAL = S_cfcbase_to_perlref(func);
OUTPUT: RETVAL

SV*
method(self, sym)
    CFCClass *self;
    const char *sym;
CODE:
    CFCMethod *method = CFCClass_method(self, sym);
    RETVAL = S_cfcbase_to_perlref(method);
OUTPUT: RETVAL

SV*
fresh_method(self, sym)
    CFCClass *self;
    const char *sym;
CODE:
    CFCMethod *method = CFCClass_fresh_method(self, sym);
    RETVAL = S_cfcbase_to_perlref(method);
OUTPUT: RETVAL

void
resolve_types(self, classes_sv)
    CFCClass *self;
    SV *classes_sv;
PPCODE:
    CFCClass **classes
        = (CFCClass**)S_av_to_array_of_cfcbase(classes_sv,
                                               "Clownfish::CFC::Model::Class");
    CFCClass_resolve_types(self, classes);
    FREEMEM(classes);

void
_set_or_get(self, ...)
    CFCClass *self;
ALIAS:
    get_cnick             = 2
    set_parent            = 5
    get_parent            = 6
    get_autocode          = 8
    get_path_part         = 10
    get_parent_class_name = 12
    final                 = 14
    inert                 = 16
    get_struct_sym        = 18
    full_struct_sym       = 20
    short_vtable_var      = 22
    full_vtable_var       = 24
    include_h             = 28
    get_docucomment       = 30
    children              = 32
    functions             = 34
    methods               = 36
    member_vars           = 38
    inert_vars            = 40
    tree_to_ladder        = 42
    fresh_methods         = 44
    fresh_member_vars     = 46
    privacy_symbol        = 48
    included              = 50
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *value = CFCClass_get_cnick(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 5: {
                CFCClass *parent = NULL;
                if (SvOK(ST(1))
                    && sv_derived_from(ST(1), "Clownfish::CFC::Model::Class")
                   ) {
                    IV objint = SvIV((SV*)SvRV(ST(1)));
                    parent = INT2PTR(CFCClass*, objint);
                }
                CFCClass_set_parent(self, parent);
                break;
            }
        case 6: {
                CFCClass *parent = CFCClass_get_parent(self);
                retval = S_cfcbase_to_perlref(parent);
                break;
            }
        case 8: {
                const char *value = CFCClass_get_autocode(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 10: {
                const char *value = CFCClass_get_path_part(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 12: {
                const char *value = CFCClass_get_parent_class_name(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 14:
            retval = newSViv(CFCClass_final(self));
            break;
        case 16:
            retval = newSViv(CFCClass_inert(self));
            break;
        case 18: {
                const char *value = CFCClass_get_struct_sym(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 20: {
                const char *value = CFCClass_full_struct_sym(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 22: {
                const char *value = CFCClass_short_vtable_var(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 24: {
                const char *value = CFCClass_full_vtable_var(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 28: {
                const char *value = CFCClass_include_h(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 30: {
                CFCDocuComment *docucomment = CFCClass_get_docucomment(self);
                retval = S_cfcbase_to_perlref(docucomment);
            }
            break;
        case 32:
            retval = S_array_of_cfcbase_to_av(
                (CFCBase**)CFCClass_children(self));
            break;
        case 34:
            retval = S_array_of_cfcbase_to_av((CFCBase**)CFCClass_functions(self));
            break;
        case 36:
            retval = S_array_of_cfcbase_to_av((CFCBase**)CFCClass_methods(self));
            break;
        case 38:
            retval = S_array_of_cfcbase_to_av((CFCBase**)CFCClass_member_vars(self));
            break;
        case 40:
            retval = S_array_of_cfcbase_to_av((CFCBase**)CFCClass_inert_vars(self));
            break;
        case 42: {
                CFCClass **ladder = CFCClass_tree_to_ladder(self);
                retval = S_array_of_cfcbase_to_av((CFCBase**)ladder);
                FREEMEM(ladder);
                break;
            }
        case 44: {
                CFCMethod **fresh = CFCClass_fresh_methods(self);
                retval = S_array_of_cfcbase_to_av((CFCBase**)fresh);
                FREEMEM(fresh);
                break;
            }
        case 46: {
                CFCVariable **fresh = CFCClass_fresh_member_vars(self);
                retval = S_array_of_cfcbase_to_av((CFCBase**)fresh);
                FREEMEM(fresh);
                break;
            }
        case 48: {
                const char *value = CFCClass_privacy_symbol(self);
                retval = value ? newSVpvn(value, strlen(value)) : newSV(0);
            }
            break;
        case 50:
            retval = newSViv(CFCClass_included(self));
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::DocuComment

SV*
parse(klass, text)
    const char *klass;
    const char *text;
CODE:
    if (strcmp(klass, "Clownfish::CFC::Model::DocuComment")) {
        croak("No subclassing allowed");
    }
    CFCDocuComment *self = CFCDocuComment_parse(text);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

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
                const char **names = CFCDocuComment_get_param_names(self);
                retval = S_string_array_to_av(names);
            }
            break;
        case 10: {
                const char **docs = CFCDocuComment_get_param_docs(self);
                retval = S_string_array_to_av(docs);
            }
            break;
        case 12: {
                const char *rv = CFCDocuComment_get_retval(self);
                retval = rv ? newSVpvn(rv, strlen(rv)) : newSV(0);
            }
            break;
    END_SET_OR_GET_SWITCH
}

MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Dumpable

SV*
new(klass)
    const char *klass;
CODE:
    if (strcmp(klass, "Clownfish::CFC::Dumpable")) {
        croak("No subclassing allowed");
    }
    CFCDumpable *self = CFCDumpable_new();
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
add_dumpables(self, klass)
    CFCDumpable *self;
    CFCClass *klass;
PPCODE:
    CFCDumpable_add_dumpables(self, klass);


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::File

SV*
_new(spec)
    CFCFileSpec *spec;
CODE:
    CFCFile *self = CFCFile_new(spec);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

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
    get_path_part      = 4
    guard_name         = 6
    guard_start        = 8
    guard_close        = 10
    blocks             = 12
    classes            = 14
    get_source_dir     = 16
    included           = 18
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
                const char *value = CFCFile_get_path_part(self);
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
        case 12:
            retval = S_array_of_cfcbase_to_av(CFCFile_blocks(self));
            break;
        case 14:
            retval = S_array_of_cfcbase_to_av(
                         (CFCBase**)CFCFile_classes(self));
            break;
        case 16: {
                const char *value = CFCFile_get_source_dir(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 18:
            retval = newSViv(CFCFile_included(self));
            break;
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
    char *buf;
    switch (ix) {
        case 1:
            buf = CFCFile_c_path(self, base_dir);
            break;
        case 2:
            buf = CFCFile_h_path(self, base_dir);
            break;
        case 3:
            buf = CFCFile_cfh_path(self, base_dir);
            break;
        default:
            croak("unexpected ix value: %d", (int)ix);
    }
    RETVAL = newSVpvn(buf, strlen(buf));
    FREEMEM(buf);
}
OUTPUT: RETVAL


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::FileSpec

SV*
_new(source_dir, path_part, is_included)
    const char *source_dir;
    const char *path_part;
    bool is_included;
CODE:
    CFCFileSpec *self = CFCFileSpec_new(source_dir, path_part, is_included);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCFileSpec *self;
ALIAS:
    get_source_dir     = 2
    get_path_part      = 4
    included           = 6
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *value = CFCFileSpec_get_source_dir(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 4: {
                const char *value = CFCFileSpec_get_path_part(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 6:
            retval = newSViv(CFCFileSpec_included(self));
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::Function

SV*
_new(parcel, exposure_sv, class_name_sv, class_cnick_sv, micro_sym_sv, return_type, param_list, docucomment, is_inline)
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
    const char *exposure =
        SvOK(exposure_sv) ? SvPV_nolen(exposure_sv) : NULL;
    const char *class_name =
        SvOK(class_name_sv) ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick =
        SvOK(class_cnick_sv) ? SvPV_nolen(class_cnick_sv) : NULL;
    const char *micro_sym =
        SvOK(micro_sym_sv) ? SvPV_nolen(micro_sym_sv) : NULL;
    CFCFunction *self = CFCFunction_new(parcel, exposure, class_name,
                                        class_cnick, micro_sym, return_type,
                                        param_list, docucomment, is_inline);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
resolve_types(self, classes_sv)
    CFCFunction *self;
    SV *classes_sv;
PPCODE:
    CFCClass **classes
        = (CFCClass**)S_av_to_array_of_cfcbase(classes_sv,
                                               "Clownfish::CFC::Model::Class");
    CFCFunction_resolve_types(self, classes);
    FREEMEM(classes);

void
_set_or_get(self, ...)
    CFCFunction *self;
ALIAS:
    get_return_type    = 2
    get_param_list     = 4
    get_docucomment    = 6
    inline             = 8
    void               = 10
    full_func_sym      = 12
    short_func_sym     = 14
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                CFCType *type = CFCFunction_get_return_type(self);
                retval = S_cfcbase_to_perlref(type);
            }
            break;
        case 4: {
                CFCParamList *param_list = CFCFunction_get_param_list(self);
                retval = S_cfcbase_to_perlref(param_list);
            }
            break;
        case 6: {
                CFCDocuComment *docucomment
                    = CFCFunction_get_docucomment(self);
                retval = S_cfcbase_to_perlref(docucomment);
            }
            break;
        case 8:
            retval = newSViv(CFCFunction_inline(self));
            break;
        case 10:
            retval = newSViv(CFCFunction_void(self));
            break;
        case 12: {
                const char *full_sym = CFCFunction_full_func_sym(self);
                retval = newSVpv(full_sym, strlen(full_sym));
            }
            break;
        case 14: {
                const char *short_sym = CFCFunction_short_func_sym(self);
                retval = newSVpv(short_sym, strlen(short_sym));
            }
            break;
    END_SET_OR_GET_SWITCH
}

MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::Hierarchy

SV*
_new(dest)
    const char *dest;
CODE:
    CFCHierarchy *self = CFCHierarchy_new(dest);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
add_source_dir(self, source_dir)
    CFCHierarchy *self;
    const char *source_dir;
PPCODE:
    CFCHierarchy_add_source_dir(self, source_dir);

void
add_include_dir(self, include_dir)
    CFCHierarchy *self;
    const char *include_dir;
PPCODE:
    CFCHierarchy_add_include_dir(self, include_dir);

void
build(self)
    CFCHierarchy *self;
PPCODE:
    CFCHierarchy_build(self);

int
propagate_modified(self, ...)
    CFCHierarchy *self;
CODE:
    int modified = items > 1 ? !!SvTRUE(ST(1)) : 0;
    RETVAL = CFCHierarchy_propagate_modified(self, modified);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCHierarchy *self;
ALIAS:
    get_dest          = 2
    get_include_dest  = 4
    get_source_dest   = 6
    files             = 8
    ordered_classes   = 10
    get_source_dirs   = 12
    get_include_dirs  = 14
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *value = CFCHierarchy_get_dest(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 4: {
                const char *value = CFCHierarchy_get_include_dest(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 6: {
                const char *value = CFCHierarchy_get_source_dest(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 8:
            retval = S_array_of_cfcbase_to_av(
                (CFCBase**)CFCHierarchy_files(self));
            break;
        case 10: {
                CFCClass **ladder = CFCHierarchy_ordered_classes(self);
                retval = S_array_of_cfcbase_to_av((CFCBase**)ladder);
                FREEMEM(ladder);
            }
            break;
        case 12: {
                const char **source_dirs = CFCHierarchy_get_source_dirs(self);
                retval = S_string_array_to_av(source_dirs);
            }
            break;
        case 14: {
                const char **include_dirs = CFCHierarchy_get_include_dirs(self);
                retval = S_string_array_to_av(include_dirs);
            }
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::Method

SV*
_new(parcel, exposure_sv, class_name_sv, class_cnick_sv, macro_sym, return_type, param_list, docucomment, is_final, is_abstract)
    CFCParcel *parcel;
    SV *exposure_sv;
    SV *class_name_sv;
    SV *class_cnick_sv;
    const char *macro_sym;
    CFCType *return_type;
    CFCParamList *param_list;
    CFCDocuComment *docucomment;
    int is_final;
    int is_abstract;
CODE:
    const char *exposure =
        SvOK(exposure_sv) ? SvPV_nolen(exposure_sv) : NULL;
    const char *class_name =
        SvOK(class_name_sv) ? SvPV_nolen(class_name_sv) : NULL;
    const char *class_cnick =
        SvOK(class_cnick_sv) ? SvPV_nolen(class_cnick_sv) : NULL;
    CFCMethod *self = CFCMethod_new(parcel, exposure, class_name, class_cnick,
                                    macro_sym, return_type, param_list,
                                    docucomment, is_final, is_abstract);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

int
compatible(self, other)
    CFCMethod *self;
    CFCMethod *other;
CODE:
    RETVAL = CFCMethod_compatible(self, other);
OUTPUT: RETVAL

void
override(self, other)
    CFCMethod *self;
    CFCMethod *other;
PPCODE:
    CFCMethod_override(self, other);

SV*
finalize(self)
    CFCMethod *self;
CODE:
    CFCMethod *finalized = CFCMethod_finalize(self);
    RETVAL = S_cfcbase_to_perlref(finalized);
    CFCBase_decref((CFCBase*)finalized);
OUTPUT: RETVAL

void
resolve_types(self, classes_sv)
    CFCMethod *self;
    SV *classes_sv;
PPCODE:
    CFCClass **classes
        = (CFCClass**)S_av_to_array_of_cfcbase(classes_sv,
                                               "Clownfish::CFC::Model::Class");
    CFCMethod_resolve_types(self, classes);
    FREEMEM(classes);

void
exclude_from_host(self)
    CFCMethod *self;
PPCODE:
    CFCMethod_exclude_from_host(self);

SV*
_various_method_syms(self, invoker)
    CFCMethod *self;
    CFCClass  *invoker;
ALIAS:
    short_method_sym  = 1
    full_method_sym   = 2
    full_offset_sym   = 3
    short_typedef     = 4
    full_typedef      = 5
CODE:
    char *buf;
    switch (ix) {
        case 1:
            buf = CFCMethod_short_method_sym(self, invoker);
            break;
        case 2:
            buf = CFCMethod_full_method_sym(self, invoker);
            break;
        case 3:
            buf = CFCMethod_full_offset_sym(self, invoker);
            break;
        case 4:
            buf = CFCMethod_short_typedef(self, invoker);
            break;
        case 5:
            buf = CFCMethod_full_typedef(self, invoker);
            break;
        default: croak("Unexpected ix: %d", (int)ix);
    }
    RETVAL = newSVpvn(buf, strlen(buf));
    FREEMEM(buf);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCMethod *self;
ALIAS:
    get_macro_sym      = 2
    full_override_sym  = 10
    abstract           = 12
    novel              = 14
    final              = 16
    self_type          = 18
    set_host_alias     = 19
    get_host_alias     = 20
    excluded_from_host = 22
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *macro_sym = CFCMethod_get_macro_sym(self);
                retval = newSVpvn(macro_sym, strlen(macro_sym));
            }
            break;
        case 10: {
                const char *value = CFCMethod_full_override_sym(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 12:
            retval = newSViv(CFCMethod_abstract(self));
            break;
        case 14:
            retval = newSViv(CFCMethod_novel(self));
            break;
        case 16:
            retval = newSViv(CFCMethod_final(self));
            break;
        case 18: {
                CFCType *type = CFCMethod_self_type(self);
                retval = S_cfcbase_to_perlref(type);
            }
            break;
        case 19: {
                const char *value = SvOK(ST(1)) ? SvPVutf8_nolen(ST(1)) : NULL;
                CFCMethod_set_host_alias(self, value);
            }
            break;
        case 20: {
                const char *value = CFCMethod_get_host_alias(self);
                if (value) {
                    retval = newSVpvn(value, strlen(value));
                }
            }
            break;
        case 22:
            retval = newSViv(CFCMethod_excluded_from_host(self));
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::ParamList

SV*
_new(klass, variadic)
    int variadic;
CODE:
    CFCParamList *self = CFCParamList_new(variadic);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
add_param(self, variable, value_sv)
    CFCParamList *self;
    CFCVariable  *variable;
    SV *value_sv;
PPCODE:
    const char *value = SvOK(value_sv) ? SvPV_nolen(value_sv) : NULL;
    CFCParamList_add_param(self, variable, value);

void
resolve_types(self, classes_sv)
    CFCParamList *self;
    SV *classes_sv;
PPCODE:
    CFCClass **classes
        = (CFCClass**)S_av_to_array_of_cfcbase(classes_sv,
                                               "Clownfish::CFC::Model::Class");
    CFCParamList_resolve_types(self, classes);
    FREEMEM(classes);

void
_set_or_get(self, ...)
    CFCParamList *self;
ALIAS:
    get_variables      = 2
    get_initial_values = 4
    variadic           = 6
    num_vars           = 8
    to_c               = 10
    name_list          = 12
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                AV *av = newAV();
                CFCVariable **vars = CFCParamList_get_variables(self);
                size_t num_vars = CFCParamList_num_vars(self);
                for (size_t i = 0; i < num_vars; i++) {
                    SV *ref = S_cfcbase_to_perlref(vars[i]);
                    av_store(av, i, ref);
                }
                retval = newRV((SV*)av);
                SvREFCNT_dec(av);
                break;
            }
        case 4: {
                AV *av = newAV();
                const char **values = CFCParamList_get_initial_values(self);
                size_t num_vars = CFCParamList_num_vars(self);
                for (size_t i = 0; i < num_vars; i++) {
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
        case 10: {
                const char *value = CFCParamList_to_c(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
        case 12: {
                const char *value = CFCParamList_name_list(self);
                retval = newSVpv(value, strlen(value));
            }
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::Parcel

SV*
_new(name_sv, cnick_sv, version, is_included)
    SV *name_sv;
    SV *cnick_sv;
    CFCVersion *version;
    bool is_included;
CODE:
    const char *name  = SvOK(name_sv)  ? SvPV_nolen(name_sv)  : NULL;
    const char *cnick = SvOK(cnick_sv) ? SvPV_nolen(cnick_sv) : NULL;
    CFCParcel *self = CFCParcel_new(name, cnick, version, is_included);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_from_file(path, is_included)
    const char *path;
    bool is_included;
CODE:
    CFCParcel *self = CFCParcel_new_from_file(path, is_included);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_from_json(json, is_included)
    const char *json;
    bool is_included;
CODE:
    CFCParcel *self = CFCParcel_new_from_json(json, is_included);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
fetch(unused, name_sv)
    SV *unused;
    SV *name_sv;
CODE:
    CHY_UNUSED_VAR(unused);
    const char *name  = SvOK(name_sv)  ? SvPV_nolen(name_sv)  : NULL;
    CFCParcel *self = CFCParcel_fetch(name);
    RETVAL = S_cfcbase_to_perlref(self);
OUTPUT: RETVAL


void
register(self)
    CFCParcel *self;
PPCODE:
    CFCParcel_register(self);

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
    CHY_UNUSED_VAR(items);
    CFCParcel *default_parcel = CFCParcel_default_parcel();
    RETVAL = S_cfcbase_to_perlref(default_parcel);
OUTPUT: RETVAL

SV*
all_parcels(...)
CODE:
    CHY_UNUSED_VAR(items);
    CFCParcel **all_parcels = CFCParcel_all_parcels();
    RETVAL = S_array_of_cfcbase_to_av((CFCBase**)all_parcels);
    FREEMEM(all_parcels);
OUTPUT: RETVAL

void
add_dependent_parcel(self, dependent)
    CFCParcel *self;
    CFCParcel *dependent;
PPCODE:
    CFCParcel_add_dependent_parcel(self, dependent);

void
add_inherited_parcel(self, inherited)
    CFCParcel *self;
    CFCParcel *inherited;
PPCODE:
    CFCParcel_add_inherited_parcel(self, inherited);

void
reap_singletons(...)
PPCODE:
    CHY_UNUSED_VAR(items);
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
    get_version = 12
    included    = 14
    dependent_parcels = 16
    inherited_parcels = 18
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
        case 12: {
                CFCVersion *value = CFCParcel_get_version(self);
                retval = S_cfcbase_to_perlref(value);
            }
            break;
        case 14:
            retval = newSViv(CFCParcel_included(self));
            break;
        case 16: {
                CFCParcel **parcels = CFCParcel_dependent_parcels(self);
                retval = S_array_of_cfcbase_to_av((CFCBase**)parcels);
                FREEMEM(parcels);
            }
            break;
        case 18: {
                CFCParcel **parcels = CFCParcel_inherited_parcels(self);
                retval = S_array_of_cfcbase_to_av((CFCBase**)parcels);
                FREEMEM(parcels);
            }
            break;
    END_SET_OR_GET_SWITCH
}


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::Symbol

SV*
_new(parcel, exposure, class_name_sv, class_cnick_sv, micro_sym_sv)
    CFCParcel *parcel;
    const char *exposure;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
CODE:
    const char *class_name  = SvOK(class_name_sv)
                              ? SvPV_nolen(class_name_sv)
                              : NULL;
    const char *class_cnick = SvOK(class_cnick_sv)
                              ? SvPV_nolen(class_cnick_sv)
                              : NULL;
    const char *micro_sym   = SvOK(micro_sym_sv)
                              ? SvPV_nolen(micro_sym_sv)
                              : NULL;
    CFCSymbol *self = CFCSymbol_new(parcel, exposure, class_name, class_cnick,
                                    micro_sym);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

int
equals(self, other)
    CFCSymbol *self;
    CFCSymbol *other;
CODE:
    RETVAL = CFCSymbol_equals(self, other);
OUTPUT: RETVAL

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
                retval = S_cfcbase_to_perlref(parcel);
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


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Model::Type

SV*
_new(flags, parcel, specifier, indirection)
    int flags;
    CFCParcel *parcel;
    const char *specifier;
    int indirection;
CODE:
    CFCType *self = CFCType_new(flags, parcel, specifier, indirection);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_integer(flags, specifier)
    int flags;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_integer(flags, specifier);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_float(flags, specifier)
    int flags;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_float(flags, specifier);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_object(flags, parcel, specifier, indirection)
    int flags;
    CFCParcel *parcel;
    const char *specifier;
    int indirection;
CODE:
    CFCType *self = CFCType_new_object(flags, parcel, specifier, indirection);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_composite(flags, child_sv, indirection, array)
    int flags;
    SV *child_sv;
    int indirection;
    const char *array;
CODE:
    CFCType *child = NULL;
    if (SvOK(child_sv) && sv_derived_from(child_sv, "Clownfish::CFC::Model::Type")) {
        IV objint = SvIV((SV*)SvRV(child_sv));
        child = INT2PTR(CFCType*, objint);
    }
    else {
        croak("Param 'child' not a Clownfish::CFC::Model::Type");
    }
    CFCType *self = CFCType_new_composite(flags, child, indirection, array);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_void(is_const)
    int is_const;
CODE:
    CFCType *self = CFCType_new_void(is_const);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_va_list()
CODE:
    CFCType *self = CFCType_new_va_list();
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
_new_arbitrary(parcel, specifier)
    CFCParcel *parcel;
    const char *specifier;
CODE:
    CFCType *self = CFCType_new_arbitrary(parcel, specifier);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

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
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_CONST;
OUTPUT: RETVAL

unsigned
NULLABLE(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_NULLABLE;
OUTPUT: RETVAL

unsigned
INCREMENTED(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_INCREMENTED;
OUTPUT: RETVAL

unsigned
DECREMENTED(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_DECREMENTED;
OUTPUT: RETVAL

unsigned
VOID(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_VOID;
OUTPUT: RETVAL

unsigned
OBJECT(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_OBJECT;
OUTPUT: RETVAL

unsigned
PRIMITIVE(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_PRIMITIVE;
OUTPUT: RETVAL

unsigned
INTEGER(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_INTEGER;
OUTPUT: RETVAL

unsigned
FLOATING(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_FLOATING;
OUTPUT: RETVAL

unsigned
STRING_TYPE(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_STRING_TYPE;
OUTPUT: RETVAL

unsigned
VA_LIST(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_VA_LIST;
OUTPUT: RETVAL

unsigned
ARBITRARY(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_ARBITRARY;
OUTPUT: RETVAL

unsigned
COMPOSITE(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = CFCTYPE_COMPOSITE;
OUTPUT: RETVAL

void
resolve(self, classes_sv)
    CFCType *self;
    SV *classes_sv;
PPCODE:
    CFCClass **classes
        = (CFCClass**)S_av_to_array_of_cfcbase(classes_sv,
                                               "Clownfish::CFC::Model::Class");
    CFCType_resolve(self, classes);
    FREEMEM(classes);

void
_set_or_get(self, ...)
    CFCType *self;
ALIAS:
    set_specifier   = 1
    get_specifier   = 2
    get_parcel      = 4
    get_indirection = 6
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
                retval = S_cfcbase_to_perlref(parcel);
            }
            break;
        case 6:
            retval = newSViv(CFCType_get_indirection(self));
            break;
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


MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Util

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

SV*
slurp_text(path)
    const char *path;
CODE:
    size_t len;
    char *contents = CFCUtil_slurp_text(path, &len);
    RETVAL = newSVpvn(contents, len);
    FREEMEM(contents);
OUTPUT: RETVAL

int
current(orig, dest)
    const char *orig;
    const char *dest;
CODE:
    RETVAL = CFCUtil_current(orig, dest);
OUTPUT: RETVAL

void
write_if_changed(path, content_sv)
    const char *path;
    SV *content_sv;
PPCODE:
    STRLEN len;
    char *content = SvPV(content_sv, len);
    CFCUtil_write_if_changed(path, content, len);

int
is_dir(path)
    const char *path;
CODE:
    RETVAL = CFCUtil_is_dir(path);
OUTPUT: RETVAL

int
make_dir(dir)
    const char *dir;
CODE:
    RETVAL = CFCUtil_make_dir(dir);
OUTPUT: RETVAL

int
make_path(path)
    const char *path;
CODE:
    RETVAL = CFCUtil_make_path(path);
OUTPUT: RETVAL

MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Model::Variable

SV*
_new(parcel, exposure, class_name_sv, class_cnick_sv, micro_sym_sv, type_sv, inert_sv)
    CFCParcel *parcel;
    const char *exposure;
    SV *class_name_sv;
    SV *class_cnick_sv;
    SV *micro_sym_sv;
    SV *type_sv;
    SV *inert_sv;
CODE:
    const char *class_name  = SvOK(class_name_sv)
                              ? SvPV_nolen(class_name_sv)
                              : NULL;
    const char *class_cnick = SvOK(class_cnick_sv)
                              ? SvPV_nolen(class_cnick_sv)
                              : NULL;
    const char *micro_sym   = SvOK(micro_sym_sv)
                              ? SvPV_nolen(micro_sym_sv)
                              : NULL;
    int inert               = SvOK(inert_sv)
                              ? !!SvTRUE(inert_sv) : 0;
    CFCType *type = NULL;
    if (SvOK(type_sv) && sv_derived_from(type_sv, "Clownfish::CFC::Model::Type")) {
        IV objint = SvIV((SV*)SvRV(type_sv));
        type = INT2PTR(CFCType*, objint);
    }
    else {
        croak("Param 'type' is not a Clownfish::CFC::Model::Type");
    }
    CFCVariable *self = CFCVariable_new(parcel, exposure, class_name,
                                        class_cnick, micro_sym, type, inert);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

int
equals(self, other)
    CFCVariable *self;
    CFCVariable *other;
CODE:
    RETVAL = CFCVariable_equals(self, other);
OUTPUT: RETVAL

void
resolve_type(self, classes_sv)
    CFCVariable *self;
    SV *classes_sv;
PPCODE:
    CFCClass **classes
        = (CFCClass**)S_av_to_array_of_cfcbase(classes_sv,
                                               "Clownfish::CFC::Model::Class");
    CFCVariable_resolve_type(self, classes);
    FREEMEM(classes);

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
                retval = S_cfcbase_to_perlref(type);
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

MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Model::Version

SV*
_new(vstring)
    const char *vstring;
CODE:
    CFCVersion *self = CFCVersion_new(vstring);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCVersion *self;
ALIAS:
    get_major         = 2
    get_vstring       = 4
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: 
            retval = newSVuv(CFCVersion_get_major(self));
            break;
        case 4: {
                const char *value = CFCVersion_get_vstring(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
    END_SET_OR_GET_SWITCH
}

int
compare_to(self, other)
    CFCVersion *self;
    CFCVersion *other;
CODE:
    RETVAL = CFCVersion_compare_to(self, other);
OUTPUT: RETVAL

MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Binding::Core

SV*
_new(hierarchy, header, footer)
    CFCHierarchy *hierarchy;
    const char   *header;
    const char   *footer;
CODE:
    CFCBindCore *self = CFCBindCore_new(hierarchy, header, footer);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

int
write_all_modified(self, ...)
    CFCBindCore *self;
CODE:
{
    int modified = 0;
    if (items > 1 && SvOK(ST(1))) {
        modified = !!SvIV(ST(1));
    }
    RETVAL = CFCBindCore_write_all_modified(self, modified);
}
OUTPUT: RETVAL


MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Binding::Core::Function

SV*
func_declaration(unused, func)
    SV *unused;
    CFCFunction *func;
CODE:
    CHY_UNUSED_VAR(unused);
    RETVAL = S_sv_eat_c_string(CFCBindFunc_func_declaration(func));
OUTPUT: RETVAL

MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Binding::Core::Method

SV*
abstract_method_def(unused, meth)
    SV *unused;
    CFCMethod *meth;
CODE:
    CHY_UNUSED_VAR(unused);
    RETVAL = S_sv_eat_c_string(CFCBindMeth_abstract_method_def(meth));
OUTPUT: RETVAL

SV*
_method_def(meth, klass)
    CFCMethod *meth;
    CFCClass  *klass;
CODE:
    RETVAL = S_sv_eat_c_string(CFCBindMeth_method_def(meth, klass));
OUTPUT: RETVAL

SV*
_novel_spec_def(meth)
    CFCMethod *meth;
CODE:
    RETVAL = S_sv_eat_c_string(CFCBindMeth_novel_spec_def(meth));
OUTPUT: RETVAL

SV*
_overridden_spec_def(meth, klass)
    CFCMethod *meth;
    CFCClass  *klass;
CODE:
    RETVAL = S_sv_eat_c_string(CFCBindMeth_overridden_spec_def(meth, klass));
OUTPUT: RETVAL

SV*
_inherited_spec_def(meth, klass)
    CFCMethod *meth;
    CFCClass  *klass;
CODE:
    RETVAL = S_sv_eat_c_string(CFCBindMeth_inherited_spec_def(meth, klass));
OUTPUT: RETVAL

MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Binding::Core::Aliases

SV*
c_aliases(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = S_sv_eat_c_string(CFCBindAliases_c_aliases());
OUTPUT: RETVAL

MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Binding::Core::Class

SV*
_new(client)
    CFCClass *client;
CODE:
    CFCBindClass *self = CFCBindClass_new(client);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
to_c_data(self)
    CFCBindClass *self;
CODE:
    RETVAL = S_sv_eat_c_string(CFCBindClass_to_c_data(self));
OUTPUT: RETVAL

SV*
to_c_header(self)
    CFCBindClass *self;
CODE:
    RETVAL = S_sv_eat_c_string(CFCBindClass_to_c_header(self));
OUTPUT: RETVAL

MODULE = Clownfish::CFC  PACKAGE = Clownfish::CFC::Binding::Core::File

void
_write_h(file, dest, header, footer)
    CFCFile *file;
    const char *dest;
    const char *header;
    const char *footer;
PPCODE:
    CFCBindFile_write_h(file, dest, header, footer);

MODULE = Clownfish   PACKAGE = Clownfish::CFC::Binding::Perl

SV*
_new(hierarchy, lib_dir, boot_class, header, footer)
    CFCHierarchy *hierarchy;
    const char *lib_dir;
    const char *boot_class;
    const char *header;
    const char *footer;
CODE:
    CFCPerl *self = CFCPerl_new(hierarchy, lib_dir, boot_class, header,
                                footer);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
write_pod(self)
    CFCPerl *self;
CODE:
    char **written = CFCPerl_write_pod(self);
    AV *modified = newAV();
    for (size_t i = 0; written[i] != NULL; i++) {
        SV *path = S_sv_eat_c_string(written[i]);
        av_push(modified, path);
    }
    FREEMEM(written);
    RETVAL = newRV_noinc((SV*)modified);
OUTPUT: RETVAL

void
write_boot(self)
    CFCPerl *self;
PPCODE:
    CFCPerl_write_boot(self);

void
write_hostdefs(self)
    CFCPerl *self;
PPCODE:
    CFCPerl_write_hostdefs(self);

void
write_bindings(self)
    CFCPerl *self;
PPCODE:
    CFCPerl_write_bindings(self);

void
write_callbacks(self)
    CFCPerl *self;
PPCODE:
    CFCPerl_write_callbacks(self);

void
write_xs_typemap(self)
    CFCPerl *self;
PPCODE:
    CFCPerl_write_xs_typemap(self);


MODULE = Clownfish   PACKAGE = Clownfish::CFC::Binding::Perl::Subroutine

void
_set_or_get(self, ...)
    CFCPerlSub *self;
ALIAS:
    get_class_name     = 2
    use_labeled_params = 4
    perl_name          = 6
    get_param_list     = 8
    c_name             = 10
    c_name_list        = 12
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *value = CFCPerlSub_get_class_name(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 4:
            retval = newSViv(CFCPerlSub_use_labeled_params(self));
            break;
        case 6: {
                const char *value = CFCPerlSub_perl_name(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 8: {
                CFCParamList *value = CFCPerlSub_get_param_list(self);
                retval = S_cfcbase_to_perlref(value);
            }
            break;
        case 10: {
                const char *value = CFCPerlSub_c_name(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 12: {
                const char *value = CFCPerlSub_c_name_list(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
    END_SET_OR_GET_SWITCH
}

SV*
params_hash_def(self)
    CFCPerlSub *self;
CODE:
    RETVAL = S_sv_eat_c_string(CFCPerlSub_params_hash_def(self));
OUTPUT: RETVAL

SV*
build_allot_params(self)
    CFCPerlSub *self;
CODE:
    RETVAL = S_sv_eat_c_string(CFCPerlSub_build_allot_params(self));
OUTPUT: RETVAL


MODULE = Clownfish   PACKAGE = Clownfish::CFC::Binding::Perl::Method

SV*
_new(method, alias)
    CFCMethod *method;
    const char *alias;
CODE:
    CFCPerlMethod *self = CFCPerlMethod_new(method, alias);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
xsub_def(self)
    CFCPerlMethod *self;
CODE:
    RETVAL = S_sv_eat_c_string(CFCPerlMethod_xsub_def(self));
OUTPUT: RETVAL


MODULE = Clownfish   PACKAGE = Clownfish::CFC::Binding::Perl::Constructor

SV*
_new(klass, alias, init_sv)
    CFCClass *klass;
    const char *alias;
    SV *init_sv;
CODE:
    const char *init = SvOK(init_sv) ? SvPVutf8_nolen(init_sv) : NULL;
    CFCPerlConstructor *self = CFCPerlConstructor_new(klass, alias, init);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
xsub_def(self)
    CFCPerlConstructor *self;
CODE:
    RETVAL = S_sv_eat_c_string(CFCPerlConstructor_xsub_def(self));
OUTPUT: RETVAL

MODULE = Clownfish   PACKAGE = Clownfish::CFC::Binding::Perl::Class

SV*
_new(parcel, class_name)
    CFCParcel  *parcel;
    const char *class_name;
CODE:
    CFCPerlClass *self = CFCPerlClass_new(parcel, class_name);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
register(unused, binding)
    SV *unused;
    CFCPerlClass *binding;
PPCODE:
    CHY_UNUSED_VAR(unused);
    CFCPerlClass_add_to_registry(binding);

SV*
singleton(unused_sv, class_name)
    SV *unused_sv;
    const char *class_name;
CODE:
    CHY_UNUSED_VAR(unused_sv);
    CFCPerlClass *binding = CFCPerlClass_singleton(class_name);
    RETVAL = S_cfcbase_to_perlref(binding);
OUTPUT: RETVAL

SV*
registered(...)
CODE:
    CHY_UNUSED_VAR(items);
    CFCPerlClass **registry = CFCPerlClass_registry();
    RETVAL = S_array_of_cfcbase_to_av((CFCBase**)registry);
OUTPUT: RETVAL

void
_clear_registry(...)
PPCODE:
    CHY_UNUSED_VAR(items);
    CFCPerlClass_clear_registry();

void
_bind_method(self, alias_sv, meth_sv)
    CFCPerlClass *self;
    SV *alias_sv;
    SV *meth_sv;
PPCODE:
    const char *alias = SvOK(alias_sv) ? SvPVutf8_nolen(alias_sv) : NULL;
    const char *meth  = SvOK(meth_sv)  ? SvPVutf8_nolen(meth_sv)  : NULL;
    CFCPerlClass_bind_method(self, alias, meth);

void
_bind_constructor(self, alias_sv, init_sv)
    CFCPerlClass *self;
    SV *alias_sv;
    SV *init_sv;
PPCODE:
    const char *alias = SvOK(alias_sv) ? SvPVutf8_nolen(alias_sv) : NULL;
    const char *init  = SvOK(init_sv)  ? SvPVutf8_nolen(init_sv)  : NULL;
    CFCPerlClass_bind_constructor(self, alias, init);

void
exclude_method(self, method)
    CFCPerlClass *self;
    const char *method;
PPCODE:
    CFCPerlClass_exclude_method(self, method);

void
exclude_constructor(self)
    CFCPerlClass *self;
PPCODE:
    CFCPerlClass_exclude_constructor(self);

void
append_xs(self, xs)
    CFCPerlClass *self;
    const char *xs;
PPCODE:
    CFCPerlClass_append_xs(self, xs);

SV*
method_bindings(unused, klass)
    SV *unused;
    CFCClass *klass;
CODE:
    CFCPerlMethod **bound = CFCPerlClass_method_bindings(klass);
    RETVAL = S_array_of_cfcbase_to_av((CFCBase**)bound);
    FREEMEM(bound);
OUTPUT: RETVAL

SV*
constructor_bindings(self)
    CFCPerlClass *self;
CODE:
    CFCPerlConstructor **bound = CFCPerlClass_constructor_bindings(self);
    RETVAL = S_array_of_cfcbase_to_av((CFCBase**)bound);
    FREEMEM(bound);
OUTPUT: RETVAL

SV*
create_pod(self)
    CFCPerlClass *self;
CODE:
    char *pod = CFCPerlClass_create_pod(self);
    RETVAL = S_sv_eat_c_string(pod);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCPerlClass *self;
ALIAS:
    get_class_name     = 2
    get_client         = 4
    get_xs_code        = 6
    set_pod_spec       = 7
    get_pod_spec       = 8
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 2: {
                const char *value = CFCPerlClass_get_class_name(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 4: {
                CFCClass *value = CFCPerlClass_get_client(self);
                retval = S_cfcbase_to_perlref(value);
            }
            break;
        case 6: {
                const char *value = CFCPerlClass_get_xs_code(self);
                retval = value
                         ? newSVpvn(value, strlen(value))
                         : newSV(0);
            }
            break;
        case 7: {
                CFCPerlPod *pod_spec = NULL;
                if (SvOK(ST(1))
                    && sv_derived_from(ST(1), "Clownfish::CFC::Binding::Perl::Pod")
                   ) {
                    IV objint = SvIV((SV*)SvRV(ST(1)));
                    pod_spec = INT2PTR(CFCPerlPod*, objint);
                }
                CFCPerlClass_set_pod_spec(self, pod_spec);
                break;
            }
        case 8: {
                CFCPerlPod *value = CFCPerlClass_get_pod_spec(self);
                retval = S_cfcbase_to_perlref(value);
            }
            break;
    END_SET_OR_GET_SWITCH
}

void
add_class_alias(self, alias)
    CFCPerlClass *self;
    const char *alias;
PPCODE:
    CFCPerlClass_add_class_alias(self, alias);

SV*
get_class_aliases(self)
    CFCPerlClass *self;
CODE:
    const char **aliases = CFCPerlClass_get_class_aliases(self);
    RETVAL = S_string_array_to_av(aliases);
OUTPUT: RETVAL


MODULE = Clownfish   PACKAGE = Clownfish::CFC::Binding::Perl::Pod

SV*
new(unused)
    SV *unused;
CODE:
    CHY_UNUSED_VAR(unused);
    CFCPerlPod *self = CFCPerlPod_new();
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

void
_add_method(self, alias, method_sv, sample_sv, pod_sv)
    CFCPerlPod *self;
    const char *alias;
    SV *method_sv;
    SV *sample_sv;
    SV *pod_sv;
PPCODE:
    const char *method = SvPOK(method_sv) ? SvPVutf8_nolen(method_sv) : NULL;
    const char *sample = SvPOK(sample_sv) ? SvPVutf8_nolen(sample_sv) : NULL;
    const char *pod    = SvPOK(pod_sv)    ? SvPVutf8_nolen(pod_sv)    : NULL;
    CFCPerlPod_add_method(self, alias, method, sample, pod);

void
_add_constructor(self, alias_sv, init_sv, sample_sv, pod_sv)
    CFCPerlPod *self;
    SV *alias_sv;
    SV *init_sv;
    SV *sample_sv;
    SV *pod_sv;
PPCODE:
    const char *alias  = SvPOK(alias_sv)  ? SvPVutf8_nolen(alias_sv)  : NULL;
    const char *init   = SvPOK(init_sv)   ? SvPVutf8_nolen(init_sv)   : NULL;
    const char *sample = SvPOK(sample_sv) ? SvPVutf8_nolen(sample_sv) : NULL;
    const char *pod    = SvPOK(pod_sv)    ? SvPVutf8_nolen(pod_sv)    : NULL;
    CFCPerlPod_add_constructor(self, alias, init, sample, pod);

SV*
methods_pod(self, klass)
    CFCPerlPod *self;
    CFCClass   *klass;
CODE:
    char *methods_pod = CFCPerlPod_methods_pod(self, klass);
    RETVAL = S_sv_eat_c_string(methods_pod);
OUTPUT: RETVAL

SV*
constructors_pod(self, klass)
    CFCPerlPod *self;
    CFCClass   *klass;
CODE:
    char *constructors_pod = CFCPerlPod_constructors_pod(self, klass);
    RETVAL = S_sv_eat_c_string(constructors_pod);
OUTPUT: RETVAL

void
_set_or_get(self, ...)
    CFCPerlPod *self;
ALIAS:
    set_synopsis       = 1
    get_synopsis       = 2
    set_description    = 3
    get_description    = 4
PPCODE:
{
    START_SET_OR_GET_SWITCH
        case 1: {
                const char *val = SvOK(ST(1)) ? SvPVutf8_nolen(ST(1)) : NULL;
                CFCPerlPod_set_synopsis(self, val);
            }
            break;
        case 2: {
                const char *value = CFCPerlPod_get_synopsis(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
        case 3: {
                const char *val = SvOK(ST(1)) ? SvPVutf8_nolen(ST(1)) : NULL;
                CFCPerlPod_set_description(self, val);
            }
            break;
        case 4: {
                const char *value = CFCPerlPod_get_description(self);
                retval = newSVpvn(value, strlen(value));
            }
            break;
    END_SET_OR_GET_SWITCH
}


SV*
_perlify_doc_text(self, source)
    CFCPerlPod   *self;
    const char   *source;
CODE:
    RETVAL = S_sv_eat_c_string(CFCPerlPod_perlify_doc_text(self, source));
OUTPUT: RETVAL

SV*
_gen_subroutine_pod(self, func, alias, klass, code_sample, class_name, is_constructor)
    CFCPerlPod *self;
    CFCFunction *func;
    const char *alias;
    CFCClass *klass;
    const char *code_sample;
    const char *class_name;
    int is_constructor;
CODE:
    char *value = CFCPerlPod_gen_subroutine_pod(self, func, alias, klass,
                                                code_sample, class_name,
                                                is_constructor);
    RETVAL = S_sv_eat_c_string(value);
OUTPUT: RETVAL


MODULE = Clownfish   PACKAGE = Clownfish::CFC::Binding::Perl::TypeMap

SV*
from_perl(type, xs_var)
    CFCType *type;
    const char *xs_var;
CODE:
    RETVAL = S_sv_eat_c_string(CFCPerlTypeMap_from_perl(type, xs_var));
OUTPUT: RETVAL

SV*
to_perl(type, cf_var)
    CFCType *type;
    const char *cf_var;
CODE:
    RETVAL = S_sv_eat_c_string(CFCPerlTypeMap_to_perl(type, cf_var));
OUTPUT: RETVAL

void
_write_xs_typemap(hierarchy)
    CFCHierarchy *hierarchy;
PPCODE:
    CFCPerlTypeMap_write_xs_typemap(hierarchy);


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Parser

SV*
new(klass)
    const char *klass;
CODE:
    if (strcmp(klass, "Clownfish::CFC::Parser")) {
        croak("No subclassing allowed");
    }
    CFCParser *self = CFCParser_new();
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

SV*
parse(self, string)
    CFCParser  *self;
    const char *string;
CODE:
    CFCBase *got = CFCParser_parse(self, string);
    RETVAL = S_cfcbase_to_perlref(got);
    CFCBase_decref((CFCBase*)got);
OUTPUT: RETVAL

SV*
_parse_file(self, string, file_spec)
    CFCParser   *self;
    const char  *string;
    CFCFileSpec *file_spec;
CODE:
    CFCFile *got = CFCParser_parse_file(self, string, file_spec);
    RETVAL = S_cfcbase_to_perlref(got);
    CFCBase_decref((CFCBase*)got);
OUTPUT: RETVAL

void
set_parcel(self, parcel)
    CFCParser *self;
    CFCParcel *parcel;
PPCODE:
    CFCParser_set_parcel(self, parcel);

void
set_class_name(self, class_name)
    CFCParser  *self;
    const char *class_name;
PPCODE:
    CFCParser_set_class_name(self, class_name);

void
set_class_cnick(self, class_cnick)
    CFCParser  *self;
    const char *class_cnick;
PPCODE:
    CFCParser_set_class_cnick(self, class_cnick);

SV*
get_parcel(self)
    CFCParser *self;
CODE:
    CFCParcel *parcel = CFCParser_get_parcel(self);
    RETVAL = S_cfcbase_to_perlref((CFCBase*)parcel);
OUTPUT: RETVAL


MODULE = Clownfish::CFC   PACKAGE = Clownfish::CFC::Test

SV*
_new(formatter_name)
    const char *formatter_name;
CODE:
    CFCTest *self = CFCTest_new(formatter_name);
    RETVAL = S_cfcbase_to_perlref(self);
    CFCBase_decref((CFCBase*)self);
OUTPUT: RETVAL

int
run_batch(self, klass)
    CFCTest *self;
    const char *klass;
CODE:
    RETVAL = CFCTest_run_batch(self, klass);
OUTPUT: RETVAL

