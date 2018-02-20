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

package LucyX::Index::ZlibDocReader;
use base qw( Lucy::Index::DocReader );
our $VERSION = '0.006002';
$VERSION = eval $VERSION;
use Lucy::Util::StringHelper qw( utf8_valid utf8_flag_on );
use Compress::Zlib qw( uncompress );
use Carp;

# Inside-out member vars.
our %ix_in;
our %dat_in;
our %binary_fields;

sub new {
    my ( $either, %args ) = @_;
    my $self = $either->SUPER::new(%args);

    # Validate metadata.  Only open streams if the segment has data we
    # recognize.
    my $segment  = $self->get_segment;
    my $metadata = $segment->fetch_metadata("zdocs");
    if ($metadata) {
        if ( $metadata->{format} != 1 ) {
            confess("Unrecognized format: '$metadata->{format}'");
        }

        # Open InStreams.
        my $dat_filename = $segment->get_name . "/zdocs.dat";
        my $ix_filename  = $segment->get_name . "/zdocs.ix";
        my $folder       = $self->get_folder;
        $ix_in{$$self} = $folder->open_in($ix_filename)
            or confess Clownfish->error;
        $dat_in{$$self} = $folder->open_in($dat_filename)
            or confess Clownfish->error;

        # Remember which fields are binary.
        my $schema = $self->get_schema;
        my $bin_fields = $binary_fields{$$self} = {};
        $bin_fields->{$_} = 1
            for grep { $schema->fetch_type($_)->binary }
            @{ $schema->all_fields };
    }

    return $self;
}

sub fetch_doc {
    my ( $self, $doc_id ) = @_;
    my $dat_in     = $dat_in{$$self};
    my $ix_in      = $ix_in{$$self};
    my $bin_fields = $binary_fields{$$self};

    # Bail if no data in segment.
    return unless $ix_in;

    # Read index information.
    $ix_in->seek( $doc_id * 8 );
    my $start = $ix_in->read_i64;
    my $len   = $ix_in->read_i64 - $start;
    my $compressed;

    # Read main data.
    $dat_in->seek($start);
    $dat_in->read( $compressed, $len );
    my $inflated      = uncompress($compressed);
    my $num_fields    = unpack( "w", $inflated );
    my $pack_template = "w ";
    $pack_template .= "w/a*" x ( $num_fields * 2 );
    my ( undef, %fields ) = unpack( $pack_template, $inflated );

    # Turn on UTF-8 flag for string fields.
    for my $field ( keys %fields ) {
        next if $bin_fields->{$field};
        utf8_flag_on( $fields{$field} );
        confess("Invalid UTF-8 read for doc $doc_id field '$field'")
            unless utf8_valid( $fields{$field} );
    }

    return Lucy::Document::HitDoc->new(
        fields => \%fields,
        doc_id => $doc_id,
    );
}

sub read_record {
    my ( $self, $doc_id, $buf ) = @_;
    my $dat_in     = $dat_in{$$self};
    my $ix_in      = $ix_in{$$self};
    my $bin_fields = $binary_fields{$$self};

    if ($ix_in) {
        $ix_in->seek( $doc_id * 8 );
        my $start = $ix_in->read_i64;
        my $len   = $ix_in->read_i64 - $start;
        $dat_in->seek($start);
        $dat_in->read( $$buf, $len );
    }
}

sub close {
    my $self = shift;
    delete $ix_in{$$self};
    delete $dat_in{$$self};
    delete $binary_fields{$$self};
}

sub DESTROY {
    my $self = shift;
    delete $ix_in{$$self};
    delete $dat_in{$$self};
    delete $binary_fields{$$self};
    $self->SUPER::DESTROY;
}

1;

__END__

__POD__

=head1 NAME

LucyX::Index::ZlibDocReader - Compressed doc storage.

=head1 DESCRIPTION

This is a proof-of-concept class to demonstrate alternate implementations for
fetching documents.  It is unsupported.

=cut
