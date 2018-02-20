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

package LucyX::Remote::SearchServer;
BEGIN { our @ISA = qw( Clownfish::Obj ) }
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
use Carp;
use Storable qw( nfreeze thaw );
use Scalar::Util qw( reftype );

# Inside-out member vars.
our %searcher;

use IO::Socket::INET;
use IO::Select;

sub new {
    my ( $either, %args ) = @_;
    my $searcher = delete $args{searcher};
    my $self     = $either->SUPER::new(%args);
    $searcher{$$self} = $searcher;

    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $searcher{$$self};
    $self->SUPER::DESTROY;
}

my %dispatch = (
    handshake     => \&do_handshake,
    terminate     => \&do_terminate,
    doc_max       => \&do_doc_max,
    doc_freq      => \&do_doc_freq,
    top_docs      => \&do_top_docs,
    fetch_doc     => \&do_fetch_doc,
    fetch_doc_vec => \&do_fetch_doc_vec,
);

sub serve {
    my ( $self, %args ) = @_;
    # Establish a listening socket.
    my $port = delete $args{port};
    confess("Invalid port: $port") unless $port =~ /^\d+$/;
    my $main_sock = IO::Socket::INET->new(
        LocalPort => $port,
        Proto     => 'tcp',
        Listen    => SOMAXCONN,
        Reuse     => 1,
    );
    confess("No socket: $!") unless $main_sock;
    $self->serve_sock($main_sock);
}

sub serve_sock {
    my ( $self, $main_sock ) = @_;

    my $read_set = IO::Select->new($main_sock);

    while ( my @ready = $read_set->can_read ) {
        for my $readhandle (@ready) {
            # If this is the main handle, we have a new client, so accept.
            if ( $readhandle == $main_sock ) {
                my $client_sock = $main_sock->accept;
                $read_set->add($client_sock);
            }
            # Otherwise it's a client sock, so process the request.
            else {
                my $client_sock = $readhandle;
                my $status      = $self->serve_rpc($client_sock);

                # If "done", the client's closing.
                if ( $status eq 'done' ) {
                    $read_set->remove($client_sock);
                    $client_sock->close;
                    next;
                }
                # Remote signal to close the server.
                elsif ( $status eq 'terminate' ) {
                    my @all_handles = $read_set->handles;
                    $read_set->remove( \@all_handles );
                    $client_sock->close;
                    $main_sock->close;
                    return;
                }
            }
        }
    }
}

sub serve_rpc {
    my ( $self, $client_sock ) = @_;
    my ( $check_val, $buf, $len );
    $check_val = $client_sock->read( $buf, 4 );
    # If read returns 0, socket has been closed cleanly at
    # the other end.
    return 'done' if $check_val == 0;
    confess $! unless $check_val == 4;
    $len = unpack( 'N', $buf );
    $check_val = $client_sock->read( $buf, $len );
    confess $! unless $check_val == $len;
    my $args = eval { thaw($buf) };
    confess $@ if $@;
    confess "Not a hashref" unless reftype($args) eq 'HASH';
    my $method = delete $args->{_action};

    # If "done", the client's closing.
    return $method if $method eq 'done';

    # Process the method call.
    $dispatch{$method}
        or confess "ERROR: Bad method name: $method\n";
    my $response   = $dispatch{$method}->( $self, $args );
    my $frozen     = nfreeze($response);
    my $packed_len = pack( 'N', length($frozen) );
    print $client_sock "$packed_len$frozen"
        or confess $!;

    # Remote signal to close the server.
    return $method if $method eq 'terminate';

    return 'continue';
}

sub do_handshake {
    my ( $self, $args ) = @_;
    my $retval = 1;
    return { retval => $retval };
}

sub do_terminate {
    return { retval => 1 };
}

sub do_doc_freq {
    my ( $self, $args ) = @_;
    return { retval => $searcher{$$self}->doc_freq(%$args) };
}

sub do_top_docs {
    my ( $self, $args ) = @_;
    my $top_docs = $searcher{$$self}->top_docs(%$args);
    return { retval => $top_docs };
}

sub do_doc_max {
    my ( $self, $args ) = @_;
    my $doc_max = $searcher{$$self}->doc_max;
    return { retval => $doc_max };
}

sub do_fetch_doc {
    my ( $self, $args ) = @_;
    my $doc = $searcher{$$self}->fetch_doc( $args->{doc_id} );
    return { retval => $doc };
}

sub do_fetch_doc_vec {
    my ( $self, $args ) = @_;
    my $doc_vec = $searcher{$$self}->fetch_doc_vec( $args->{doc_id} );
    return { retval => $doc_vec };
}

1;

__END__

=head1 NAME

LucyX::Remote::SearchServer - Make a Searcher remotely accessible.

=head1 SYNOPSIS

    my $searcher = Lucy::Search::IndexSearcher->new( 
        index => '/path/to/index' 
    );
    my $search_server = LucyX::Remote::SearchServer->new(
        searcher => $searcher
    );
    $search_server->serve(
        port => 7890
    );

=head1 DESCRIPTION 

The SearchServer class, in conjunction with either
L<SearchClient|LucyX::Remote::SearchClient> or
L<ClusterSearcher|LucyX::Remote::ClusterSearcher>, makes it possible to run a
search on one machine and report results on another.  

By aggregating several SearchClients under a ClusterSearcher, the cost of
searching what might have been a prohibitively large monolithic index can be
distributed across multiple nodes, each with its own, smaller index.

=head1 METHODS

=head2 new

    my $search_server = LucyX::Remote::SearchServer->new(
        searcher => $searcher, # required
    );

Constructor.  Takes hash-style parameters.

=over

=item *

B<searcher> - the L<Searcher|Lucy::Search::IndexSearcher> that the SearchServer
will wrap.

=back

=head2 serve

    $search_server->serve(
        port => 7890,      # required
    );

Open a listening socket on localhost and wait for SearchClients to connect.

=over

=item *

B<port> - the port on localhost that the server should open and listen on.

=back

=head2 serve_rpc

    my $status = $search_server->serve_rpc($sock);

Handle a single RPC from socket $sock. Returns 'done' if the connection should
be closed. Returns 'terminate' if the server should shut down. Returns
'continue' if the server should continue to handle requests from this client.

=cut
