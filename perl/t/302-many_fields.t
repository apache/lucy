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

package MySchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::RegexTokenizer;

our %fields = ();

package main;

use Test::More tests => 10;
use Lucy::Test;

my $schema = MySchema->new;
my $type   = Lucy::Plan::FullTextType->new(
    analyzer => Lucy::Analysis::RegexTokenizer->new, );

for my $num_fields ( 1 .. 10 ) {
    # Build an index with $num_fields fields, and the same content in each.
    $schema->spec_field( name => "field$num_fields", type => $type );
    my $folder  = Lucy::Store::RAMFolder->new;
    my $indexer = Lucy::Index::Indexer->new(
        schema => $schema,
        index  => $folder,
    );

    for my $content ( 'a' .. 'z', 'x x y' ) {
        my %doc;
        for ( 1 .. $num_fields ) {
            $doc{"field$_"} = $content;
        }
        $indexer->add_doc( \%doc );
    }
    $indexer->commit;

    # See if our search results match as expected.
    my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
    my $hits = $searcher->hits(
        query      => 'x',
        num_wanted => 100,
    );
    is( $hits->total_hits, 2,
        "correct number of hits for $num_fields fields" );
    my $top_hit = $hits->next;
}
