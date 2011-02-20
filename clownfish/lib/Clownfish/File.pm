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
use Clownfish::Util qw( verify_args );
use Carp;

our %new_PARAMS = ( source_class => undef, );

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $package = ref($either) || $either;
    return $either->_new( $args{source_class} );
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
    );

=over

=item * B<source_class> - The class name associated with the source file,
regardless of how what classes are defined in the source file. Example: If
source_class is "Foo::Bar", that implies that the source file could be found
at 'Foo/Bar.cfh' within the source directory and that the output C header file
should be 'Foo/Bar.h' within the target include directory.

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
