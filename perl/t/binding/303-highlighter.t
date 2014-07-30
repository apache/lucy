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

package MySchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::StandardTokenizer;

sub new {
    my $class      = shift;
    my $self       = $class->SUPER::new(@_);
    my $tokenizer  = Lucy::Analysis::StandardTokenizer->new;
    my $plain_type = Lucy::Plan::FullTextType->new(
        analyzer      => $tokenizer,
        highlightable => 1,
    );
    my $dunked_type = Lucy::Plan::FullTextType->new(
        analyzer      => $tokenizer,
        highlightable => 1,
        boost         => 0.1,
    );
    $self->spec_field( name => 'content', type => $plain_type );
    $self->spec_field( name => 'alt',     type => $dunked_type );
    return $self;
}

package MyHighlighter;
use base qw( Lucy::Highlight::Highlighter );

sub encode {
    my ( $self, $text ) = @_;
    $text =~ s/blind/wise/;
    return $text;
}

sub highlight {
    my ( $self, $text ) = @_;
    return "*$text*";
}

package main;

use Test::More tests => 6;
use Lucy::Test;

binmode( STDOUT, ":utf8" );

my $phi         = "\x{03a6}";
my $encoded_phi = "&#934;";

my $string = '1 2 3 4 5 ' x 20;    # 200 characters
$string .= "$phi a b c d x y z h i j k ";
$string .= '6 7 8 9 0 ' x 20;
my $with_quotes = '"I see," said the blind man.';

my $folder  = Lucy::Store::RAMFolder->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => MySchema->new,
);

$indexer->add_doc( { content => $_ } ) for ( $string, $with_quotes );
$indexer->add_doc(
    {   content => "x but not why or 2ee",
        alt     => $string . " and extra stuff so it scores lower",
    }
);
$indexer->commit;

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $q    = qq|"x y z" AND $phi|;
my $hits = $searcher->hits( query => $q );
my $hl   = Lucy::Highlight::Highlighter->new(
    searcher => $searcher,
    query    => $q,
    field    => 'content',
);

my $hit     = $hits->next;
my $excerpt = $hl->create_excerpt($hit);
like( $excerpt, qr#<strong>x y z</strong>#, "highlighter tagged the phrase" );
like(
    $excerpt,
    qr#<strong>$encoded_phi</strong>#i,
    "highlighter tagged the single term"
);

$hl->set_pre_tag("\e[1m");
$hl->set_post_tag("\e[0m");
like(
    $hl->create_excerpt($hit),
    qr#\e\[1m$encoded_phi\e\[0m#i, "set_pre_tag and set_post_tag",
);

$q = $searcher->glean_query("foo");
my $compiler = $q->make_compiler(
    searcher => $searcher,
    boost    => $q->get_boost,
);
$hl = Lucy::Highlight::Highlighter->new(
    searcher => $searcher,
    query    => $compiler,
    field    => 'content',
);
is( $$compiler, ${ $hl->get_query },
    "Highlighter accepts Compiler as Query" );
is( $$compiler, ${ $hl->get_compiler },
    "Highlighter uses supplied Compiler" );

$hl = MyHighlighter->new(
    searcher => $searcher,
    query    => "blind",
    field    => 'content',
);
$hits = $searcher->hits( query => 'blind' );
$hit = $hits->next;
like( $hl->create_excerpt($hit),
    qr/\*wise\*/, "override both Encode() and Highlight()" );

