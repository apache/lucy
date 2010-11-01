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

package KinoSearch::Search::SortRule;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Search::SortRule

int32_t
FIELD()
CODE:
    RETVAL = kino_SortRule_FIELD;
OUTPUT: RETVAL

int32_t
SCORE()
CODE:
    RETVAL = kino_SortRule_SCORE;
OUTPUT: RETVAL

int32_t
DOC_ID()
CODE:
    RETVAL = kino_SortRule_DOC_ID;
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    my $sort_spec = KinoSearch::Search::SortSpec->new(
        rules => [
            KinoSearch::Search::SortRule->new( field => 'date' ),
            KinoSearch::Search::SortRule->new( type  => 'doc_id' ),
        ],
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $by_title   = KinoSearch::Search::SortRule->new( field => 'title' );
    my $by_score   = KinoSearch::Search::SortRule->new( type  => 'score' );
    my $by_doc_id  = KinoSearch::Search::SortRule->new( type  => 'doc_id' );
    my $reverse_date = KinoSearch::Search::SortRule->new(
        field   => 'date',
        reverse => 1,
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::SortRule",
    xs_code           => $xs_code,
    bind_constructors => ["_new"],
    bind_methods      => [qw( Get_Field Get_Reverse )],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [qw( get_field get_reverse )],
    },
);


