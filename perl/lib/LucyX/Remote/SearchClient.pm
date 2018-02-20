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
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
use Carp;
use Storable qw( nfreeze thaw );

# Inside-out member vars.
our %peer_address;
our %sock;

use IO::Socket::INET;

sub new {
    my ( $either, %args ) = @_;
    my $peer_address = delete $args{peer_address};
    my $self         = $either->SUPER::new(%args);
    $peer_address{$$self} = $peer_address;

    # Establish a connection.
    my $sock = $sock{$$self} = IO::Socket::INET->new(
        PeerAddr => $peer_address,
        Proto    => 'tcp',
    );
    confess("No socket: $!") unless $sock;
    $sock->autoflush(1);
    my %handshake_args = ( _action => 'handshake' );
    my $response = $self->_rpc( \%handshake_args );
    confess("Failed to connect") unless $response;

    return $self;
}

sub DESTROY {
    my $self = shift;
    if ( defined $sock{$$self} ) {
        # Ignore exceptions in destructor.
        eval {
            $self->close;
        };
    }
    delete $peer_address{$$self};
    delete $sock{$$self};
    $self->SUPER::DESTROY;
}

=for comment

Make a remote procedure call.  For every call that does not close/terminate
the socket connection, expect a response back that's been serialized using
Storable.

=cut

sub _rpc {
    my ( $self, $args ) = @_;
    my $sock = $sock{$$self};

    my $serialized = nfreeze($args);
    my $packed_len = pack( 'N', length($serialized) );
    print $sock "$packed_len$serialized" or confess $!;

    # disabled
    #my $check_val = $sock->syswrite("$packed_len$serialized");
    #confess $! if $check_val != length($serialized) + 4;

    my $check_val;

    # Bail out if we're either closing or shutting down the server remotely.
    return if $args->{_action} eq 'done';
    return if $args->{_action} eq 'terminate';

    # Decode response.
    $check_val = $sock->read( $packed_len, 4 );
    confess("Failed to read 4 bytes: $!")
        unless $check_val == 4;
    my $arg_len = unpack( 'N', $packed_len );
    $check_val = $sock->read( $serialized, $arg_len );
    confess("Failed to read $arg_len bytes")
        unless $check_val == $arg_len;
    my $response = thaw($serialized);
    if ( exists $response->{retval} ) {
        return $response->{retval};
    }
    return;
}

sub top_docs {
    my $self = shift;
    my %args = ( @_, _action => 'top_docs' );
    return $self->_rpc( \%args );
}

sub terminate {
    my $self = shift;
    my %args = ( _action => 'terminate' );
    return $self->_rpc( \%args );
}

sub fetch_doc {
    my ( $self, $doc_id ) = @_;
    my %args = ( doc_id => $doc_id, _action => 'fetch_doc' );
    return $self->_rpc( \%args );
}

sub fetch_doc_vec {
    my ( $self, $doc_id ) = @_;
    my %args = ( doc_id => $doc_id, _action => 'fetch_doc_vec' );
    return $self->_rpc( \%args );
}

sub doc_max {
    my $self = shift;
    my %args = ( _action => 'doc_max' );
    return $self->_rpc( { _action => 'doc_max' } );
}

sub doc_freq {
    my $self = shift;
    my %args = ( @_, _action => 'doc_freq' );
    return $self->_rpc( \%args );
}

sub close {
    my $self = shift;
    $self->_rpc( { _action => 'done' } );
    my $sock = $sock{$$self};
    close $sock or confess("Error when closing socket: $!");
    delete $sock{$$self};
}

1;

__END__

=head1 NAME

LucyX::Remote::SearchClient - Connect to a remote SearchServer.

=head1 SYNOPSIS

    my $client = LucyX::Remote::SearchClient->new(
        peer_address => 'searchserver1:7890',
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

=back

=cut
