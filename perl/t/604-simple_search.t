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
use Lucy::Analysis::StandardTokenizer;

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::StandardTokenizer->new, );
    $self->spec_field( name => 'title', type => $type );
    $self->spec_field( name => 'body',  type => $type );
    return $self;
}

package main;

use Test::More tests => 12;
use Lucy::Test;

my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = MySchema->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
my %docs = (
    'a' => 'foo',
    'b' => 'bar',
);

while ( my ( $title, $body ) = each %docs ) {
    $indexer->add_doc(
        {   title => $title,
            body  => $body,
        }
    );
}
$indexer->commit;

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $tokenizer = Lucy::Analysis::StandardTokenizer->new;
my $or_parser = Lucy::Search::QueryParser->new(
    schema   => $schema,
    analyzer => $tokenizer,
    fields   => [ 'title', 'body', ],
);
my $and_parser = Lucy::Search::QueryParser->new(
    schema         => $schema,
    analyzer       => $tokenizer,
    fields         => [ 'title', 'body', ],
    default_boolop => 'AND',
);

sub test_qstring {
    my ( $qstring, $expected, $message ) = @_;

    my $hits = $searcher->hits( query => $qstring );
    is( $hits->total_hits, $expected, $message );

    my $query = $or_parser->parse($qstring);
    $hits = $searcher->hits( query => $query );
    is( $hits->total_hits, $expected, "OR: $message" );

    $query = $and_parser->parse($qstring);
    $hits = $searcher->hits( query => $query );
    is( $hits->total_hits, $expected, "AND: $message" );
}

test_qstring( 'a foo', 1, "simple match across multiple fields" );
test_qstring( 'a -foo', 0,
    "match of negated term on any field should exclude document" );
test_qstring(
    'a +foo',
    1,
    "failure to match of required term on a field "
        . "should not exclude doc if another field matches."
);
test_qstring( '+a +foo', 1,
    "required terms spread across disparate fields should match" );
