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

package Clownfish::File;
use Clownfish::Util qw( verify_args a_isa_b );
use Clownfish::Parcel;
use Scalar::Util qw( blessed );
use File::Spec::Functions qw( catfile );
use Carp;

# Inside out member vars.
our %blocks;
our %source_class;
our %parcel;
our %modified;

our %new_PARAMS = (
    source_class => undef,
    parcel       => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $package = ref($either) || $either;
    my $self = $either->_new();
    $parcel{$self} = Clownfish::Parcel->acquire( $args{parcel} );
    $blocks{$self} = [];
    $source_class{$self} = $args{source_class};
    $modified{$self} = 0;
    confess("Missing required param 'source_class'")
        unless $self->get_source_class;
    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $parcel{$self};
    delete $blocks{$self};
    delete $source_class{$self};
    delete $modified{$self};
    $self->_destroy;
}

our %block_types = (
    'Clownfish::Parcel' => 1,
    'Clownfish::Class'  => 1,
    'Clownfish::CBlock' => 1,
);

sub add_block {
    my ( $self, $block ) = @_;
    my $block_class = ref($block);
    confess("Invalid block type: $block_class")
        unless $block_types{$block_class};
    push @{ $blocks{$self} }, $block;
}

sub get_modified     { $modified{ +shift } }
sub set_modified     { $modified{ $_[0] } = $_[1] }
sub get_source_class { $source_class{ +shift } }

sub blocks { $blocks{ +shift } }

sub classes {
    my $self = shift;
    my @classes
        = grep { ref $_ and $_->isa('Clownfish::Class') } @{ $self->blocks };
    return \@classes;
}

# Return a string used for an include guard, unique per file.
sub guard_name {
    my $self       = shift;
    my $guard_name = "H_" . uc( $self->get_source_class );
    $guard_name =~ s/\W+/_/g;
    return $guard_name;
}

# Return a string opening the include guard.
sub guard_start {
    my $self       = shift;
    my $guard_name = $self->guard_name;
    return "#ifndef $guard_name\n#define $guard_name 1\n";
}

# Return a string closing the include guard.  Other classes count on being
# able to match this string.
sub guard_close {
    my $self       = shift;
    my $guard_name = $self->guard_name;
    return "#endif /\* $guard_name \*/\n";
}

sub c_path   { return $_[0]->_some_path( $_[1], '.c' ) }
sub h_path   { return $_[0]->_some_path( $_[1], '.h' ) }
sub cfh_path { return $_[0]->_some_path( $_[1], '.cfh' ) }

sub _some_path {
    my ( $self, $base_dir, $ext ) = @_;
    my @components = split( '::', $self->get_source_class );
    unshift @components, $base_dir
        if defined $base_dir;
    $components[-1] .= $ext;
    return catfile(@components);
}

1;

__END__

__POD__

=head1 NAME

Clownfish::File - Structured representation of the contents of a
Clownfish source file.

=head1 DESCRIPTION

An abstraction representing a file which contains Clownfish code.

=head1 METHODS

=head2 new

    my $file_obj = Clownfish::File->new(
        source_class => 'Crustacean::Lobster',    # required
        parcel       => 'Crustacean',             # default: special
    );

=over

=item * B<source_class> - The class name associated with the source file,
regardless of how what classes are defined in the source file. Example: If
source_class is "Foo::Bar", that implies that the source file could be found
at 'Foo/Bar.cfh' within the source directory and that the output C header file
should be 'Foo/Bar.h' within the target include directory.

=item * B<parcel> - A Clownfish::Parcel or parcel name.

=back

=head2 add_block

    $file_obj->add_block($block);

Add an element to the blocks array.  The block must be either a
Clownfish::Class, a Clownfish::Parcel, or a Clownfish::CBlock.

=head2 blocks

    my $blocks = $file->blocks;

Return all blocks as an arrayref.

=head2 classes

    my $classes = $file->classes;

Return all Clownfish::Class blocks from the file as an arrayref.

=head2 get_modified set_modified

Accessors for the file's "modified" property, which is initially false.

=head2 get_source_class

Accessor.

=head2 c_path h_path cfh_path

    # '/path/to/Source/Class.c', etc.
    my $c_path   = $file->c_path('/path/to');
    my $h_path   = $file->h_path('/path/to');
    my $cfh_path = $file->cfh_path('/path/to');

Given a base directory, return a path name derived from the File's
source_class with the specified extension.

=head2 guard_name

    # e.g. "H_CRUSTACEAN_LOBSTER"
    my $guard_name = $file->guard_name

Return a string used for an include guard in a C header, unique per file.

=head2 guard_start

Return a string opening the include guard.

=head2 guard_close

Return a string closing the include guard.  Other classes count on being able
to match this string.

=cut
