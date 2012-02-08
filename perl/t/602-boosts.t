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

package ControlSchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::StandardTokenizer->new );
    $self->spec_field( name => 'content',  type => $type );
    $self->spec_field( name => 'category', type => $type );
    return $self;
}

package BoostedFieldSchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self       = shift->SUPER::new(@_);
    my $tokenizer  = Lucy::Analysis::StandardTokenizer->new;
    my $plain_type = Lucy::Plan::FullTextType->new( analyzer => $tokenizer );
    my $boosted_type = Lucy::Plan::FullTextType->new(
        analyzer => $tokenizer,
        boost    => 100,
    );
    $self->spec_field( name => 'content',  type => $plain_type );
    $self->spec_field( name => 'category', type => $boosted_type );
    return $self;
}

package main;
use Test::More tests => 3;

my $control_folder       = Lucy::Store::RAMFolder->new;
my $boosted_doc_folder   = Lucy::Store::RAMFolder->new;
my $boosted_field_folder = Lucy::Store::RAMFolder->new;
my $control_indexer      = Lucy::Index::Indexer->new(
    schema => ControlSchema->new,
    index  => $control_folder,
);
my $boosted_field_indexer = Lucy::Index::Indexer->new(
    schema => BoostedFieldSchema->new,
    index  => $boosted_field_folder,
);
my $boosted_doc_indexer = Lucy::Index::Indexer->new(
    schema => ControlSchema->new,
    index  => $boosted_doc_folder,
);

my %source_docs = (
    'x'         => '',
    'x a a a a' => 'x a',
    'a b'       => 'x a a',
);

while ( my ( $content, $cat ) = each %source_docs ) {
    my %fields = (
        content  => $content,
        category => $cat,
    );
    $control_indexer->add_doc( \%fields );
    $boosted_field_indexer->add_doc( \%fields );

    my $boost = $content =~ /b/ ? 2 : 1;
    $boosted_doc_indexer->add_doc( doc => \%fields, boost => $boost );
}

$control_indexer->commit;
$boosted_field_indexer->commit;
$boosted_doc_indexer->commit;

my $searcher = Lucy::Search::IndexSearcher->new( index => $control_folder, );
my $hits = $searcher->hits( query => 'a' );
my $hit = $hits->next;
is( $hit->{content}, "x a a a a", "best doc ranks highest with no boosting" );

$searcher
    = Lucy::Search::IndexSearcher->new( index => $boosted_field_folder, );
$hits = $searcher->hits( query => 'a' );
$hit = $hits->next;
is( $hit->{content}, 'a b', "boost in FieldType works" );

$searcher = Lucy::Search::IndexSearcher->new( index => $boosted_doc_folder, );
$hits = $searcher->hits( query => 'a' );
$hit = $hits->next;
is( $hit->{content}, 'a b', "boost from \$doc->set_boost works" );
