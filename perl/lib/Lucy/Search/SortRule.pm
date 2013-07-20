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

package Lucy::Search::SortRule;
use Lucy;
our $VERSION = '0.003003';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Search::SortRule

int32_t
FIELD()
CODE:
    RETVAL = lucy_SortRule_FIELD;
OUTPUT: RETVAL

int32_t
SCORE()
CODE:
    RETVAL = lucy_SortRule_SCORE;
OUTPUT: RETVAL

int32_t
DOC_ID()
CODE:
    RETVAL = lucy_SortRule_DOC_ID;
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    my $sort_spec = Lucy::Search::SortSpec->new(
        rules => [
            Lucy::Search::SortRule->new( field => 'date' ),
            Lucy::Search::SortRule->new( type  => 'doc_id' ),
        ],
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $by_title   = Lucy::Search::SortRule->new( field => 'title' );
    my $by_score   = Lucy::Search::SortRule->new( type  => 'score' );
    my $by_doc_id  = Lucy::Search::SortRule->new( type  => 'doc_id' );
    my $reverse_date = Lucy::Search::SortRule->new(
        field   => 'date',
        reverse => 1,
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Search::SortRule",
    xs_code           => $xs_code,
    bind_constructors => ["_new"],
    bind_methods      => [qw( Get_Field Get_Reverse )],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [qw( get_field get_reverse )],
    },
);


