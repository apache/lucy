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

use Test::More;
use Time::HiRes qw( sleep );
use IO::Socket::INET;

my $PORT_NUM = 7890;
BEGIN {
    if ( $^O =~ /(mswin|cygwin)/i ) {
        plan( 'skip_all', "fork on Windows not supported by Lucy" );
    }
    elsif ( $ENV{LUCY_VALGRIND} ) {
        plan( 'skip_all', "time outs cause probs under valgrind" );
    }
}

package SortSchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::RegexTokenizer;

sub new {
    my $self       = shift->SUPER::new(@_);
    my $plain_type = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::RegexTokenizer->new );
    my $string_type = Lucy::Plan::StringType->new( sortable => 1 );
    $self->spec_field( name => 'content', type => $plain_type );
    $self->spec_field( name => 'number',  type => $string_type );
    return $self;
}

package main;

use Lucy::Test;
use LucyX::Remote::SearchServer;
use LucyX::Remote::SearchClient;

my $kid;
$kid = fork;
if ($kid) {
    sleep .25;    # allow time for the server to set up the socket
    die "Failed fork: $!" unless defined $kid;
}
else {
    my $folder  = Lucy::Store::RAMFolder->new;
    my $indexer = Lucy::Index::Indexer->new(
        index  => $folder,
        schema => SortSchema->new,
    );
    my $number = 5;
    for (qw( a b c )) {
        $indexer->add_doc( { content => "x $_", number => $number } );
        $number -= 2;
    }
    $indexer->commit;

    my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
    my $server = LucyX::Remote::SearchServer->new(
        searcher => $searcher,
    );
    $server->serve( port => $PORT_NUM );
    exit(0);
}

my $test_client_sock = IO::Socket::INET->new(
    PeerAddr => "localhost:$PORT_NUM",
    Proto    => 'tcp',
);
if ($test_client_sock) {
    plan( tests => 10 );
    undef $test_client_sock;
}
else {
    plan( 'skip_all', "Can't get a socket: $!" );
}

my $searchclient = LucyX::Remote::SearchClient->new(
    schema       => SortSchema->new,
    peer_address => "localhost:$PORT_NUM",
);

is( $searchclient->doc_freq( field => 'content', term => 'x' ),
    3, "doc_freq" );
is( $searchclient->doc_max, 3, "doc_max" );
isa_ok( $searchclient->fetch_doc(1), "Lucy::Document::HitDoc", "fetch_doc" );
isa_ok( $searchclient->fetch_doc_vec(1),
    "Lucy::Index::DocVector", "fetch_doc_vec" );

my $hits = $searchclient->hits( query => 'x' );
is( $hits->total_hits, 3, "retrieved hits from search server" );

$hits = $searchclient->hits( query => 'a' );
is( $hits->total_hits, 1, "retrieved hit from search server" );

my $folder_b = Lucy::Store::RAMFolder->new;
my $number   = 6;
for (qw( a b c )) {
    my $indexer = Lucy::Index::Indexer->new(
        index  => $folder_b,
        schema => SortSchema->new,
    );
    $indexer->add_doc( { content => "y $_", number => $number } );
    $number -= 2;
    $indexer->add_doc( { content => 'blah blah blah' } ) for 1 .. 3;
    $indexer->commit;
}

my $searcher_b = Lucy::Search::IndexSearcher->new( index => $folder_b, );
is( ref( $searcher_b->get_reader ), 'Lucy::Index::PolyReader', );

my $poly_searcher = Lucy::Search::PolySearcher->new(
    schema    => SortSchema->new,
    searchers => [ $searcher_b, $searchclient ],
);

$hits = $poly_searcher->hits( query => 'b' );
is( $hits->total_hits, 2, "retrieved hits from PolySearcher" );

my %results;
$results{ $hits->next()->{content} } = 1;
$results{ $hits->next()->{content} } = 1;
my %expected = ( 'x b' => 1, 'y b' => 1, );

is_deeply( \%results, \%expected, "docs fetched from both local and remote" );

my $sort_spec = Lucy::Search::SortSpec->new(
    rules => [
        Lucy::Search::SortRule->new( field => 'number' ),
        Lucy::Search::SortRule->new( type  => 'doc_id' ),
    ],
);
$hits = $poly_searcher->hits(
    query     => 'b',
    sort_spec => $sort_spec,
);
my @got;

while ( my $hit = $hits->next ) {
    push @got, $hit->{content};
}
$sort_spec = Lucy::Search::SortSpec->new(
    rules => [
        Lucy::Search::SortRule->new( field => 'number', reverse => 1 ),
        Lucy::Search::SortRule->new( type  => 'doc_id' ),
    ],
);
$hits = $poly_searcher->hits(
    query     => 'b',
    sort_spec => $sort_spec,
);
my @reversed;
while ( my $hit = $hits->next ) {
    push @reversed, $hit->{content};
}
is_deeply(
    \@got,
    [ reverse @reversed ],
    "Sort combination of remote and local"
);

END {
    $searchclient->terminate if defined $searchclient;
    kill( TERM => $kid ) if $kid;
}
