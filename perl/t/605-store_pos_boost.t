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

package MyRegexTokenizer;
use base qw( Lucy::Analysis::Analyzer );
use Lucy::Analysis::Inversion;

sub transform {
    my ( $self, $inversion ) = @_;
    my $new_inversion = Lucy::Analysis::Inversion->new;

    while ( my $token = $inversion->next ) {
        for ( $token->get_text ) {
            my $this_time = /z/ ? 1 : 0;
            # Accumulate token start_offsets and end_offsets.
            while (/(\w)/g) {
                # Special boost just for one doc.
                my $boost = ( $1 eq 'a' and $this_time ) ? 100 : 1;
                $new_inversion->append(
                    Lucy::Analysis::Token->new(
                        text         => "$1",
                        start_offset => $-[0],
                        end_offset   => $+[0],
                        boost        => $boost,
                    ),
                );
            }
        }
    }

    return $new_inversion;
}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless ref($self) eq ref($other);
    return 1;
}

package RichSim;
use base qw( Lucy::Index::Similarity );
use Lucy::Index::Posting::RichPosting;

sub make_posting {
    Lucy::Index::Posting::RichPosting->new( similarity => shift );
}

package MySchema::boosted;
use base qw( Lucy::Plan::FullTextType );

sub make_similarity { RichSim->new }

package MySchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::RegexTokenizer;

sub new {
    my $self       = shift->SUPER::new(@_);
    my $plain_type = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::RegexTokenizer->new );
    my $boosted_type
        = MySchema::boosted->new( analyzer => MyRegexTokenizer->new, );
    $self->spec_field( name => 'plain',   type => $plain_type );
    $self->spec_field( name => 'boosted', type => $boosted_type );
    return $self;
}

package main;

use Test::More tests => 2;

my $good    = "x x x a a x x x x x x x x";
my $better  = "x x x a a a x x x x x x x";
my $best    = "x x x a a a a a a a a a a";
my $boosted = "z x x a x x x x x x x x x";

my $schema  = MySchema->new;
my $folder  = Lucy::Store::RAMFolder->new;
my $indexer = Lucy::Index::Indexer->new(
    schema => $schema,
    index  => $folder,
);

for ( $good, $better, $best, $boosted ) {
    $indexer->add_doc( { plain => $_, boosted => $_ } );
}
$indexer->commit;

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $q_for_plain = Lucy::Search::TermQuery->new(
    field => 'plain',
    term  => 'a',
);
my $hits = $searcher->hits( query => $q_for_plain );
is( $hits->next->{plain},
    $best, "verify that search on unboosted field returns best match" );

my $q_for_boosted = Lucy::Search::TermQuery->new(
    field => 'boosted',
    term  => 'a',
);
$hits = $searcher->hits( query => $q_for_boosted );
is( $hits->next->{boosted},
    $boosted, "artificially boosted token overrides better match" );

