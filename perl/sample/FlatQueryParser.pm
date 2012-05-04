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
use PrefixQuery;
use Carp;

our $VERSION = '0.003001';
$VERSION = eval $VERSION;

# Inherit new()

sub parse {
    my ( $self, $query_string ) = @_;
    my $tokens = $self->_tokenize($query_string);
    my $or_query = Lucy::Search::ORQuery->new;
    for my $token (@$tokens) {
        my $leaf_query = Lucy::Search::LeafQuery->new( text => $token );
        $or_query->add_child($leaf_query);
    }
    return $self->expand($or_query);
}

sub _tokenize {
    my ( $self, $query_string ) = @_;
    my @tokens;
    while ( length $query_string ) {
        if ( $query_string =~ s/^\s+// ) {
            next;    # skip whitespace
        }
        elsif ( $query_string =~ s/^("[^"]*(?:"|$))// ) {
            push @tokens, $1;    # double-quoted phrase
        }
        else {
            $query_string =~ s/(\S+)//;
            push @tokens, $1;    # single word
        }
    }
    return \@tokens;
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

