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

use Test::More tests => 1;
use KinoSearch::Test::TestUtils qw( create_index );

my $good   = "x a x x x x b x x x x c x";
my $better = "x x x x a x b x c x x x x";
my $best   = "x x x x x a b c x x x x x";
my $folder = create_index( $good, $better, $best );

my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );

my $hits = $searcher->hits( query => 'a b c' );

my @contents;
while ( my $hit = $hits->next ) {
    push @contents, $hit->{content};
}

TODO: {
    local $TODO = "positions not passed to boolscorer correctly yet";
    is_deeply(
        \@contents,
        [ $best, $better, $good ],
        "proximity helps boost scores"
    );
}

