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

package FlatQueryParser;
use base qw( Lucy::Search::QueryParser );
use Lucy::Search::TermQuery;
use Lucy::Search::PhraseQuery;
use Lucy::Search::ORQuery;
use Lucy::Search::NoMatchQuery;
use PrefixQuery;
use Parse::RecDescent;
use Carp;

our %rd_parser;

my $grammar = <<'END_GRAMMAR';

tree:
    leaf_queries
    { 
        $return = Lucy::Search::ORQuery->new;
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
    { Lucy::Search::LeafQuery->new( text => $1 ) }

phrase_query:
    /("[^"]*(?:"|$))/   # terminated by either quote or end of string
    { Lucy::Search::LeafQuery->new( text => $1 ) }
    
prefix_query:
    /(\w+\*)/
    { Lucy::Search::LeafQuery->new( text => $1 ) }

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
        : Lucy::Search::NoMatchQuery->new;
}

sub tree {
    my ( $self, $query_string ) = @_;
    return $rd_parser{$$self}->tree($query_string);
}

sub expand_leaf {
    my ( $self, $leaf_query ) = @_;
    my $text = $leaf_query->get_text;
    if ( $text =~ /\*$/ ) {
        my $or_query = Lucy::Search::ORQuery->new;
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

    my $searcher = Lucy::Search::IndexSearcher->new( 
        index => '/path/to/index' 
    );
    my $parser = FlatQueryParser->new( $searcher->get_schema );
    my $query  = $parser->parse($query_string);
    my $hits   = $searcher->hits( query => $query );
    ...

=head1 DESCRIPTION

See L<Lucy::Docs::Cookbook::CustomQueryParser>.

=cut

