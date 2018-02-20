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

package LucyX::Index::ZlibDocWriter;
use base qw( Lucy::Index::DataWriter );
use Carp;
use Scalar::Util qw( blessed );
use Compress::Zlib qw( compress );
use Lucy::Util::StringHelper qw( cat_bytes );
use Clownfish;
use bytes;
no bytes;

our $VERSION = '0.006002';
$VERSION = eval $VERSION;

# Inside-out member vars.
our %ix_out;
our %dat_out;

# Inherit constructor.

sub _lazy_init {
    my $self = shift;

    # Get outstreams.  Skip past non-doc #0.
    my $folder   = $self->get_folder;
    my $ix_file  = $self->get_segment->get_name . "/zdocs.ix";
    my $dat_file = $self->get_segment->get_name . "/zdocs.dat";
    $ix_out{$$self} = $folder->open_out($ix_file)
        or confess Clownfish->error;
    $dat_out{$$self} = $folder->open_out($dat_file)
        or confess Clownfish->error;
    $ix_out{$$self}->write_i64(0);
}

sub add_inverted_doc {
    my ( $self, %args ) = @_;
    _lazy_init($self) unless $ix_out{$$self};
    my $inverter = $args{inverter};
    my $ix_out   = $ix_out{$$self};
    my $dat_out  = $dat_out{$$self};

    # Check doc id.
    my $expected = $ix_out->tell / 8;
    confess("Expected doc id $expected, got '$args{doc_id}'")
        unless $args{doc_id} == $expected;

    my $to_compress = "";
    my $count       = 0;
    my $schema      = $self->get_schema;
    $inverter->iterate;
    while ( $inverter->next ) {
        next unless $inverter->get_type->stored;
        my $name  = $inverter->get_field_name;
        my $value = $inverter->get_value;
        cat_bytes( $to_compress, pack( "w", bytes::length($name) ) );
        cat_bytes( $to_compress, $name );
        cat_bytes( $to_compress, pack( "w", bytes::length($value) ) );
        cat_bytes( $to_compress, $value );
        $count++;
    }
    # Prepend count of fields to store in this Doc.
    $to_compress = pack( "w", $count ) . $to_compress;

    # Write file pointer to index file.  Write compressed serialized string to
    # main file.
    $ix_out->write_i64( $dat_out->tell );
    $dat_out->print( compress($to_compress) );
}

sub add_segment {
    my ( $self, %args ) = @_;
    my $seg_reader = $args{reader};
    my $doc_map    = $args{doc_map};
    my $doc_max    = $seg_reader->doc_max;

    # Bail if the supplied segment is empty. */
    return unless $doc_max;

    _lazy_init($self) unless $ix_out{$$self};
    my $ix_out     = $ix_out{$$self};
    my $dat_out    = $dat_out{$$self};
    my $doc_reader = $seg_reader->obtain("Lucy::Index::DocReader");
    confess("Not a ZlibDocReader")
        unless ( blessed($doc_reader)
        and $doc_reader->isa("LucyX::Index::ZlibDocReader") );

    for ( my $i = 1; $i <= $doc_max; $i++ ) {
        next unless $doc_map->get($i);
        my $buf;
        $doc_reader->read_record( $i, \$buf );
        $ix_out->write_i64( $dat_out->tell );
        $dat_out->print($buf);
    }
}

sub finish {
    my $self    = shift;
    my $ix_out  = $ix_out{$$self};
    my $dat_out = $dat_out{$$self};
    if ($ix_out) {
        # Write one extra file pointer so that we can always derive record
        # length.
        $ix_out->write_i64( $dat_out->tell );

        # Close streams and store metadata.
        $ix_out->close;
        $dat_out->close;
        my $segment = $self->get_segment;
        $segment->store_metadata(
            key      => 'zdocs',
            metadata => $self->metadata,
        );
    }
}

sub format {1}

sub DESTROY {
    my $self = shift;
    delete $ix_out{$$self};
    delete $dat_out{$$self};
    $self->SUPER::DESTROY;
}

1;

__END__

__POD__

=head1 NAME

LucyX::Index::ZlibDocWriter - Compressed doc storage.

=head1 DESCRIPTION

This is a proof-of-concept class to demonstrate alternate implementations for
fetching documents.  It is unsupported.

=cut
