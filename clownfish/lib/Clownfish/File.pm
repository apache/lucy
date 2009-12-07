use strict;
use warnings;

package Boilerplater::File;
use Boilerplater::Util qw( verify_args a_isa_b );
use Boilerplater::Parcel;
use Scalar::Util qw( blessed );
use File::Spec::Functions qw( catfile );
use Carp;

our %new_PARAMS = (
    blocks       => undef,
    source_class => undef,
    parcel       => undef,
);

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless {
        %new_PARAMS,
        blocks       => [],
        source_class => undef,
        modified     => 0,
        @_,
        },
        ref($either) || $either;
    if ( defined $self->{parcel} ) {
        if ( !blessed( $self->{parcel} ) ) {
            $self->{parcel}
                = Boilerplater::Parcel->singleton( name => $self->{parcel} );
        }
        confess("Not a Boilerplater::Parcel")
            unless $self->{parcel}->isa('Boilerplater::Parcel');
    }
    for my $block ( @{ $self->{blocks} } ) {
        next if a_isa_b( $block, "Boilerplater::Parcel" );
        next if a_isa_b( $block, "Boilerplater::Class" );
        next if a_isa_b( $block, "Boilerplater::CBlock" );
        confess("Invalid block: $block");
    }
    confess("Missing required param 'source_class'")
        unless $self->{source_class};
    return $self;
}

sub get_modified     { shift->{modified} }
sub set_modified     { $_[0]->{modified} = $_[1] }
sub get_source_class { shift->{source_class} }

sub blocks { @{ shift->{blocks} } }

sub classes {
    my $self = shift;
    return
        grep { ref $_ and $_->isa('Boilerplater::Class') }
        @{ $self->{blocks} };
}

# Return a string used for an include guard, unique per file.
sub guard_name {
    my $self       = shift;
    my $guard_name = "H_" . uc( $self->{source_class} );
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

sub c_path  { return $_[0]->_some_path( $_[1], '.c' ) }
sub h_path  { return $_[0]->_some_path( $_[1], '.h' ) }
sub bp_path { return $_[0]->_some_path( $_[1], '.bp' ) }

sub _some_path {
    my ( $self, $base_dir, $ext ) = @_;
    my @components = split( '::', $self->{source_class} );
    unshift @components, $base_dir
        if defined $base_dir;
    $components[-1] .= $ext;
    return catfile(@components);
}

1;

__END__

__POD__

=head1 NAME

Boilerplater::File - Structured representation of the contents of a
Boilerplater source file.

=head1 DESCRIPTION

An abstraction representing a file which contains Boilerplater code.

=head1 METHODS

=head2 new

    my $file_obj = Boilerplater::File->new(
        blocks       => \@blocks,            # required
        source_class => 'Dog::Dalmation',    # required
        parcel       => 'Dog',               # default: special
    );

=over

=item * B<blocks> - An arrayref.  Each element must be either a
Boilerplater::Class, a Boilerplater::Parcel, or a Boilerplater::CBlock.

=item * B<source_class> - The class name associated with the source file,
regardless of how what classes are defined in the source file. Example: If
source_class is "Foo::Bar", that implies that the source file could be found
at 'Foo/Bar.bp' within the source directory and that the output C header file
should be 'Foo/Bar.h' within the target include directory.

=item * B<parcel> - A Boilerplater::Parcel or parcel name.

=back

=head2 blocks

    my @blocks = $file->blocks;

Return all blocks as a list.

=head2 classes

    my @classes = $file->classes;

Return all Boilerplater::Class blocks from the file as a list.

=head2 get_modified set_modified

Accessors for the file's "modified" property, which is initially false.

=head2 get_source_class

Accessor.

=head2 c_path h_path bp_path

    # '/path/to/Source/Class.c', etc.
    my $c_path  = $file->c_path('/path/to');
    my $h_path  = $file->h_path('/path/to');
    my $bp_path = $file->bp_path('/path/to');

Given a base directory, return a path name derived from the File's
source_class with the specified extension.

=head2 guard_name

    # e.g. "H_DOG_DALMATION"
    my $guard_name = $file->guard_name

Return a string used for an include guard in a C header, unique per file.

=head2 guard_start

Return a string opening the include guard.

=head2 guard_close

Return a string closing the include guard.  Other classes count on being able
to match this string.

=head1 COPYRIGHT AND LICENSE

    /**
     * Copyright 2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

=cut
