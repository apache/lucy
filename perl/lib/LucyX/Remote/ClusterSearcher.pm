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
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
use Carp;
use Storable qw( nfreeze thaw );
use Scalar::Util qw( reftype );

# Inside-out member vars.
our %shards;
our %num_shards;
our %starts;
our %doc_max;

use IO::Socket::INET;

sub new {
    my ( $either, %args ) = @_;
    my $addrs    = delete $args{shards};
    my $self     = $either->SUPER::new(%args);
    confess("'shards' must be an arrayref")
        unless reftype($addrs) eq 'ARRAY';
    $num_shards{$$self} = scalar @$addrs;

    # Establish connections.
    my @shards;
    for my $addr (@$addrs) {
        my $sock = IO::Socket::INET->new(
            PeerAddr => $addr,
            Proto    => 'tcp',
            Blocking => 0,
        );
        confess("No socket: $!") unless $sock;
        push @shards,
            {
            addr => $addr,
            sock => $sock,
            };
    }
    $shards{$$self} = \@shards;

    # Handshake with servers.
    my %handshake_args = ( _action => 'handshake' );
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
    $self->close if defined $shards{$$self};
    delete $shards{$$self};
    delete $num_shards{$$self};
    delete $starts{$$self};
    delete $doc_max{$$self};
    $self->SUPER::DESTROY;
}

# Send a remote procedure call to all shards.
sub _multi_rpc {
    my ( $self, $args ) = @_;
    return $self->_rpc( $args, $shards{$$self} );
}

# Send a remote procedure call to one shard.
sub _single_rpc {
    my ( $self, $args, $shard_num ) = @_;
    my $shard = $shards{$$self}[$shard_num];
    my $responses = $self->_rpc( $args, [$shard] );
    return $responses->[0];
}

sub _rpc {
    my ( $self, $args, $shards ) = @_;

    my $request  = $self->_serialize_request($args);
    my $timeout  = 5;
    my $shutdown = $args->{_action} eq 'done'
        || $args->{_action} eq 'terminate';

    my ( $rin, $win, $ein ) = ( '', '', '' );

    # Initialize shards to send the request
    for my $shard (@$shards) {
        my $fileno = $shard->{sock}->fileno;
        vec( $win, $fileno, 1 ) = 1;
        $shard->{response} = undef;
        $shard->{error}    = undef;
        $shard->{buf}      = $request;
        $shard->{sent}     = 0;
        $shard->{callback} = \&_cb_send;
        $shard->{shutdown} = $shutdown;
    }

    my $remaining = @$shards;

    # Event loop
    while ( $remaining > 0 ) {
        my ( $rout, $wout, $eout );

        my $n = select( $rout = $rin, $wout = $win, $eout = $ein, $timeout );

        confess("select: $!")  if $n == -1;
        confess("I/O timeout") if $n == 0;

        for my $shard (@$shards) {
            next if !$shard->{callback};
            my $fileno = $shard->{sock}->fileno;
            next if !vec( $rout, $fileno, 1 ) && !vec( $wout, $fileno, 1 );
            # Dispatch event
            $shard->{callback}->( $shard, \$rin, \$win, \$ein );
            --$remaining if !$shard->{callback};
        }
    }

    # Collect responses and cleanup
    my @responses;
    my @errors;
    for my $shard (@$shards) {
        if ( defined $shard->{error} ) {
            push( @errors, $shard->{error} . ' @ ' . $shard->{addr} );
        }
        else {
            push( @responses, $shard->{response}{retval} );
        }
        $shard->{response} = undef;
        $shard->{error}    = undef;
        $shard->{buf}      = undef;
    }
    confess( 'RPC error: ' . join( ', ', @errors ) ) if @errors;
    return \@responses;
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

# Send a (partial) request to a shard
sub _cb_send {
    my ( $shard, $rin, $win, $ein ) = @_;

    my $msg = substr( ${ $shard->{buf} }, $shard->{sent} );
    my $sent = $shard->{sock}->send($msg);

    if ( !defined($sent) ) {
        $shard->{error}    = $!;
        $shard->{callback} = undef;
        vec( $$win, $shard->{sock}->fileno, 1 ) = 0;
        return;
    }

    $shard->{sent} += $sent;

    if ( $sent >= length($msg) ) {
        # Complete
        my $fileno = $shard->{sock}->fileno;
        vec( $$win, $fileno, 1 ) = 0;
        if ( $shard->{shutdown} ) {
            # Bail out if we're either closing or shutting down the server
            # remotely.
            $shard->{callback} = undef;
        }
        else {
            # Setup shard to read response length
            $shard->{buf}      = '';
            $shard->{callback} = \&_cb_recv_length;
            vec( $$rin, $fileno, 1 ) = 1;
        }
    }
}

# Receive a (partial) response length from a shard
sub _cb_recv_length {
    my ( $shard, $rin, $win, $ein ) = @_;

    my $data;
    my $r = $shard->{sock}->recv( $data, 4 - length( $shard->{buf} ) );

    if ( !defined($r) || length($data) == 0 ) {
        $shard->{error} = !defined($r) ? $! : 'Remote shutdown';
        $shard->{callback} = undef;
        vec( $$rin, $shard->{sock}->fileno, 1 ) = 0;
        return;
    }

    $shard->{buf} .= $data;

    if ( length( $shard->{buf} ) >= 4 ) {
        # Complete, setup shard to receive response
        $shard->{response_size} = unpack( 'N', $shard->{buf} );
        $shard->{buf}           = '';
        $shard->{callback}      = \&_cb_recv_response;
    }
}

# Receive a (partial) response from a shard
sub _cb_recv_response {
    my ( $shard, $rin, $win, $ein ) = @_;

    my $data;
    my $remaining = $shard->{response_size} - length( $shard->{buf} );
    my $r = $shard->{sock}->recv( $data, $remaining );

    if ( !defined($r) || length($data) == 0 ) {
        $shard->{error} = !defined($r) ? $! : 'Remote shutdown';
        $shard->{callback} = undef;
        vec( $$rin, $shard->{sock}->fileno, 1 ) = 0;
        return;
    }

    $shard->{buf} .= $data;

    if ( length( $shard->{buf} ) >= $shard->{response_size} ) {
        # Finished
        $shard->{response} = thaw( $shard->{buf} );
        $shard->{callback} = undef;
    }
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
        : $query->make_compiler(
            searcher => $self,
            boost    => $query->get_boost,
          );

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
    my $starts  = $starts{$$self};
    my $tick    = Lucy::Index::PolyReader::sub_tick( $starts, $doc_id );
    my $start   = $starts->get($tick);
    my %args    = ( doc_id => $doc_id - $start, _action => 'fetch_doc' );
    my $hit_doc = $self->_single_rpc( \%args, $tick );
    $hit_doc->set_doc_id($doc_id);
    return $hit_doc;
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
    return unless $shards{$$self};
    $self->_multi_rpc( { _action => 'done' } );
    for my $shard ( @{ $shards{$$self} } ) {
        close $shard->{sock} or confess("Error when closing socket: $!");
    }
    delete $shards{$$self};
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

=back

=cut
