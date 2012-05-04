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

package Lucy::Index::Similarity;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Index::Similarity

SV*
get_norm_decoder(self)
    lucy_Similarity *self;
CODE:
    RETVAL = newSVpvn((char*)Lucy_Sim_Get_Norm_Decoder(self),
                      (256 * sizeof(float)));
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    package MySimilarity;

    sub length_norm { return 1.0 }    # disable length normalization

    package MyFullTextType;
    use base qw( Lucy::Plan::FullTextType );

    sub make_similarity { MySimilarity->new }
END_SYNOPSIS

my $constructor = qq|    my \$sim = Lucy::Index::Similarity->new;\n|;

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Index::Similarity",
    xs_code      => $xs_code,
    bind_methods => [
        qw( IDF
            TF
            Encode_Norm
            Decode_Norm
            Query_Norm
            Length_Norm
            Coord )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [qw( length_norm )],
    }
);


