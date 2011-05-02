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

package MatchOnlySim;
use base qw( Lucy::Index::Similarity );

sub make_posting {
    my $self = shift;
    return Lucy::Index::Posting::MatchPosting->new( similarity => $self );
}

package MatchSchema::MatchOnly;
use base qw( Lucy::Plan::FullTextType );
use Lucy::Index::Posting::MatchPosting;

sub make_similarity { MatchOnlySim->new }

package MatchSchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::RegexTokenizer;

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = MatchSchema::MatchOnly->new(
        analyzer => Lucy::Analysis::RegexTokenizer->new );
    $self->spec_field( name => 'content', type => $type );
    return $self;
}

package main;

use Lucy::Test::TestUtils qw( get_uscon_docs );
use Test::More tests => 6;

my $uscon_docs   = get_uscon_docs();
my $match_folder = make_index( MatchSchema->new, $uscon_docs );
my $score_folder = make_index( Lucy::Test::TestSchema->new, $uscon_docs );

my $match_searcher
    = Lucy::Search::IndexSearcher->new( index => $match_folder );
my $score_searcher
    = Lucy::Search::IndexSearcher->new( index => $score_folder );

for (qw( land of the free )) {
    my $match_got = hit_ids_array( $match_searcher, $_ );
    my $score_got = hit_ids_array( $score_searcher, $_ );
    is_deeply( $match_got, $score_got, "same hits for '$_'" );
}

my $qstring          = '"the legislature"';
my $should_have_hits = hit_ids_array( $score_searcher, $qstring );
my $should_be_empty  = hit_ids_array( $match_searcher, $qstring );
ok( scalar @$should_have_hits, "successfully scored phrase $qstring" );
ok( !scalar @$should_be_empty, "no hits matched for phrase $qstring" );

sub make_index {
    my ( $schema, $docs ) = @_;
    my $folder  = Lucy::Store::RAMFolder->new;
    my $indexer = Lucy::Index::Indexer->new(
        schema => $schema,
        index  => $folder,
    );
    $indexer->add_doc( { content => $_->{bodytext} } ) for values %$docs;
    $indexer->commit;
    return $folder;
}

sub hit_ids_array {
    my ( $searcher, $query_string ) = @_;
    my $query = $searcher->glean_query($query_string);

    my $bit_vec
        = Lucy::Object::BitVector->new( capacity => $searcher->doc_max + 1 );
    my $bit_collector
        = Lucy::Search::Collector::BitCollector->new( bit_vector => $bit_vec,
        );
    $searcher->collect( query => $query, collector => $bit_collector );
    return $bit_vec->to_array->to_arrayref;
}
