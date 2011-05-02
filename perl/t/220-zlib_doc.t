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

package MyArchitecture;
use base qw( Lucy::Plan::Architecture );

use LucyX::Index::ZlibDocWriter;
use LucyX::Index::ZlibDocReader;

sub register_doc_writer {
    my ( $self, $seg_writer ) = @_;
    my $doc_writer = LucyX::Index::ZlibDocWriter->new(
        schema     => $seg_writer->get_schema,
        snapshot   => $seg_writer->get_snapshot,
        segment    => $seg_writer->get_segment,
        polyreader => $seg_writer->get_polyreader,
    );
    $seg_writer->register(
        api       => "Lucy::Index::DocReader",
        component => $doc_writer,
    );
    $seg_writer->add_writer($doc_writer);
}

sub register_doc_reader {
    my ( $self, $seg_reader ) = @_;
    my $doc_reader = LucyX::Index::ZlibDocReader->new(
        schema   => $seg_reader->get_schema,
        folder   => $seg_reader->get_folder,
        segments => $seg_reader->get_segments,
        seg_tick => $seg_reader->get_seg_tick,
        snapshot => $seg_reader->get_snapshot,
    );
    $seg_reader->register(
        api       => 'Lucy::Index::DocReader',
        component => $doc_reader,
    );
}

package MySchema;
use base qw( Lucy::Plan::Schema );

sub architecture { MyArchitecture->new }

sub new {
    my $self      = shift->SUPER::new(@_);
    my $tokenizer = Lucy::Analysis::RegexTokenizer->new;
    my $main_type = Lucy::Plan::FullTextType->new( analyzer => $tokenizer );
    my $unstored_type = Lucy::Plan::FullTextType->new(
        analyzer => $tokenizer,
        stored   => 0,
    );
    my $blob_type = Lucy::Plan::BlobType->new( stored => 1 );
    $self->spec_field( name => 'content',  type => $main_type );
    $self->spec_field( name => 'smiley',   type => $main_type );
    $self->spec_field( name => 'unstored', type => $unstored_type );
    $self->spec_field( name => 'binary',   type => $blob_type );
    return $self;
}

package main;
use Test::More tests => 7;
use Lucy::Test;

my $folder = Lucy::Store::RAMFolder->new;
my $schema = MySchema->new;

my $smiley = "\x{263a}";
my $binary = pack( 'b4', 1, 2, 3, 4 );

sub add_to_index {
    my $indexer = Lucy::Index::Indexer->new(
        index  => $folder,
        schema => $schema,
    );
    for (@_) {
        $indexer->add_doc(
            {   content  => $_,
                binary   => $binary,
                smiley   => $smiley,
                unstored => $_,
            }
        );
    }
    $indexer->commit;
}

add_to_index(qw( a b c ));

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
my $hits = $searcher->hits( query => 'b' );
my $hit = $hits->next;
is( $hit->{content}, 'b',     "single segment, single hit" );
is( $hit->{smiley},  $smiley, "utf8 preserved" );
is( $hit->{binary},  $binary, "blob field binary preserved" );
ok( !defined( $hit->{unstored} ), "unstored" );

add_to_index(qw( d e f g h ));
add_to_index(qw( i j k l m ));

$searcher = Lucy::Search::IndexSearcher->new( index => $folder );
$hits = $searcher->hits( query => 'f' );
is( $hits->next->{content}, 'f', "multiple segments, single hit" );

my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->delete_by_term( field => 'content', term => $_ ) for qw( b f l );
$indexer->optimize;
$indexer->commit;

$searcher = Lucy::Search::IndexSearcher->new( index => $folder );
$hits = $searcher->hits( query => 'b' );
is( $hits->next, undef, "doc deleted" );

$hits = $searcher->hits( query => 'c' );
is( $hits->next->{content}, 'c', "map around deleted doc" );
