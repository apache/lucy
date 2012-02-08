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

package MySchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self         = shift->SUPER::new(@_);
    my $tokenizer    = Lucy::Analysis::StandardTokenizer->new;
    my $easyanalyzer = Lucy::Analysis::EasyAnalyzer->new( language => 'en' );
    my $plain = Lucy::Plan::FullTextType->new( analyzer => $tokenizer, );
    my $easyanalyzed
        = Lucy::Plan::FullTextType->new( analyzer => $easyanalyzer );
    my $string_spec          = Lucy::Plan::StringType->new;
    my $unindexedbutanalyzed = Lucy::Plan::FullTextType->new(
        analyzer => $tokenizer,
        indexed  => 0,
    );
    my $unanalyzedunindexed = Lucy::Plan::StringType->new( indexed => 0, );
    $self->spec_field( name => 'analyzed',     type => $plain );
    $self->spec_field( name => 'easyanalyzed', type => $easyanalyzed );
    $self->spec_field( name => 'string',       type => $string_spec );
    $self->spec_field(
        name => 'unindexedbutanalyzed',
        type => $unindexedbutanalyzed
    );
    $self->spec_field(
        name => 'unanalyzedunindexed',
        type => $unanalyzedunindexed
    );
    return $self;
}

package main;
use Test::More tests => 10;

my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = MySchema->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);

$indexer->add_doc( { $_ => 'United States' } ) for qw(
    analyzed
    easyanalyzed
    string
    unindexedbutanalyzed
    unanalyzedunindexed
);

$indexer->commit;

sub check {
    my ( $field, $query_text, $expected_num_hits ) = @_;
    my $query = Lucy::Search::TermQuery->new(
        field => $field,
        term  => $query_text,
    );
    my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
    my $hits = $searcher->hits( query => $query );

    is( $hits->total_hits, $expected_num_hits, "$field correct num hits " );

    # Don't check the contents of the hit if there aren't any.
    return unless $expected_num_hits;

    my $hit = $hits->next;
    is( $hit->{$field}, 'United States', "$field correct doc returned" );
}

check( 'analyzed',             'States',        1 );
check( 'easyanalyzed',         'state',         1 );
check( 'string',               'United States', 1 );
check( 'unindexedbutanalyzed', 'state',         0 );
check( 'unindexedbutanalyzed', 'United States', 0 );
check( 'unanalyzedunindexed',  'state',         0 );
check( 'unanalyzedunindexed',  'United States', 0 );
