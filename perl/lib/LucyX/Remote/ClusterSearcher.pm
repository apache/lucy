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
    $shards{$$self}   = $shards;
    $password{$$self} = $password;

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
    my $doc_max_responses = $self->_rpc( 'doc_max', {} );
    my $doc_max = 0;
    my @starts;
    for my $shard_doc_max (@$doc_max_responses) {
        push @starts, $doc_max;
        $doc_max += $shard_doc_max;
    }
    $starts{$$self}  = \@starts;
    $doc_max{$$self} = $doc_max;

    return $self;
}

sub DESTROY {
    my $self = shift;
    $self->close if defined $socks{$$self};
    delete $shards{$$self};
    delete $password{$$self};
    delete $socks{$$self};
    delete $starts{$$self};
    delete $doc_max{$$self};
    $self->SUPER::DESTROY;
}

=for comment

Make a remote procedure call.  For every call that does not close/terminate
the socket connection, expect a response back that's been serialized using
Storable.

=cut

sub _rpc {
    my ( $self, $method, $args ) = @_;
    my $serialized = nfreeze($args);
    my $packed_len = pack( 'N', length($serialized) );

    for my $sock ( @{ $socks{$$self} } ) {
        print $sock "$method\n$packed_len$serialized";
    }

    # Bail out if we're either closing or shutting down the server remotely.
    return if $method eq 'done';
    return if $method eq 'terminate';

    my @responses;
    for my $sock ( @{ $socks{$$self} } ) {
        # Decode response.
        $sock->read( $packed_len, 4 );
        my $arg_len = unpack( 'N', $packed_len );
        my $check_val = read( $sock, $serialized, $arg_len );
        confess("Tried to read $arg_len bytes, got $check_val")
            unless ( defined $arg_len and $check_val == $arg_len );
        my $response = thaw($serialized);
        push @responses, $response->{retval};
    }
    return \@responses;
}

sub top_docs {
    my $self = shift;
    return $self->_rpc( 'top_docs', {@_} )->[0];
}

sub terminate {
    my $self = shift;
    $self->_rpc( 'terminate', {} );
    return;
}

sub fetch_doc {
    my ( $self, $doc_id ) = @_;
    return $self->_rpc( 'fetch_doc', { doc_id => $doc_id } )->[0];
}

sub fetch_doc_vec {
    my ( $self, $doc_id ) = @_;
    return $self->_rpc( 'fetch_doc_vec', { doc_id => $doc_id } )->[0];
}

sub doc_max {
    my $self = shift;
    return $doc_max{$$self};
}

sub doc_freq {
    my $self = shift;
    my $responses = $self->_rpc( 'doc_freq', {@_} );
    my $doc_freq = 0;
    $doc_freq += $_ for @$responses;
    return $doc_freq;
}

sub close {
    my $self = shift;
    $self->_rpc( 'done', {} );
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
