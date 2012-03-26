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

my $schema  = Lucy::Test::TestSchema->new;
my $type    = $schema->fetch_type('content');
my $folder  = Lucy::Store::RAMFolder->new;
my $indexer = Lucy::Index::Indexer->new(
    schema => $schema,
    index  => $folder,
);

my %one_field    = ( content => 'x 1' );
my %two_fields   = ( content => 'x x 2', a => 'a' );
my %three_fields = ( content => 'x x x 3', a => 'a', b => 'b' );
my %four_fields  = ( content => 'x x x 3', a => 'a', b => 'b', c => 'c', );
my %foo_doc      = ( content => 'foo', foo => 'foo' );
my %five_fields = (
    content => 'x x x x x 5',
    a       => 'a',
    b       => 'b',
    c       => 'c',
    foo     => 'stuff'
);

$indexer->add_doc( \%one_field );
$schema->spec_field( name => 'a', type => $type );
$indexer->add_doc( \%two_fields );
pass('Add a field in the middle of indexing');

$schema->spec_field( name => 'a', type => $type );
pass('Add same field again');

$schema->spec_field( name => 'b', type => $type );
$indexer->add_doc( \%three_fields );
pass('Add another field');
$indexer->commit;

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
my $hits = $searcher->hits( query => 'x', num_wanted => 100 );
is( $hits->total_hits, 3,
    "disparate docs successfully indexed and retrieved" );
my $top_hit = $hits->next;
is_deeply( $top_hit->get_fields, \%three_fields,
    "all fields stored successfully" );

my $schema2 = Lucy::Test::TestSchema->new;
my $folder2 = Lucy::Store::RAMFolder->new;
$schema2->spec_field( name => 'foo', type => $type );
my $indexer2 = Lucy::Index::Indexer->new(
    schema => $schema2,
    index  => $folder2,
);
$indexer2->add_doc( \%foo_doc );
$indexer2->commit;

undef $indexer;
$indexer = Lucy::Index::Indexer->new(
    schema => $schema,
    index  => $folder,
);

$schema->spec_field( name => 'c', type => $type );
$indexer->add_doc( \%four_fields );

$indexer->add_index($folder2);
$indexer->add_doc( \%five_fields );
pass('successfully absorbed new field def during add_index');
$indexer->commit;

$searcher = Lucy::Search::IndexSearcher->new( index => $folder );
$hits = $searcher->hits( query => 'stuff', num_wanted => 100 );
is( $hits->total_hits, 1,
    "successfully aborbed unknown field during add_index" );
$top_hit = $hits->next;
delete $top_hit->{score};
is_deeply( $top_hit->get_fields, \%five_fields,
    "all fields stored successfully" );

$hits = $searcher->hits( query => 'x', num_wanted => 100 );
is( $hits->total_hits, 5, "indexes successfully merged" );
