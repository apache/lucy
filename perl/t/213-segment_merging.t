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

package NonMergingIndexManager;
use base qw( Lucy::Index::IndexManager );

sub recycle {
    return Clownfish::VArray->new;
}

# BiggerSchema is like TestSchema, but it has an extra field named "aux".
# Because "aux" sorts before "content", it forces a remapping of field numbers
# when an index created under TestSchema is opened/modified under
# BiggerSchema.
package BiggerSchema;
use base qw( Lucy::Test::TestSchema );

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = Lucy::Plan::FullTextType->new(
        analyzer      => Lucy::Analysis::StandardTokenizer->new,
        highlightable => 1,
    );
    $self->spec_field( name => 'content', type => $type );
    $self->spec_field( name => 'aux',     type => $type );
    return $self;
}

package main;
use Test::More tests => 10;
use Lucy::Test::TestUtils qw( create_index init_test_index_loc );
use File::Find qw( find );

my $index_loc = init_test_index_loc();

my $num_reps;
{
    # Verify that optimization truly cuts down on the number of segments.
    my $schema = Lucy::Test::TestSchema->new;
    for ( $num_reps = 1;; $num_reps++ ) {
        my $indexer = Lucy::Index::Indexer->new(
            index  => $index_loc,
            schema => $schema,
        );
        my $num_segmeta = num_segmeta($index_loc);
        if ( $num_reps > 2 and $num_segmeta > 1 ) {
            $indexer->optimize;
            $indexer->commit;
            $num_segmeta = num_segmeta($index_loc);
            is( $num_segmeta, 1, 'commit after optimize' );
            last;
        }
        else {
            $indexer->add_doc( { content => $_ } ) for 1 .. 5;
            $indexer->commit;
        }
    }
}

my @correct;
for my $num_letters ( reverse 1 .. 10 ) {
    my $truncate = $num_letters == 10 ? 1 : 0;
    my $indexer = Lucy::Index::Indexer->new(
        index    => $index_loc,
        truncate => $truncate,
    );

    for my $letter ( 'a' .. 'b' ) {
        my $content = ( "$letter " x $num_letters ) . ( 'z ' x 50 );
        $indexer->add_doc( { content => $content } );
        push @correct, $content if $letter eq 'b';
    }
    $indexer->commit;
}

{
    my $searcher = Lucy::Search::IndexSearcher->new( index => $index_loc );
    my $hits = $searcher->hits( query => 'b' );
    is( $hits->total_hits, 10, "correct total_hits from merged index" );
    my @got;
    push @got, $hits->next->{content} for 1 .. $hits->total_hits;
    is_deeply( \@got, \@correct,
        "correct top scoring hit from merged index" );
}

{
    # Reopen index under BiggerSchema and add some content.
    my $schema  = BiggerSchema->new;
    my $folder  = Lucy::Store::FSFolder->new( path => $index_loc );
    my $indexer = Lucy::Index::Indexer->new(
        schema  => $schema,
        index   => $folder,
        manager => NonMergingIndexManager->new,
    );
    $indexer->add_doc( { aux => 'foo', content => 'bar' } );

    # Now add some indexes.
    my $another_folder = create_index( "atlantic ocean", "fresh fish" );
    my $yet_another_folder = create_index("bonus");
    $indexer->add_index($another_folder);
    $indexer->add_index($yet_another_folder);
    $indexer->commit;
    cmp_ok( num_segmeta($index_loc), '>', 1,
        "non-merging Indexer should produce multi-seg index" );
}

{
    my $searcher = Lucy::Search::IndexSearcher->new( index => $index_loc );
    my $hits = $searcher->hits( query => 'fish' );
    is( $hits->total_hits, 1, "correct total_hits after add_index" );
    is( $hits->next->{content},
        'fresh fish', "other indexes successfully absorbed" );
}

{
    # Open an IndexReader, to prevent the deletion of files on Windows and
    # verify the file purging mechanism.
    my $schema  = BiggerSchema->new;
    my $folder  = Lucy::Store::FSFolder->new( path => $index_loc );
    my $reader  = Lucy::Index::IndexReader->open( index => $folder );
    my $indexer = Lucy::Index::Indexer->new(
        schema => $schema,
        index  => $folder,
    );
    $indexer->optimize;
    $indexer->commit;
    $reader->close;
    undef $reader;
    $indexer = Lucy::Index::Indexer->new(
        schema => $schema,
        index  => $folder,
    );
    $indexer->optimize;
    $indexer->commit;
}

is( num_segmeta($index_loc), 1, "merged segment files successfully deleted" );

{
    my $folder = Lucy::Store::RAMFolder->new;
    my $schema = Lucy::Test::TestSchema->new;
    my $number = 1;
    for ( 1 .. 3 ) {
        my $indexer = Lucy::Index::Indexer->new(
            index   => $folder,
            schema  => $schema,
            manager => NonMergingIndexManager->new,
        );
        $indexer->add_doc( { content => $number++ } ) for 1 .. 20;
        $indexer->commit;
    }
    my $indexer = Lucy::Index::Indexer->new(
        index  => $folder,
        schema => $schema,
    );
    $indexer->delete_by_term( field => 'content', term => $_ )
        for ( 3, 23, 24, 25 );
    $indexer->commit;

    ok( $folder->exists("seg_1/segmeta.json"),
        "Segment with under 10% deletions preserved"
    );
    ok( !$folder->exists("seg_2/segmeta.json"),
        "Segment with over 10% deletions merged away"
    );
}

is( Lucy::Store::FileHandle::object_count(),
    0, "All FileHandle objects have been cleaned up" );

sub num_segmeta {
    my $dir         = shift;
    my $num_segmeta = 0;
    find(
        {   no_chdir => 1,
            wanted =>
                sub { $num_segmeta++ if $File::Find::name =~ /segmeta.json/ },
        },
        $dir,
    );
    return $num_segmeta;
}
