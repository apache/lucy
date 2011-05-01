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

package Lucy::Document::HitDoc;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Document::HitDoc

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    SV *fields_sv = NULL;
    int32_t doc_id = 0;
    float score = 0.0f;
    chy_bool_t args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              "Lucy::Document::HitDoc::new_PARAMS",
                              ALLOT_SV(&fields_sv, "fields", 6, false),
                              ALLOT_I32(&doc_id, "doc_id", 6, false),
                              ALLOT_F32(&score, "score", 5, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(LUCY_INCREF(cfish_Err_get_error()));
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

    lucy_HitDoc *self = (lucy_HitDoc*)XSBind_new_blank_obj(either_sv);
    lucy_HitDoc_init(self, fields, doc_id, score);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    while ( my $hit_doc = $hits->next ) {
        print "$hit_doc->{title}\n";
        print $hit_doc->get_score . "\n";
        ...
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Document::HitDoc",
    bind_methods      => [qw( Set_Score Get_Score )],
    xs_code           => $xs_code,
    make_pod          => {
        methods  => [qw( set_score get_score )],
        synopsis => $synopsis,
    },
);


