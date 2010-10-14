use strict;
use warnings;

package MockSearcher;
use base qw( KinoSearch::Search::Searcher );

package MyQuery;
use base qw( KinoSearch::Search::Query );

sub make_compiler {
    my $self = shift;
    return MyCompiler->new( @_, parent => $self );
}

package MyCompiler;
use base qw( KinoSearch::Search::Compiler );

sub apply_norm_factor {
    my ( $self, $factor ) = @_;
    $self->SUPER::apply_norm_factor($factor);
}

package main;
use Test::More tests => 1;

my $q = KinoSearch::Search::ORQuery->new;
for ( 1 .. 50 ) {
    my @kids = ( $q, ( MyQuery->new ) x 10 );
    $q = KinoSearch::Search::ORQuery->new( children => \@kids );
}
my $searcher = MockSearcher->new( schema => KinoSearch::Plan::Schema->new );
my $compiler = $q->make_compiler( searcher => $searcher );

pass("Made it through deep recursion with multiple stack reallocations");

