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
        $sock->autoflush(1);

        # Handshake.
        print $sock "$password\n";
        chomp( my $response = <$sock> );
        confess("Failed to connect: '$response'") unless $response =~ /accept/i;

        push @$socks, $sock;
    }

    # Derive doc_max and relative start offsets.
    my $doc_max_responses = $self->_multi_rpc( 'doc_max', {} );
    my $doc_max = 0;
    my @starts;
    for my $shard_doc_max (@$doc_max_responses) {
        push @starts, $doc_max;
        $doc_max += $shard_doc_max;
    }
    $starts{$$self}  = Lucy::Object::I32Array->new( ints => \@starts );
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
    my ( $self, $method, $args ) = @_;
    my $num_shards = $num_shards{$$self};
    my $request = $self->_serialize_request( $method, $args );
    for ( my $i = 0; $i < $num_shards; $i++ ) {
        $self->_send_request_to_shard( $i, $request );
    }

    # Bail out if we're either closing or shutting down the server remotely.
    return if $method eq 'done';
    return if $method eq 'terminate';

    my @responses;
    for ( my $i = 0; $i < $num_shards; $i++ ) {
        my $response = $self->_retrieve_response_from_shard($i);
        push @responses, $response->{retval};
    }
    return \@responses;
}

# Send a remote procedure call to one shard.
sub _single_rpc {
    my ( $self, $method, $args, $shard_num ) = @_;
    my $request = $self->_serialize_request( $method, $args );
    $self->_send_request_to_shard( $shard_num, $request );
    my $response = $self->_retrieve_response_from_shard($shard_num);
    return $response->{retval};
}

# Serialize a method name and hash-style parameters using the conventions
# understood by SearchServer.
sub _serialize_request {
    my ( $self, $method, $args ) = @_;
    my $serialized = nfreeze($args);
    my $packed_len = pack( 'N', length($serialized) );
    my $request    = "$method\n$packed_len$serialized";
    return \$request;
}

# Send a serialized request to one shard.
sub _send_request_to_shard {
    my ( $self, $shard_num, $request ) = @_;
    my $sock = $socks{$$self}[$shard_num];
    print $sock $$request;
}

# Retrieve the response from a shard.
sub _retrieve_response_from_shard {
    my ( $self, $shard_num ) = @_;
    my $sock = $socks{$$self}[$shard_num];
    my $packed_len;
    my $serialized;
    $sock->read( $packed_len, 4 );
    my $arg_len = unpack( 'N', $packed_len );
    my $check_val = read( $sock, $serialized, $arg_len );
    confess("Tried to read $arg_len bytes, got $check_val")
        unless ( defined $arg_len and $check_val == $arg_len );
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
    my $responses = $self->_multi_rpc( 'top_docs', \%args );
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
    $self->_multi_rpc( 'terminate', {} );
    return;
}

sub fetch_doc {
    my ( $self, $doc_id ) = @_;
    my $tick = Lucy::Index::PolyReader::sub_tick( $starts{$$self}, $doc_id );
    return $self->_single_rpc( 'fetch_doc', { doc_id => $doc_id }, $tick );
}

sub fetch_doc_vec {
    my ( $self, $doc_id ) = @_;
    my $tick = Lucy::Index::PolyReader::sub_tick( $starts{$$self}, $doc_id );
    return $self->_single_rpc( 'fetch_doc_vec', { doc_id => $doc_id },
        $tick );
}

sub doc_max {
    my $self = shift;
    return $doc_max{$$self};
}

sub doc_freq {
    my $self = shift;
    my $responses = $self->_multi_rpc( 'doc_freq', {@_} );
    my $doc_freq = 0;
    $doc_freq += $_ for @$responses;
    return $doc_freq;
}

sub close {
    my $self = shift;
    $self->_multi_rpc( 'done', {} );
    for my $sock ( @{ $socks{$$self} } ) {
        close $sock or confess("Error when closing socket: $!");
    }
    delete $socks{$$self};
}

1;

__END__

=head1 NAME

LucyX::Remote::ClusterSearcher - Connect to a remote SearchServer.

=head1 SYNOPSIS

    my $client = LucyX::Remote::ClusterSearcher->new(
        shards   => ['searchserver1:7890'], 
        password => $pass,
    );
    my $hits = $client->hits( query => $query );

=head1 DESCRIPTION

ClusterSearcher is a subclass of L<Lucy::Search::Searcher> which can be
used to search an index on a remote machine made accessible via
L<SearchServer|LucyX::Remote::SearchServer>.

=head1 METHODS

=head2 new

Constructor.  Takes hash-style params.

=over

=item *

B<shards> - An array of host:port pairs identifying the shards that make up
the composite index and that the client should connect to.

=item *

B<password> - Password to be supplied to the SearchServer when initializing
socket connection.

=back

=cut
