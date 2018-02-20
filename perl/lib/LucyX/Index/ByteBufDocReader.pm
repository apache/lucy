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

package LucyX::Index::ByteBufDocReader;
use base qw( Lucy::Index::DocReader );
use Lucy::Document::HitDoc;
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
use Carp;

# Inside-out member vars.
our %width;
our %field;
our %instream;

sub new {
    my ( $either, %args ) = @_;
    my $width = delete $args{width};
    my $field = delete $args{field};
    my $self  = $either->SUPER::new(%args);
    confess("Missing required param 'width'") unless defined $width;
    confess("Missing required param 'field'") unless $field;
    if ( $width < 1 ) { confess("'width' must be at least 1") }
    $width{$$self} = $width;
    $field{$$self} = $field;

    my $segment  = $self->get_segment;
    my $metadata = $self->get_segment->fetch_metadata("bytebufdocs");
    if ($metadata) {
        if ( $metadata->{format} != 1 ) {
            confess("Unrecognized format: '$metadata->{format}'");
        }
        my $filename = $segment->get_name . "/bytebufdocs.dat";
        $instream{$$self} = $self->get_folder->open_in($filename)
            or confess Clownfish->error;
    }

    return $self;
}

sub fetch_doc {
    my ( $self, $doc_id ) = @_;
    my $field = $field{$$self};
    my %fields = ( $field => '' );
    $self->read_record( $doc_id, \$fields{$field} );
    return Lucy::Document::HitDoc->new(
        doc_id => $doc_id,
        fields => \%fields,
    );
}

sub read_record {
    my ( $self, $doc_id, $buf ) = @_;
    my $instream = $instream{$$self};
    if ($instream) {
        my $width = $width{$$self};
        $instream->seek( $width * $doc_id );
        $instream->read( $$buf, $width );
    }
}

sub close {
    my $self = shift;
    delete $width{$$self};
    delete $instream{$$self};
}

sub DESTROY {
    my $self = shift;
    delete $width{$$self};
    delete $field{$$self};
    delete $instream{$$self};
    $self->SUPER::DESTROY;
}

1;

__END__

__POD__

=head1 NAME

LucyX::Index::ByteBufDocReader - Read a Doc as a fixed-width byte array.

=head1 SYNOPSIS

    # See LucyX::Index::ByteBufDocWriter

=head1 DESCRIPTION

This is a proof-of-concept class to demonstrate alternate implementations for
fetching documents.  It is unsupported.

=cut

