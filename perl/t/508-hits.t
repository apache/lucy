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

use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 9;
use Lucy::Test;
use Lucy::Test::TestUtils qw( create_index );

my @docs     = ( 'a b', 'a a b', 'a a a b', 'x' );
my $folder   = create_index(@docs);
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $hits = $searcher->hits(
    query      => 'a',
    offset     => 0,
    num_wanted => 1,
);
is( $hits->total_hits, 3, "total_hits" );
my $hit = $hits->next;
cmp_ok( $hit->get_score, '>', 0.0, "score field added" );
is( $hits->next, undef, "hits exhausted" );

$hits->next;
is( $hits->next, undef, "hits exhausted" );

my @retrieved;
@retrieved = ();
$hits      = $searcher->hits(
    query      => 'a',
    offset     => 0,
    num_wanted => 100,
);
is( $hits->total_hits, 3, "total_hits still correct" );
while ( my $hit = $hits->next ) {
    push @retrieved, $hit->{content};
}
is_deeply( \@retrieved, [ @docs[ 2, 1, 0 ] ], "correct content via next()" );

@retrieved = ();
$hits      = $searcher->hits(
    query      => 'a',
    offset     => 1,
    num_wanted => 100,
);
is( $hits->total_hits, 3, "total_hits correct with offset" );
while ( my $hit = $hits->next ) {
    push @retrieved, $hit->{content};
}
is( scalar @retrieved, 2, "number retrieved with offset" );
is_deeply( \@retrieved, [ @docs[ 1, 0 ] ], "correct content with offset" );
