use strict;
use warnings;

package EvensOnlyCollector;
use base qw( KinoSearch::Search::Collector );

our %doc_ids;

sub new {
    my $self = shift->SUPER::new;
    $doc_ids{$$self} = [];
    return $self;
}

sub collect {
    my ( $self, $doc_id ) = @_;
    if ( $doc_id % 2 == 0 ) {
        push @{ $doc_ids{$$self} }, $doc_id;
    }
}

sub get_doc_ids { $doc_ids{ ${ +shift } } }

sub DESTROY {
    my $self = shift;
    delete $doc_ids{$$self};
    $self->SUPER::DESTROY;
}

package main;

use Test::More tests => 1;
use KinoSearch::Test;
use KSx::Search::MockScorer;

my $collector = EvensOnlyCollector->new;
my $matcher = KSx::Search::MockScorer->new( doc_ids => [ 1, 5, 10, 1000 ], );
$collector->set_matcher($matcher);
while ( my $doc_id = $matcher->next ) {
    $collector->collect($doc_id);
}
is_deeply(
    $collector->get_doc_ids,
    [ 10, 1000 ],
    "Collector can be subclassed"
);

