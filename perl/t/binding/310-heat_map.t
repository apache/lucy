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

use Test::More tests => 3;
use Lucy::Test;

my $heat_map = Lucy::Highlight::HeatMap->new( spans => [], );

my $boost = $heat_map->calc_proximity_boost(
    span1 => make_span( 0,  10, 1.0 ),
    span2 => make_span( 10, 10, 1.0 )
);
cmp_ok( $boost, '>', 0, "calc_proximity_boost" );

my $spans = make_spans( [ 10, 10, 1.0 ], [ 16, 14, 2.0 ] );
my $flattened = $heat_map->flatten_spans($spans);
is( scalar @$flattened, 3, "flatten_spans" );
my $boosts = $heat_map->generate_proximity_boosts($spans);
is( scalar @$boosts, 1, "generate_proximity_boosts" );

sub make_span {
    return Lucy::Search::Span->new(
        offset => $_[0],
        length => $_[1],
        weight => $_[2],
    );
}

sub make_spans {
    my @spans;
    for my $arg_ref (@_) {
        push @spans, make_span( @{$arg_ref}[ 0 .. 2 ] );
    }
    return \@spans;
}

