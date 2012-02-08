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

use Lucy::Test;

package NoMergeSeg1Manager;
use base qw( Lucy::Index::IndexManager );
sub recycle {
    my $seg_readers = shift->SUPER::recycle(@_);
    @$seg_readers = grep { $_->get_seg_num != 1 } @$seg_readers;
    return $seg_readers;
}

package DelSchema;
use base 'Lucy::Plan::Schema';

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::StandardTokenizer->new, );
    $self->spec_field( name => 'foo', type => $type );
    $self->spec_field( name => 'bar', type => $type );
    return $self;
}

package main;

use Test::More tests => 70;

my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = DelSchema->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->add_doc( { foo => 'foo', bar => $_ } ) for qw( x y z );
$indexer->commit;

for my $iter ( 1 .. 10 ) {
    is( search_doc('foo'), 3, "match all docs prior to deletion $iter" );
    is( search_doc('x'),   1, "match doc to be deleted $iter" );

    $indexer = Lucy::Index::Indexer->new(
        schema => $schema,
        index  => $folder,
    );
    $indexer->delete_by_term( field => 'bar', term => 'x' );
    $indexer->optimize;
    $indexer->commit;

    is( search_doc('x'),   0, "deletion successful $iter" );
    is( search_doc('foo'), 2, "match all docs after deletion $iter" );

    $indexer = Lucy::Index::Indexer->new(
        schema => $schema,
        index  => $folder,
    );
    $indexer->add_doc( { foo => 'foo', bar => 'x' } );
    $indexer->optimize;
    $indexer->commit;
}

$folder  = Lucy::Store::RAMFolder->new;
$schema  = DelSchema->new;
$indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
my @dox = ( 'a' .. 'z' );
$indexer->add_doc( { foo => 'foo', bar => $_ } ) for @dox;
$indexer->commit;

for ( 1 .. 10 ) {
    $indexer = Lucy::Index::Indexer->new(
        manager => NoMergeSeg1Manager->new,
        schema  => $schema,
        index   => $folder,
    );
    $indexer->delete_by_term( field => 'bar', term => 'y' );
    $indexer->commit;
    my @num_seg_1_bv_files = grep {/deletions-seg_1.bv/} @{ $folder->list_r };
    is( scalar @num_seg_1_bv_files,
        1, "seg_1 deletions file carried forward" );

    $indexer = Lucy::Index::Indexer->new(
        manager => NoMergeSeg1Manager->new,
        schema  => $schema,
        index   => $folder,
    );
    $indexer->delete_by_term( field => 'bar', term => 'new' );
    $indexer->add_doc( { foo => 'foo', bar => 'new' } );
    $indexer->commit;
    @num_seg_1_bv_files = grep {/deletions-seg_1.bv/} @{ $folder->list_r };
    is( scalar @num_seg_1_bv_files,
        1, "seg_1 deletions file carried forward" );

    is( search_doc('foo'), scalar @dox, "deletions applied correctly" );
}

sub search_doc {
    my $query_string = shift;
    my $searcher     = Lucy::Search::IndexSearcher->new( index => $folder );
    my $hits         = $searcher->hits( query => $query_string );
    return $hits->total_hits;
}
