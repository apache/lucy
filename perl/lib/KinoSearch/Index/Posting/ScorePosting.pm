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

package KinoSearch::Index::Posting::ScorePosting;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Index::Posting::ScorePosting

SV*
get_prox(self)
    kino_ScorePosting *self;
CODE:
{
    AV *out_av            = newAV();
    uint32_t *positions  = Kino_ScorePost_Get_Prox(self);
    uint32_t i, max;

    for (i = 0, max = Kino_ScorePost_Get_Freq(self); i < max; i++) {
        SV *pos_sv = newSVuv(positions[i]);
        av_push(out_av, pos_sv);
    }

    RETVAL = newRV_noinc((SV*)out_av);
}
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    # ScorePosting is used indirectly, by specifying in FieldType subclass.
    package MySchema::Category;
    use base qw( KinoSearch::Plan::FullTextType );
    # (It's the default, so you don't need to spec it.)
    # sub posting {
    #     my $self = shift;
    #     return KinoSearch::Index::Posting::ScorePosting->new(@_);
    # }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::Posting::ScorePosting",
    xs_code           => $xs_code,
    bind_constructors => ["new"],
#    make_pod => {
#        synopsis => $synopsis,
#    }
);


