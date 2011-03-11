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

use Test::More tests => 18;
use List::Util qw( shuffle );

package ReverseType;
use base qw( Lucy::Plan::Int32Type );

sub new {
    return shift->SUPER::new( indexed => 0, sortable => 1, @_ );
}

sub compare_values {
    my ( $self, %args ) = @_;
    return $args{b} <=> $args{a};
}

package SortSchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self       = shift->SUPER::new(@_);
    my $unsortable = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::RegexTokenizer->new, );
    my $string_type = Lucy::Plan::StringType->new( sortable => 1 );
    my $int32_type = Lucy::Plan::Int32Type->new(
        indexed  => 0,
        sortable => 1,
    );
    my $int64_type = Lucy::Plan::Int64Type->new(
        indexed  => 0,
        sortable => 1,
    );
    my $float32_type = Lucy::Plan::Float32Type->new(
        indexed  => 0,
        sortable => 1,
    );
    my $float64_type = Lucy::Plan::Float64Type->new(
        indexed  => 0,
        sortable => 1,
    );
    $self->spec_field( name => 'name',    type => $string_type );
    $self->spec_field( name => 'speed',   type => $int32_type );
    $self->spec_field( name => 'sloth',   type => ReverseType->new );
    $self->spec_field( name => 'weight',  type => $int32_type );
    $self->spec_field( name => 'int32',   type => $int32_type );
    $self->spec_field( name => 'int64',   type => $int64_type );
    $self->spec_field( name => 'float32', type => $float32_type );
    $self->spec_field( name => 'float64', type => $float64_type );
    $self->spec_field( name => 'home',    type => $string_type );
    $self->spec_field( name => 'cat',     type => $string_type );
    $self->spec_field( name => 'unused',  type => $string_type );
    $self->spec_field( name => 'nope',    type => $unsortable );
    return $self;
}

package main;
use Lucy::Test;

my $airplane = {
    name   => 'airplane',
    speed  => 200,
    sloth  => 200,
    weight => 8000,
    home   => 'air',
    cat    => 'vehicle',
};
my $bike = {
    name   => 'bike',
    speed  => 15,
    sloth  => 15,
    weight => 25,
    home   => 'land',
    cat    => 'vehicle',
};
my $car = {
    name   => 'car',
    speed  => 70,
    sloth  => 70,
    weight => 3000,
    home   => 'land',
    cat    => 'vehicle',
};

my $folder = Lucy::Store::RAMFolder->new;
my $schema = SortSchema->new;
my $indexer;

sub refresh_indexer {
    $indexer->commit if $indexer;
    $indexer = Lucy::Index::Indexer->new(
        index  => $folder,
        schema => $schema,
    );
}

# First, add vehicles.
refresh_indexer();
$indexer->add_doc($_) for ( $airplane, $bike, $car );

# Add random strings.
my @random_strings;
my @letters = 'a' .. 'z';
for ( 0 .. 99 ) {
    my $string = "";
    for ( 0 .. int( rand(10) ) ) {
        $string .= $letters[ rand @letters ];
    }
    $indexer->add_doc(
        {   cat  => 'random',
            name => $string,
        }
    );
    push @random_strings, $string;
    refresh_indexer() if $_ % 10 == 0;
}
@random_strings = sort @random_strings;

# Add random int32s.
my @random_int32s;
my $i32_max = 2**31 - 1;
for ( 0 .. 99 ) {
    my $random_num = int( rand($i32_max) );
    $indexer->add_doc(
        {   cat   => 'random_int32s',
            name  => $random_num,
            int32 => $random_num,
        }
    );
    push @random_int32s, $random_num;
    refresh_indexer() if $_ % 10 == 0;
}
@random_int32s = sort { $a <=> $b } @random_int32s;

# Add random int64s.  On 32-bit Perls, precision errors may occur since we SVs
# only store numbers in doubles above U32_MAX, but that's fine because the
# errors precede the indexing stage.
my @random_int64s;
my $i64_max = 2**63 - 1;
for ( 0 .. 99 ) {
    my $random_num = int( rand($i64_max) );
    $indexer->add_doc(
        {   cat   => 'random_int64s',
            name  => $random_num,
            int64 => $random_num,
        }
    );
    push @random_int64s, $random_num;
    refresh_indexer() if $_ % 10 == 0;
}
@random_int64s = sort { $a <=> $b } @random_int64s;

# Add random float32s.
my @random_float32s;
for ( 0 .. 99 ) {
    my $random_num = rand(10);
    $random_num = unpack( "f", pack( "f", $random_num ) );   # strip precision
    $indexer->add_doc(
        {   cat     => 'random_float32s',
            name    => $random_num,
            float32 => $random_num,
        }
    );
    push @random_float32s, $random_num;
    refresh_indexer() if $_ % 10 == 0;
}
@random_float32s = sort { $a <=> $b } @random_float32s;

# Add random float64s.
my @random_float64s;
for ( 0 .. 99 ) {
    my $random_num = rand(10);
    $indexer->add_doc(
        {   cat     => 'random_float64s',
            name    => $random_num,
            float64 => $random_num,
        }
    );
    push @random_float64s, $random_num;
    refresh_indexer() if $_ % 10 == 0;
}
@random_float64s = sort { $a <=> $b } @random_float64s;

# Add numbers to verify consistent ordering.
for ( shuffle( 0 .. 99 ) ) {
    $indexer->add_doc(
        {   cat  => 'num',
            name => sprintf( '%02d', $_ ),
        }
    );
    refresh_indexer() if $_ % 10 == 0;
}

$indexer->commit;
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $results = test_sorted_search( 'vehicle', 100, name => 0 );
is_deeply( $results, [qw( airplane bike car )], "sort by one criteria" );

SKIP: {
    skip( "known leaks", 2 ) if $ENV{LUCY_VALGRIND};
    eval { $results = test_sorted_search( 'vehicle', 100, nope => 0 ) };
    like( $@, qr/sortable/,
        "sorting on a non-sortable field throws an error" );

    eval { $results = test_sorted_search( 'vehicle', 100, unknown => 0 ) };
    like( $@, qr/sortable/, "sorting on an unknown field throws an error" );
}

$results = test_sorted_search( 'vehicle', 100, weight => 0 );
is_deeply( $results, [qw( bike car airplane )], "sort by one criteria" );

$results = test_sorted_search( 'vehicle', 100, name => 1 );
is_deeply( $results, [qw( car bike airplane )], "reverse sort" );

$results = test_sorted_search( 'vehicle', 100, home => 0, name => 0 );
is_deeply( $results, [qw( airplane bike car )], "multiple criteria" );

$results = test_sorted_search( 'vehicle', 100, home => 0, name => 1 );
is_deeply( $results, [qw( airplane car bike )],
    "multiple criteria with reverse" );

$results = test_sorted_search( 'vehicle', 100, speed => 1 );
my $reversed = test_sorted_search( 'vehicle', 100, sloth => 0 );
is_deeply( $results, $reversed, "FieldType_Compare_Values" );

$results = test_sorted_search( 'random', 100, name => 0, );
is_deeply( $results, \@random_strings, "random strings" );

$results = test_sorted_search( 'random_int32s', 100, int32 => 0, );
is_deeply( $results, \@random_int32s, "int32" );

$results = test_sorted_search( 'random_int64s', 100, int64 => 0, );
is_deeply( $results, \@random_int64s, "int64" );

$results = test_sorted_search( 'random_float32s', 100, float32 => 0, );
is_deeply( $results, \@random_float32s, "float32" );

$results = test_sorted_search( 'random_float64s', 100, float64 => 0, );
is_deeply( $results, \@random_float64s, "float64" );

$results
    = test_sorted_search( 'bike bike bike car car airplane', 100, unused => 0,
    );
is_deeply( $results, [qw( airplane bike car )],
    "sorting on field with no values sorts by doc id" );

$results = test_sorted_search( '99 OR car', 10, speed => 0 );
is_deeply( $results, [qw( car 99 )], "doc with NULL value sorts last" );

my $ten_results    = test_sorted_search( 'num', 10, name => 0 );
my $thirty_results = test_sorted_search( 'num', 30, name => 0 );
my @first_ten_of_thirty = @{$thirty_results}[ 0 .. 9 ];
is_deeply( $ten_results, \@first_ten_of_thirty,
    "same order regardless of queue size" );

$ten_results    = test_sorted_search( 'num', 10, name => 1 );
$thirty_results = test_sorted_search( 'num', 30, name => 1 );
@first_ten_of_thirty = @{$thirty_results}[ 0 .. 9 ];
is_deeply( $ten_results, \@first_ten_of_thirty,
    "same order regardless of queue size (reverse sort)" );

# Add another seg to index.
undef $indexer;
$indexer = Lucy::Index::Indexer->new(
    schema => $schema,
    index  => $folder,
);
$indexer->add_doc(
    {   name   => 'carrot',
        speed  => 0,
        weight => 1,
        home   => 'land',
        cat    => 'food',
    }
);
$indexer->commit;
$searcher = Lucy::Search::IndexSearcher->new( index => $folder );

$results = test_sorted_search( 'vehicle', 100, name => 0 );
is_deeply( $results, [qw( airplane bike car )], "Multi-segment sort" );

# Take a list of criteria, create a SortSpec, perform a search, and return an
# Array of 'name' values for the sorted results.
sub test_sorted_search {
    my ( $query, $num_wanted, @criteria ) = @_;
    my @rules;

    while (@criteria) {
        my $field = shift @criteria;
        my $rev   = shift @criteria;
        push @rules,
            Lucy::Search::SortRule->new(
            field   => $field,
            reverse => $rev,
            );
    }
    push @rules, Lucy::Search::SortRule->new( type => 'doc_id' );
    my $sort_spec = Lucy::Search::SortSpec->new( rules => \@rules );
    my $hits = $searcher->hits(
        query      => $query,
        sort_spec  => $sort_spec,
        num_wanted => $num_wanted,
    );
    my @results;
    while ( my $hit = $hits->next ) {
        push @results, $hit->{name};
    }

    return \@results;
}
