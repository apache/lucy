use strict;
use warnings;

package FlatQueryParser;
use base qw( KinoSearch::Search::QueryParser );
use KinoSearch::Search::TermQuery;
use KinoSearch::Search::PhraseQuery;
use KinoSearch::Search::ORQuery;
use KinoSearch::Search::NoMatchQuery;
use PrefixQuery;
use Parse::RecDescent;
use Carp;

our %rd_parser;

my $grammar = <<'END_GRAMMAR';

tree:
    leaf_queries
    { 
        $return = KinoSearch::Search::ORQuery->new;
        $return->add_child($_) for @{ $item[1] };
    }

leaf_queries:
    leaf_query(s?)
    { $item{'leaf_query(s?)'} }

leaf_query:
      phrase_query
    | prefix_query
    | term_query
    
term_query:
    /(\S+)/
    { KinoSearch::Search::LeafQuery->new( text => $1 ) }

phrase_query:
    /("[^"]*(?:"|$))/   # terminated by either quote or end of string
    { KinoSearch::Search::LeafQuery->new( text => $1 ) }
    
prefix_query:
    /(\w+\*)/
    { KinoSearch::Search::LeafQuery->new( text => $1 ) }

END_GRAMMAR

sub new {
    my $class = shift;
    my $self  = $class->SUPER::new(@_);
    $rd_parser{$$self} = Parse::RecDescent->new($grammar);
    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $rd_parser{$$self};
    $self->SUPER::DESTROY;
}

sub parse {
    my ( $self, $query_string ) = @_;
    my $tree = $self->tree($query_string);
    return $tree
        ? $self->expand($tree)
        : KinoSearch::Search::NoMatchQuery->new;
}

sub tree {
    my ( $self, $query_string ) = @_;
    return $rd_parser{$$self}->tree($query_string);
}

sub expand_leaf {
    my ( $self, $leaf_query ) = @_;
    my $text = $leaf_query->get_text;
    if ( $text =~ /\*$/ ) {
        my $or_query = KinoSearch::Search::ORQuery->new;
        for my $field ( @{ $self->get_fields } ) {
            my $prefix_query = PrefixQuery->new(
                field        => $field,
                query_string => $text,
            );
            $or_query->add_child($prefix_query);
        }
        return $or_query;
    }
    else {
        return $self->SUPER::expand_leaf($leaf_query);
    }
}

1;

__END__

=head1 NAME

FlatQueryParser - Simple query parser, with no boolean operators.

=head1 SYNOPSIS

    my $searcher = KinoSearch::Search::IndexSearcher->new( 
        index => '/path/to/index' 
    );
    my $parser = FlatQueryParser->new( $searcher->get_schema );
    my $query  = $parser->parse($query_string);
    my $hits   = $searcher->hits( query => $query );
    ...

=head1 DESCRIPTION

See L<KinoSearch::Docs::Cookbook::CustomQueryParser>.

=cut

