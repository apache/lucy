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

package LucyX::Remote::SearchClient;
BEGIN { our @ISA = qw( Lucy::Search::Searcher ) }
use Carp;
use Storable qw( nfreeze thaw );
use bytes;
no bytes;

# Inside-out member vars.
our %peer_address;
our %password;
our %sock;

use IO::Socket::INET;

sub new {
    my ( $either, %args ) = @_;
    my $peer_address = delete $args{peer_address};
    my $password     = delete $args{password};
    my $self         = $either->SUPER::new(%args);
    $peer_address{$$self} = $peer_address;
    $password{$$self}     = $password;

    # Establish a connection.
    my $sock = $sock{$$self} = IO::Socket::INET->new(
        PeerAddr => $peer_address,
        Proto    => 'tcp',
    );
    confess("No socket: $!") unless $sock;
    $sock->autoflush(1);

    # Verify password.
    print $sock "$password\n";
    chomp( my $response = <$sock> );
    confess("Failed to connect: '$response'") unless $response =~ /accept/i;

    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $peer_address{$$self};
    delete $password{$$self};
    delete $sock{$$self};
    $self->SUPER::DESTROY;
}

=for comment

Make a remote procedure call.  For every call that does not close/terminate
the socket connection, expect a response back that's been serialized using
Storable.

=cut

sub _rpc {
    my ( $self, $method, $args ) = @_;
    my $sock = $sock{$$self};

    my $serialized = nfreeze($args);
    my $packed_len = pack( 'N', bytes::length($serialized) );
    print $sock "$method\n$packed_len$serialized";

    # Bail out if we're either closing or shutting down the server remotely.
    return if $method eq 'done';
    return if $method eq 'terminate';

    # Decode response.
    $sock->read( $packed_len, 4 );
    my $arg_len = unpack( 'N', $packed_len );
    my $check_val = read( $sock, $serialized, $arg_len );
    confess("Tried to read $arg_len bytes, got $check_val")
        unless ( defined $arg_len and $check_val == $arg_len );
    my $response = thaw($serialized);
    if ( exists $response->{retval} ) {
        return $response->{retval};
    }
    return;
}

sub top_docs {
    my $self = shift;
    return $self->_rpc( 'top_docs', {@_} );
}

sub terminate {
    my $self = shift;
    return $self->_rpc( 'terminate', {} );
}

sub fetch_doc {
    my ( $self, $doc_id ) = @_;
    return $self->_rpc( 'fetch_doc', { doc_id => $doc_id } );
}

sub fetch_doc_vec {
    my ( $self, $doc_id ) = @_;
    return $self->_rpc( 'fetch_doc_vec', { doc_id => $doc_id } );
}

sub doc_max {
    my $self = shift;
    return $self->_rpc( 'doc_max', {} );
}

sub doc_freq {
    my $self = shift;
    return $self->_rpc( 'doc_freq', {@_} );
}

sub close {
    my $self = shift;
    $self->_rpc( 'done', {} );
    my $sock = $sock{$$self};
    close $sock or confess("Error when closing socket: $!");
    delete $sock{$$self};
}

sub NUKE {
    my $self = shift;
    $self->close if defined $sock{$$self};
}

1;

__END__

=head1 NAME

LucyX::Remote::SearchClient - Connect to a remote SearchServer.

=head1 SYNOPSIS

    my $client = LucyX::Remote::SearchClient->new(
        peer_address => 'searchserver1:7890',
        password     => $pass,
    );
    my $hits = $client->hits( query => $query );

=head1 DESCRIPTION

SearchClient is a subclass of L<Lucy::Search::Searcher> which can be
used to search an index on a remote machine made accessible via
L<SearchServer|LucyX::Remote::SearchServer>.

=head1 METHODS

=head2 new

Constructor.  Takes hash-style params.

=over

=item *

B<peer_address> - The name/IP and the port number which the client should
attempt to connect to.

=item *

B<password> - Password to be supplied to the SearchServer when initializing
socket connection.

=back

=cut
