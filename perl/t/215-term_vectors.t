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
use Lucy::Test;

package MySchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = Lucy::Plan::FullTextType->new(
        analyzer      => Lucy::Analysis::RegexTokenizer->new,
        highlightable => 1,
    );
    $self->spec_field( name => 'content', type => $type );
    return $self;
}

package main;
use utf8;
use Test::More tests => 5;
use Storable qw( freeze thaw );

my $schema  = MySchema->new;
my $folder  = Lucy::Store::RAMFolder->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);

my $hasta = 'hasta la mañana';
for ( 'a b c foo foo bar', $hasta ) {
    $indexer->add_doc( { content => $_ } );
}
$indexer->commit;

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
my $doc_vec = $searcher->fetch_doc_vec(1);

my $term_vector = $doc_vec->term_vector( field => "content", term => "foo" );
ok( defined $term_vector, "successfully retrieved term vector" );

$doc_vec = $searcher->fetch_doc_vec(2);
$term_vector = $doc_vec->term_vector( field => 'content', term => 'mañana' );

ok( defined $term_vector, "utf-8 term vector retrieved" );
is( $term_vector->get_end_offsets->get(0),
    length $hasta,
    "end offset in utf8 characters, not bytes"
);

# Reopen the Folder under a new Schema with two fields.  The new field ("aux")
# sorts lexically before "content" so that "content" will have a new field
# num.  This tests the field num mapping during merging.
my $alt_folder = Lucy::Store::RAMFolder->new;
my $alt_schema = MySchema->new;
my $type       = $alt_schema->fetch_type('content');
$alt_schema->spec_field( name => 'aux', type => $type );

$indexer = Lucy::Index::Indexer->new(
    schema => $alt_schema,
    index  => $alt_folder,
);
for ( 'blah blah blah ', 'yada yada yada ' ) {
    $indexer->add_doc(
        {   content => $_,
            aux     => $_ . $_,
        }
    );
}
$indexer->commit;

$indexer = Lucy::Index::Indexer->new(
    schema => $alt_schema,
    index  => $alt_folder,
);
$indexer->add_index($folder);
$indexer->commit;

$searcher = Lucy::Search::IndexSearcher->new( index => $alt_folder );
my $hits = $searcher->hits( query => $hasta );
my $hit_id = $hits->next->get_doc_id;
$doc_vec = $searcher->fetch_doc_vec($hit_id);
$term_vector = $doc_vec->term_vector( field => 'content', term => 'mañana' );
ok( defined $term_vector, "utf-8 term vector retrieved after merge" );

my $dupe = thaw( freeze($term_vector) );
ok( $term_vector->equals($dupe), "freeze/thaw" );
