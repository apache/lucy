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
    my @exposed     = qw( Set_Doc_ID Get_Doc_ID Get_Fields );
    my @hand_rolled = qw( Set_Fields Get_Fields );

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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Document::Doc

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    SV*       fields_sv = NULL;
    HV       *fields    = NULL;
    int32_t   doc_id    = 0;
    lucy_Doc *self      = NULL;
    bool      args_ok;

    args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              ALLOT_SV(&fields_sv, "fields", 6, false),
                              ALLOT_I32(&doc_id, "doc_id", 6, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }

    if (fields_sv && XSBind_sv_defined(fields_sv)) {
        if (SvROK(fields_sv)) {
            fields = (HV*)SvRV(fields_sv);
        }
        if (!fields || SvTYPE((SV*)fields) != SVt_PVHV) {
            CFISH_THROW(CFISH_ERR, "fields is not a hashref");
        }
    }

    self = (lucy_Doc*)XSBind_new_blank_obj(either_sv);
    lucy_Doc_init(self, fields, doc_id);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

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
    $binding->bind_method(
        alias  => '_load',
        method => 'Load',
    );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->exclude_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_hitdoc {
    my @exposed = qw( Set_Score Get_Score );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    while ( my $hit_doc = $hits->next ) {
        print "$hit_doc->{title}\n";
        print $hit_doc->get_score . "\n";
        ...
    }
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Document::HitDoc

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    SV          *fields_sv = NULL;
    HV          *fields    = NULL;
    int32_t      doc_id    = 0;
    float        score     = 0.0f;
    lucy_HitDoc *self      = NULL;
    bool         args_ok;

    args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              ALLOT_SV(&fields_sv, "fields", 6, false),
                              ALLOT_I32(&doc_id, "doc_id", 6, false),
                              ALLOT_F32(&score, "score", 5, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }

    if (fields_sv && XSBind_sv_defined(fields_sv)) {
        if (SvROK(fields_sv)) {
            fields = (HV*)SvRV(fields_sv);
        }
        if (!fields || SvTYPE((SV*)fields) != SVt_PVHV) {
            CFISH_THROW(CFISH_ERR, "fields is not a hashref");
        }
    }

    self = (lucy_HitDoc*)XSBind_new_blank_obj(either_sv);
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
