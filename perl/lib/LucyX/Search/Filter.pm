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

package LucyX::Search::Filter;
BEGIN { our @ISA = qw( Lucy::Search::Query ) }
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
use Carp;
use Storable qw( nfreeze thaw );
use Scalar::Util qw( blessed weaken );
use bytes;
no bytes;

# Inside-out member vars.
our %query;
our %cached_bits;

sub new {
    my ( $either, %args ) = @_;
    my $query = delete $args{query};
    confess("required parameter query is not a Lucy::Search::Query")
        unless ( blessed($query)
        && $query->isa('Lucy::Search::Query') );
    my $self = $either->SUPER::new(%args);
    $self->_init_cache;
    $query{$$self} = $query;
    $self->set_boost(0);
    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $query{$$self};
    delete $cached_bits{$$self};
    $self->SUPER::DESTROY;
}

sub make_compiler {
    my ( $self, %args ) = @_;
    my $subordinate = delete $args{subordinate};
    my $compiler
        = LucyX::Search::FilterCompiler->new( %args, parent => $self );
    $compiler->normalize unless $subordinate;
    return $compiler;
}

sub serialize {
    my ( $self, $outstream ) = @_;
    $self->SUPER::serialize($outstream);
    my $frozen = nfreeze( $query{$$self} );
    $outstream->write_cu32( bytes::length($frozen) );
    $outstream->print($frozen);
}

sub deserialize {
    my ( $self, $instream ) = @_;
    $self->SUPER::deserialize($instream);
    my $len = $instream->read_cu32;
    my $frozen;
    $instream->read( $frozen, $len );
    $query{$$self} = thaw($frozen);
    return $self;
}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless $other->isa(__PACKAGE__);
    return 0 unless $query{$$self}->equals( $query{$$other} );
    return 0 unless $self->get_boost == $other->get_boost;
    return 1;
}

sub to_string {
    my $self = shift;
    return 'Filter(' . $query{$$self}->to_string . ')';
}

sub _bits {
    my ( $self, $seg_reader ) = @_;

    my $cached_bits = $self->_fetch_cached_bits($seg_reader);

    # Fill the cache.
    if ( !defined $cached_bits ) {
        $cached_bits = Lucy::Object::BitVector->new(
            capacity => $seg_reader->doc_max + 1 );
        $self->_store_cached_bits( $seg_reader, $cached_bits );

        my $collector = Lucy::Search::Collector::BitCollector->new(
            bit_vector => $cached_bits );

        my $polyreader = Lucy::Index::PolyReader->new(
            schema      => $seg_reader->get_schema,
            folder      => $seg_reader->get_folder,
            snapshot    => $seg_reader->get_snapshot,
            sub_readers => [$seg_reader],
        );
        my $searcher
            = Lucy::Search::IndexSearcher->new( index => $polyreader );

        # Perform the search.
        $searcher->collect(
            query     => $query{$$self},
            collector => $collector,
        );
    }

    return $cached_bits;
}

# Store a cached BitVector associated with a particular SegReader.  Store a
# weak reference to the SegReader as an indicator of cache validity.
sub _store_cached_bits {
    my ( $self, $seg_reader, $bits ) = @_;
    my $pair = { seg_reader => $seg_reader, bits => $bits };
    weaken( $pair->{seg_reader} );
    $cached_bits{$$self}{$$seg_reader} = $pair;
}

# Retrieve a cached BitVector associated with a particular SegReader.  As a
# side effect, clear away any BitVectors which are no longer valid because
# their SegReaders have gone away.
sub _fetch_cached_bits {
    my ( $self, $seg_reader ) = @_;
    my $cached_bits = $cached_bits{$$self};

    # Sweep.
    while ( my ( $addr, $pair ) = each %$cached_bits ) {
        # If weak ref has decomposed into undef, SegReader is gone... so
        # delete.
        next if defined $pair->{seg_reader};
        delete $cached_bits->{$addr};
    }

    # Fetch.
    my $pair = $cached_bits->{$$seg_reader};
    return $pair->{bits} if defined $pair;
    return;
}

# Kill any existing cached filters.
sub _init_cache {
    my $self = shift;
    $cached_bits{$$self} = {};
}

# Testing only.
sub _cached_count {
    my $self = shift;
    return scalar grep { defined $cached_bits{$$self}{$_}{seg_reader} }
        keys %{ $cached_bits{$$self} };
}

package LucyX::Search::FilterCompiler;
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
BEGIN { our @ISA = qw( Lucy::Search::Compiler ) }

sub new {
    my ( $class, %args ) = @_;
    $args{similarity} ||= $args{searcher}->get_schema->get_similarity;
    return $class->SUPER::new(%args);
}

sub make_matcher {
    my ( $self, %args ) = @_;
    my $seg_reader = $args{reader};
    my $bits       = $self->get_parent->_bits($seg_reader);
    return LucyX::Search::FilterMatcher->new(
        bits    => $bits,
        doc_max => $seg_reader->doc_max,
    );
}

package LucyX::Search::FilterMatcher;
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
BEGIN { our @ISA = qw( Lucy::Search::Matcher ) }

1;

__END__



=head1 NAME

LucyX::Search::Filter - Build a caching filter based on results of a Query.

=head1 SYNOPSIS

    my %category_filters;
    for my $category (qw( sweet sour salty bitter )) {
        my $cat_query = Lucy::Search::TermQuery->new(
            field => 'category',
            term  => $category,
        );
        $category_filters{$category} = LucyX::Search::Filter->new( 
            query => $cat_query, 
        );
    }
    
    while ( my $cgi = CGI::Fast->new ) {
        my $user_query = $cgi->param('q');
        my $filter     = $category_filters{ $cgi->param('category') };
        my $and_query  = Lucy::Search::ANDQuery->new;
        $and_query->add_child($user_query);
        $and_query->add_child($filter);
        my $hits = $searcher->hits( query => $and_query );
        ...

=head1 DESCRIPTION 

A Filter is a L<Lucy::Search::Query> subclass that can be used to filter
the results of another Query.  The effect is very similar to simply using the
wrapped inner query, but there are two important differences:

=over

=item

A Filter does not contribute to the score of the documents it matches.  

=item

A Filter caches its results, so it is more efficient if you use it more than
once.

=back

To obtain logically equivalent results to the Filter but avoid the caching,
substitute the wrapped query but use set_boost() to set its C<boost> to 0.

=head1 METHODS

=head2 new

    my $filter = LucyX::Search::Filter->new(
        query => $query;
    );

Constructor.  Takes one hash-style parameter, C<query>, which must be an
object belonging to a subclass of L<Lucy::Search::Query>.

=head1 BUGS

Filters do not cache when used in a search cluster with LucyX::Remote's
SearchServer and SearchClient.

=cut
