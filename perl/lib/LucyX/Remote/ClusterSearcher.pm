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

package LucyX::Remote::ClusterSearcher;
BEGIN { our @ISA = qw( Lucy::Search::Searcher ) }
use Carp;
use Storable qw( nfreeze thaw );
use Scalar::Util qw( reftype );

# Inside-out member vars.
our %shards;
our %num_shards;
our %password;
our %socks;
our %starts;
our %doc_max;

use IO::Socket::INET;

sub new {
    my ( $either, %args ) = @_;
    my $shards   = delete $args{shards};
    my $password = delete $args{password};
    my $self     = $either->SUPER::new(%args);
    confess("'shards' must be an arrayref")
        unless reftype($shards) eq 'ARRAY';
    $shards{$$self}     = $shards;
    $num_shards{$$self} = scalar @$shards;
    $password{$$self}   = $password;

    # Establish connections.
    my $socks = $socks{$$self} = [];
    for my $shard (@$shards) {
        my $sock = IO::Socket::INET->new(
            PeerAddr => $shard,
            Proto    => 'tcp',
        );
        confess("No socket: $!") unless $sock;
        push @$socks, $sock;
    }

    # Handshake with servers.
    my %handshake_args = ( password => $password, _action => 'handshake' );
    my $responses = $self->_multi_rpc( \%handshake_args );
    for my $response (@$responses) {
        confess unless $response;
    }

    # Derive doc_max and relative start offsets.
    my $doc_max_responses = $self->_multi_rpc( { _action => 'doc_max' } );
    my $doc_max = 0;
    my @starts;
    for my $shard_doc_max (@$doc_max_responses) {
        push @starts, $doc_max;
        $doc_max += $shard_doc_max;
    }
    $starts{$$self} = Lucy::Object::I32Array->new( ints => \@starts );
    $doc_max{$$self} = $doc_max;

    return $self;
}

sub DESTROY {
    my $self = shift;
    $self->close if defined $socks{$$self};
    delete $shards{$$self};
    delete $num_shards{$$self};
    delete $password{$$self};
    delete $socks{$$self};
    delete $starts{$$self};
    delete $doc_max{$$self};
    $self->SUPER::DESTROY;
}

# Send a remote procedure call to all shards.
sub _multi_rpc {
    my ( $self, $args ) = @_;
    my $num_shards = $num_shards{$$self};
    my $request    = $self->_serialize_request($args);
    for ( my $i = 0; $i < $num_shards; $i++ ) {
        $self->_send_request_to_shard( $i, $request );
    }

    # Bail out if we're either closing or shutting down the server remotely.
    return if $args->{_action} eq 'done';
    return if $args->{_action} eq 'terminate';

    my @responses;
    for ( my $i = 0; $i < $num_shards; $i++ ) {
        my $response = $self->_retrieve_response_from_shard($i);
        push @responses, $response->{retval};
    }
    return \@responses;
}

# Send a remote procedure call to one shard.
sub _single_rpc {
    my ( $self, $args, $shard_num ) = @_;
    my $request = $self->_serialize_request($args);
    $self->_send_request_to_shard( $shard_num, $request );
    my $response = $self->_retrieve_response_from_shard($shard_num);
    return $response->{retval};
}

# Serialize a method name and hash-style parameters using the conventions
# understood by SearchServer.
sub _serialize_request {
    my ( $self, $args ) = @_;
    my $serialized = nfreeze($args);
    my $packed_len = pack( 'N', length($serialized) );
    my $request    = "$packed_len$serialized";
    return \$request;
}

# Send a serialized request to one shard.
sub _send_request_to_shard {
    my ( $self, $shard_num, $request ) = @_;
    my $sock      = $socks{$$self}[$shard_num];
    my $check_val = $sock->syswrite($$request);
    confess $! unless $check_val == length($$request);
}

# Retrieve the response from a shard.
sub _retrieve_response_from_shard {
    my ( $self, $shard_num ) = @_;
    my $sock = $socks{$$self}[$shard_num];
    my $packed_len;
    my $serialized;
    my $check_val = $sock->sysread( $packed_len, 4 );
    confess $! unless $check_val == 4;
    my $arg_len = unpack( 'N', $packed_len );
    $check_val = $sock->sysread( $serialized, $arg_len );
    confess $! unless $check_val == $arg_len;
    return thaw($serialized);
}

sub top_docs {
    my ( $self, %args ) = @_;
    my $starts     = $starts{$$self};
    my $num_shards = $num_shards{$$self};
    my $query      = $args{query};
    my $num_wanted = $args{num_wanted};
    my $sort_spec  = $args{sort_spec};

    # Weight query if necessary.
    my $compiler
        = $query->isa("Lucy::Search::Compiler")
        ? $query
        : $query->make_compiler( searcher => $self );

    # Create HitQueue.
    my $hit_q;
    if ($sort_spec) {
        $hit_q = Lucy::Search::HitQueue->new(
            schema    => $self->get_schema,
            sort_spec => $sort_spec,
            wanted    => $num_wanted,
        );
    }
    else {
        $hit_q = Lucy::Search::HitQueue->new( wanted => $num_wanted, );
    }

    # Gather remote responses and aggregate.
    $args{_action} = 'top_docs';
    my $responses  = $self->_multi_rpc( \%args );
    my $total_hits = 0;
    for ( my $i = 0; $i < $num_shards; $i++ ) {
        my $base           = $starts->get($i);
        my $sub_top_docs   = $responses->[$i];
        my @sub_match_docs = sort { $a->get_doc_id <=> $b->get_doc_id }
            @{ $sub_top_docs->get_match_docs };
        for my $match_doc (@sub_match_docs) {
            $match_doc->set_doc_id( $match_doc->get_doc_id + $base );
            $hit_q->insert($match_doc);
        }
        $total_hits += $sub_top_docs->get_total_hits;
    }

    # Return a TopDocs object with the best of the best.
    my $best_match_docs = $hit_q->pop_all;
    return Lucy::Search::TopDocs->new(
        total_hits => $total_hits,
        match_docs => $best_match_docs,
    );
}

sub terminate {
    my $self = shift;
    $self->_multi_rpc( { _action => 'terminate' } );
    return;
}

sub fetch_doc {
    my ( $self, $doc_id ) = @_;
    my $starts = $starts{$$self};
    my $tick   = Lucy::Index::PolyReader::sub_tick( $starts, $doc_id );
    my $start  = $starts->get($tick);
    my %args   = ( doc_id => $doc_id - $start, _action => 'fetch_doc' );
    return $self->_single_rpc( \%args, $tick );
}

sub fetch_doc_vec {
    my ( $self, $doc_id ) = @_;
    my $starts = $starts{$$self};
    my $tick   = Lucy::Index::PolyReader::sub_tick( $starts, $doc_id );
    my $start  = $starts->get($tick);
    my %args   = ( doc_id => $doc_id - $start, _action => 'fetch_doc_vec' );
    return $self->_single_rpc( \%args, $tick );
}

sub doc_max {
    my $self = shift;
    return $doc_max{$$self};
}

sub doc_freq {
    my $self      = shift;
    my %args      = ( @_, _action => 'doc_freq' );
    my $responses = $self->_multi_rpc( \%args );
    my $doc_freq  = 0;
    $doc_freq += $_ for @$responses;
    return $doc_freq;
}

sub close {
    my $self = shift;
    $self->_multi_rpc( { _action => 'done' } );
    for my $sock ( @{ $socks{$$self} } ) {
        close $sock or confess("Error when closing socket: $!");
    }
    delete $socks{$$self};
}

1;

__END__

=head1 NAME

LucyX::Remote::ClusterSearcher - Search multiple remote indexes.

=head1 SYNOPSIS

    my $searcher = eval {
        LucyX::Remote::ClusterSearcher->new(
            schema => MySchema->new,
            shards => [ 'search1:7890', 'search2:7890', 'search3:7890' ],
        );
    };
    ...
    my $hits = eval { $searcher->hits( query => $query ) };

=head1 DESCRIPTION

ClusterSearcher is a subclass of L<Lucy::Search::Searcher> which can be used
to search a composite index made up of multiple shards, where each shard is
represented by a host:port pair running L<LucyX::Remote::SearchServer>.

=head1 METHODS

=head2 new

Constructor.  Takes hash-style params.

=over

=item *

B<schema> - A Schema, which must match the Schema used by each remote node.

=item *

B<shards> - An array of host:port pairs running LucyX::Remote::SearchServer
instances, which identifying the shards that make up the composite index.

=item *

B<password> - Optional password to be supplied to the SearchServers when
initializing socket connections.

=back

=cut
