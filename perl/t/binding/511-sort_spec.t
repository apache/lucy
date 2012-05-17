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

use Test::More tests => 7;

package ReverseType;
use base qw( Lucy::Plan::Int32Type );

sub new {
    return shift->SUPER::new( indexed => 0, sortable => 1, @_ );
}

sub compare_values {
    my ( $self, %args ) = @_;
    return $args{b} <=> $args{a};
}

package main;
use Lucy::Test;

my $schema = Lucy::Plan::Schema->new;

my $unsortable = Lucy::Plan::FullTextType->new(
    analyzer => Lucy::Analysis::StandardTokenizer->new
);
my $string_type = Lucy::Plan::StringType->new( sortable => 1 );
my $int32_type = Lucy::Plan::Int32Type->new(
    indexed  => 0,
    sortable => 1,
);

$schema->spec_field( name => 'name',    type => $string_type );
$schema->spec_field( name => 'speed',   type => $int32_type );
$schema->spec_field( name => 'sloth',   type => ReverseType->new );
$schema->spec_field( name => 'weight',  type => $int32_type );
$schema->spec_field( name => 'home',    type => $string_type );
$schema->spec_field( name => 'cat',     type => $string_type );
$schema->spec_field( name => 'nope',    type => $unsortable );

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
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);

# First, add vehicles.
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
}
@random_strings = sort @random_strings;

$indexer->commit;
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $results = test_sorted_search( 'vehicle', 100, name => 0 );
is_deeply( $results, [qw( airplane bike car )], "sort by one criteria" );

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
