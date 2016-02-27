# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
package Lucy::Build::Binding::Document;
use strict;
use warnings;

our $VERSION = '0.004000';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_doc;
    $class->bind_hitdoc;
}

sub bind_doc {
    my @hand_rolled = qw( Store Set_Fields Get_Fields );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $doc = Lucy::Document::Doc->new(
        fields => { foo => 'foo foo', bar => 'bar bar' },
    );
    $indexer->add_doc($doc);

Doc objects allow access to field values via hashref overloading:

    $doc->{foo} = 'new value for field "foo"';
    print "foo: $doc->{foo}\n";
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $doc = Lucy::Document::Doc->new(
        fields => { foo => 'foo foo', bar => 'bar bar' },
    );
END_CONSTRUCTOR
    my $store_pod = <<'END_POD';
=head2 store

    $doc->store($field, $value);

Store a field value in the Doc.

=over

=item *

B<field> - The field name.

=item *

B<value> - The value.

=back
END_POD
    my $get_fields_pod = <<'END_POD';
=head2 get_fields

    my $hashref = $doc->get_fields();

Return the Doc's backing fields hash.
END_POD
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );
    $pod_spec->add_method(
        method => 'Store',
        alias  => 'store',
        pod    => $store_pod,
    );
    $pod_spec->add_method(
        method => 'Get_Fields',
        alias  => 'get_fields',
        pod    => $get_fields_pod,
    );

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Document::Doc

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    static const XSBind_ParamSpec param_specs[2] = {
        XSBIND_PARAM("fields", false),
        XSBIND_PARAM("doc_id", false)
    };
    int32_t   locations[2];
    SV*       fields_sv = NULL;
    HV       *fields    = NULL;
    int32_t   doc_id    = 0;
    lucy_Doc *self      = NULL;

    XSBind_locate_args(aTHX_ &ST(0), 1, items, param_specs, locations, 2);

    fields_sv = locations[0] < items ? ST(locations[0]) : NULL;
    doc_id    = locations[1] < items ? (int32_t)SvIV(ST(locations[1])) : 0;

    if (fields_sv && XSBind_sv_defined(aTHX_ fields_sv)) {
        if (SvROK(fields_sv)) {
            fields = (HV*)SvRV(fields_sv);
        }
        if (!fields || SvTYPE((SV*)fields) != SVt_PVHV) {
            CFISH_THROW(CFISH_ERR, "fields is not a hashref");
        }
    }

    self = (lucy_Doc*)XSBind_new_blank_obj(aTHX_ either_sv);
    lucy_Doc_init(self, fields, doc_id);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

void
store(self, field, value_sv)
    lucy_Doc *self;
    cfish_String *field;
    SV *value_sv;
PPCODE:
{
    cfish_Obj *value
        = (cfish_Obj*)XSBind_perl_to_cfish(aTHX_ value_sv, CFISH_OBJ);
    LUCY_Doc_Store(self, field, value);
    CFISH_DECREF(value);
}

SV*
get_fields(self, ...)
    lucy_Doc *self;
CODE:
    CFISH_UNUSED_VAR(items);
    RETVAL = newRV_inc((SV*)LUCY_Doc_Get_Fields(self));
OUTPUT: RETVAL

void
set_fields(self, fields)
    lucy_Doc *self;
    HV *fields;
PPCODE:
    LUCY_Doc_Set_Fields_IMP(self, fields);
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Document::Doc",
    );
    $binding->append_xs($xs_code);
    $binding->exclude_method($_) for @hand_rolled;
    $binding->exclude_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_hitdoc {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    while ( my $hit_doc = $hits->next ) {
        print "$hit_doc->{title}\n";
        print $hit_doc->get_score . "\n";
        ...
    }
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Document::HitDoc

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    static const XSBind_ParamSpec param_specs[3] = {
        XSBIND_PARAM("fields", false),
        XSBIND_PARAM("doc_id", false),
        XSBIND_PARAM("score", false)
    };
    int32_t      locations[3];
    SV          *fields_sv = NULL;
    HV          *fields    = NULL;
    int32_t      doc_id    = 0;
    float        score     = 0.0f;
    lucy_HitDoc *self      = NULL;

    XSBind_locate_args(aTHX_ &ST(0), 1, items, param_specs, locations, 3);

    fields_sv = locations[0] < items ? ST(locations[0]) : NULL;
    doc_id    = locations[1] < items ? (int32_t)SvIV(ST(locations[1])) : 0;
    score     = locations[2] < items ? (float)SvNV(ST(locations[2])) : 0.0f;

    if (fields_sv && XSBind_sv_defined(aTHX_ fields_sv)) {
        if (SvROK(fields_sv)) {
            fields = (HV*)SvRV(fields_sv);
        }
        if (!fields || SvTYPE((SV*)fields) != SVt_PVHV) {
            CFISH_THROW(CFISH_ERR, "fields is not a hashref");
        }
    }

    self = (lucy_HitDoc*)XSBind_new_blank_obj(aTHX_ either_sv);
    lucy_HitDoc_init(self, fields, doc_id, score);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Document::HitDoc",
    );
    $binding->append_xs($xs_code);
    $binding->set_pod_spec($pod_spec);
    $binding->exclude_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
