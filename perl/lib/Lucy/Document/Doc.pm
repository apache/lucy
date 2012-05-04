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

package Lucy::Document::Doc;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Document::Doc

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    SV* fields_sv = NULL;
    int32_t doc_id = 0;
    chy_bool_t args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              "Lucy::Document::Doc::new_PARAMS",
                              ALLOT_SV(&fields_sv, "fields", 6, false),
                              ALLOT_I32(&doc_id, "doc_id", 6, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }

    HV *fields = NULL;
    if (fields_sv && XSBind_sv_defined(fields_sv)) {
        if (SvROK(fields_sv)) {
            fields = (HV*)SvRV(fields_sv);
        }
        if (!fields || SvTYPE((SV*)fields) != SVt_PVHV) {
            CFISH_THROW(CFISH_ERR, "fields is not a hashref");
        }
    }

    lucy_Doc *self = (lucy_Doc*)XSBind_new_blank_obj(either_sv);
    lucy_Doc_init(self, fields, doc_id);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
get_fields(self, ...)
    lucy_Doc *self;
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = newRV_inc((SV*)Lucy_Doc_Get_Fields(self));
OUTPUT: RETVAL

void
set_fields(self, fields)
    lucy_Doc *self;
    HV *fields;
PPCODE:
    lucy_Doc_set_fields(self, fields);
END_XS_CODE

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

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Document::Doc",
    xs_code           => $xs_code,
    bind_methods      => [qw( Set_Doc_ID Get_Doc_ID )],
    make_pod          => {
        methods     => [qw( set_doc_id get_doc_id get_fields )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


