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

my @ports = 7890 .. 7895;
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
        analyzer      => Lucy::Analysis::RegexTokenizer->new,
        highlightable => 1,
    );
    my $num_type = Lucy::Plan::Int32Type->new( sortable => 1, indexed => 0 );
    my $string_type = Lucy::Plan::StringType->new( sortable => 1 );
    $self->spec_field( name => 'content', type => $plain_type );
    $self->spec_field( name => 'junk',    type => $plain_type );
    $self->spec_field( name => 'number',  type => $num_type );
    $self->spec_field( name => 'port',    type => $string_type );
    return $self;
}

package main;

use Lucy::Test;
use LucyX::Remote::SearchServer;
use LucyX::Remote::ClusterSearcher;

my @kids;
my $number = 7;
for my $port (@ports) {
    my $kid = fork;
    if ($kid) {
        die "Failed fork: $!" unless defined $kid;
        push @kids, $kid;
    }
    else {
        my $folder  = Lucy::Store::RAMFolder->new;
        my $indexer = Lucy::Index::Indexer->new(
            index  => $folder,
            schema => SortSchema->new,
        );
        for (qw( a b c )) {
            my %doc = (
                content => "x $_ $port",
                junk    => "xyz " x 4000,    # should trigger partial reads
                number  => $number,
                port    => $port,
            );
            $indexer->add_doc( \%doc );
            $number += 2;
        }
        $indexer->commit;

        my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
        my $server = LucyX::Remote::SearchServer->new(
            searcher => $searcher,
        );
        $server->serve( port => $port );
        exit(0);
    }
}

# Allow time for the servers to set up their sockets.
sleep .5;

my $test_client_sock = IO::Socket::INET->new(
    PeerAddr => "localhost:$ports[0]",
    Proto    => 'tcp',
);
if ($test_client_sock) {
    plan( tests => 10 );
    undef $test_client_sock;
}
else {
    plan( 'skip_all', "Can't get a socket: $!" );
}

my $solo_cluster_searcher = LucyX::Remote::ClusterSearcher->new(
    schema   => SortSchema->new,
    shards   => ["localhost:$ports[0]"],
);

is( $solo_cluster_searcher->doc_freq( field => 'content', term => 'x' ),
    3, "doc_freq" );
is( $solo_cluster_searcher->doc_max, 3, "doc_max" );
isa_ok( $solo_cluster_searcher->fetch_doc(1),
    "Lucy::Document::HitDoc", "fetch_doc" );
isa_ok( $solo_cluster_searcher->fetch_doc_vec(1),
    "Lucy::Index::DocVector", "fetch_doc_vec" );

my $hits = $solo_cluster_searcher->hits( query => 'x' );
is( $hits->total_hits, 3, "retrieved hits from search server" );

$hits = $solo_cluster_searcher->hits( query => 'a' );
is( $hits->total_hits, 1, "retrieved hit from search server" );

my $cluster_searcher = LucyX::Remote::ClusterSearcher->new(
    schema   => SortSchema->new,
    shards   => [ map {"localhost:$_"} @ports ],
);

$hits = $cluster_searcher->hits( query => 'b' );
is( $hits->total_hits, scalar @ports, "matched hits across multiple shards" );

my $highlighter = Lucy::Highlight::Highlighter->new(
    searcher => $cluster_searcher,
    query    => 'b',
    field    => 'content',
);

my %content_expected = map { ( "x b $_" => 1 ) } @ports;
my %highlight_expected = map { ( "x <strong>b</strong> $_" => 1 ) } @ports;
my %content_results;
my %highlight_results;
while ( my $hit = $hits->next ) {
    $content_results{ $hit->{content} } = 1;
    my $excerpt = $highlighter->create_excerpt($hit);
    $highlight_results{$excerpt} = 1;
}

is_deeply( \%content_results, \%content_expected,
    "docs fetched from multiple shards" );
is_deeply( \%highlight_results, \%highlight_expected,
    "highlighting across multiple shards" );

my $sort_spec = Lucy::Search::SortSpec->new(
    rules => [
        Lucy::Search::SortRule->new( field => 'number' ),
        Lucy::Search::SortRule->new( type  => 'doc_id' ),
    ],
);
$hits = $cluster_searcher->hits(
    query     => 'b',
    sort_spec => $sort_spec,
);
my @got;

while ( my $hit = $hits->next ) {
    push @got, $hit->{number};
}
$sort_spec = Lucy::Search::SortSpec->new(
    rules => [
        Lucy::Search::SortRule->new( field => 'number', reverse => 1 ),
        Lucy::Search::SortRule->new( type  => 'doc_id' ),
    ],
);
$hits = $cluster_searcher->hits(
    query     => 'b',
    sort_spec => $sort_spec,
);
my @reversed;
while ( my $hit = $hits->next ) {
    push @reversed, $hit->{number};
}
is_deeply( \@got, [ reverse @reversed ],
    "Sort hits accross multiple shards" );

END {
    $solo_cluster_searcher->close if defined $solo_cluster_searcher;
    $cluster_searcher->close      if defined $cluster_searcher;
    for my $kid (@kids) {
        kill( TERM => $kid ) if $kid;
    }
}
