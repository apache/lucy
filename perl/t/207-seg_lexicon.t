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
use utf8;

use Test::More tests => 5;
use Lucy::Test;

package TestAnalyzer;
use base qw( Lucy::Analysis::Analyzer );
sub transform { $_[1] }

package MySchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = Lucy::Plan::FullTextType->new( analyzer => TestAnalyzer->new );
    $self->spec_field( name => 'a', type => $type );
    $self->spec_field( name => 'b', type => $type );
    $self->spec_field( name => 'c', type => $type );
    return $self;
}

package main;

my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = MySchema->new;
my $indexer = Lucy::Index::Indexer->new(
    create => 1,
    index  => $folder,
    schema => $schema,
);

# We need to test strings that exceed the Latin-1 range to make sure that
# get_term treats them correctly. (See change 3103 in the svn repo.)
my @animals = qw( cat dog sloth λεοντάρι змейка );
for my $animal (@animals) {
    $indexer->add_doc(
        {   a => $animal,
            b => $animal,
            c => $animal,
        }
    );
}
$indexer->commit;

my $snapshot = Lucy::Index::Snapshot->new->read_file( folder => $folder );
my $segment = Lucy::Index::Segment->new( number => 1 );
$segment->read_file($folder);
my $lex_reader = Lucy::Index::DefaultLexiconReader->new(
    schema   => $schema,
    folder   => $folder,
    snapshot => $snapshot,
    segments => [$segment],
    seg_tick => 0,
);
my %lexicons;
for (qw( a b c )) {
    $lexicons{$_} = $lex_reader->lexicon( field => $_ );
}

my @fields;
my @terms;
for (qw( a b c )) {
    my $lexicon = $lexicons{$_};
    while ( $lexicon->next ) {
        push @fields, $lexicon->get_field;
        push @terms,  $lexicon->get_term;
    }
}
is_deeply( \@fields, [qw( a a a a a b b b b b c c c c c )],
    "correct fields" );
my @correct_texts = (@animals) x 3;
is_deeply( \@terms, \@correct_texts, "correct terms" );

my $lexicon = $lexicons{b};
$lexicon->seek("dog");
$lexicon->next;
is( $lexicon->get_term,  'sloth', "lexicon seeks to correct term (ptr)" );
is( $lexicon->get_field, 'b',     "lexicon has correct field" );

$lexicon->reset;
$lexicon->next;
is( $lexicon->get_term, 'cat', "reset" );
