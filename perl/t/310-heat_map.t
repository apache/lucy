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

use Test::More tests => 13;
use Lucy::Test;

my $heat_map = Lucy::Highlight::HeatMap->new( spans => [], );

my $big_boost = $heat_map->calc_proximity_boost(
    span1 => make_span( 0,  10, 1.0 ),
    span2 => make_span( 10, 10, 1.0 )
);
my $equally_big_boost = $heat_map->calc_proximity_boost(
    span1 => make_span( 0, 10, 1.0 ),
    span2 => make_span( 5, 4,  1.0 )
);
my $smaller_boost = $heat_map->calc_proximity_boost(
    span1 => make_span( 0,   10, 1.0 ),
    span2 => make_span( 100, 10, 1.0 )
);
my $zero_boost = $heat_map->calc_proximity_boost(
    span1 => make_span( 0,   10, 1.0 ),
    span2 => make_span( 150, 10, 1.0 )
);
is( $big_boost, $equally_big_boost,
    "overlapping and abutting produce the same proximity boost" );
cmp_ok( $big_boost, '>', $smaller_boost, "closer is better" );
is( $zero_boost, 0, "distance outside of window yields no prox boost" );

my $spans = make_spans( [ 10, 10, 1.0 ], [ 16, 14, 2.0 ] );
my $flattened = $heat_map->flatten_spans($spans);
is_deeply(
    spans_to_arg_array($flattened),
    [ [ 10, 6, 1.0 ], [ 16, 4, 3.0 ], [ 20, 10, 2.0 ] ],
    "flatten two overlapping spans"
);
my $boosts = $heat_map->generate_proximity_boosts($spans);
is_deeply(
    spans_to_arg_array($boosts),
    [ [ 10, 20, 3.0 ] ],
    "prox boosts for overlap"
);

$spans = make_spans( [ 10, 10, 1.0 ], [ 16, 14, 2.0 ], [ 50, 1, 1.0 ] );
$flattened = $heat_map->flatten_spans($spans);
is_deeply(
    spans_to_arg_array($flattened),
    [ [ 10, 6, 1.0 ], [ 16, 4, 3.0 ], [ 20, 10, 2.0 ], [ 50, 1, 1.0 ] ],
    "flatten two overlapping spans, leave hole, then third span"
);
$boosts = $heat_map->generate_proximity_boosts($spans);
is( scalar @$boosts,
    2 + 1, "boosts generated for each unique pair, since all were in range" );

$spans = make_spans( [ 10, 10, 1.0 ], [ 14, 4, 4.0 ], [ 16, 14, 2.0 ] );
$flattened = $heat_map->flatten_spans($spans);
is_deeply(
    spans_to_arg_array($flattened),
    [   [ 10, 4,  1.0 ],
        [ 14, 2,  5.0 ],
        [ 16, 2,  7.0 ],
        [ 18, 2,  3.0 ],
        [ 20, 10, 2.0 ]
    ],
    "flatten three overlapping spans"
);
$boosts = $heat_map->generate_proximity_boosts($spans);
is( scalar @$boosts,
    2 + 1, "boosts generated for each unique pair, since all were in range" );

$spans = make_spans(
    [ 10, 10, 1.0 ],
    [ 16, 14, 4.0 ],
    [ 16, 14, 2.0 ],
    [ 30, 10, 10.0 ]
);
$flattened = $heat_map->flatten_spans($spans);
is_deeply(
    spans_to_arg_array($flattened),
    [ [ 10, 6, 1.0 ], [ 16, 4, 7.0 ], [ 20, 10, 6.0 ], [ 30, 10, 10.0 ] ],
    "flatten 4 spans, middle two have identical range"
);
$boosts = $heat_map->generate_proximity_boosts($spans);
is( scalar @$boosts,
    3 + 2 + 1,
    "boosts generated for each unique pair, since all were in range"
);

$spans = make_spans(
    [ 10,  10, 1.0 ],
    [ 16,  4,  4.0 ],
    [ 16,  14, 2.0 ],
    [ 230, 10, 10.0 ]
);
$flattened = $heat_map->flatten_spans($spans);
is_deeply(
    spans_to_arg_array($flattened),
    [ [ 10, 6, 1.0 ], [ 16, 4, 7.0 ], [ 20, 10, 2.0 ], [ 230, 10, 10.0 ] ],
    "flatten 4 spans, middle two have identical starts but different ends"
);
$boosts = $heat_map->generate_proximity_boosts($spans);
is( scalar @$boosts, 2 + 1, "boosts not generated for out of range span" );

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

sub spans_to_arg_array {
    my $spans = shift;
    my @out;
    for (@$spans) {
        push @out, [ $_->get_offset, $_->get_length, $_->get_weight ];
    }
    return \@out;
}

