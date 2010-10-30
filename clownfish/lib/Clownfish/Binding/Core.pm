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

package Clownfish::Binding::Core;
use Clownfish::Util qw( a_isa_b verify_args );
use Clownfish::Binding::Core::File;
use File::Spec::Functions qw( catfile );
use Fcntl;

our %new_PARAMS = (
    hierarchy => undef,
    dest      => undef,
    header    => undef,
    footer    => undef,
);

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless { %new_PARAMS, @_ }, ref($either) || $either;

    # Validate.
    for ( keys %new_PARAMS ) {
        confess("Missing required param '$_'") unless defined $self->{$_};
    }
    confess("Not a Hierarchy")
        unless a_isa_b( $self->{hierarchy}, "Clownfish::Hierarchy" );

    return $self;
}

sub write_all_modified {
    my ( $self, $modified ) = @_;
    my $hierarchy = $self->{hierarchy};
    my $header    = $self->{header};
    my $footer    = $self->{footer};
    my $dest      = $self->{dest};

    $modified = $hierarchy->propagate_modified($modified);

    # Iterate over all File objects, writing out those which don't have
    # up-to-date auto-generated files.
    my %written;
    for my $file ( $hierarchy->files ) {
        next unless $file->get_modified;
        my $source_class = $file->get_source_class;
        next if $written{$source_class};
        $written{$source_class} = 1;
        Clownfish::Binding::Core::File->write_h(
            file   => $file,
            dest   => $dest,
            header => $header,
            footer => $footer,
        );
        Clownfish::Binding::Core::File->write_c(
            file   => $file,
            dest   => $dest,
            header => $header,
            footer => $footer,
        );
    }

    # If any class definition changed, rewrite the boil.h file.
    $self->_write_boil_h if $modified;

    return $modified;
}

# Write the "boil.h" header file, which contains common symbols needed by all
# classes, plus typedefs for all class structs.
sub _write_boil_h {
    my $self     = shift;
    my @ordered  = $self->{hierarchy}->ordered_classes;
    my $typedefs = "";

    # Declare object structs for all instantiable classes.
    for my $class (@ordered) {
        next if $class->inert;
        my $full_struct = $class->full_struct_sym;
        $typedefs .= "typedef struct $full_struct $full_struct;\n";
    }

    my $filepath = catfile( $self->{dest}, "boil.h" );
    unlink $filepath;
    sysopen( my $fh, $filepath, O_CREAT | O_EXCL | O_WRONLY )
        or confess("Can't open '$filepath': $!");
    print $fh <<END_STUFF;
$self->{header}
#ifndef BOIL_H
#define BOIL_H 1

#include <stddef.h>
#include "charmony.h"

$typedefs

/* Refcount / host object */
typedef union {
    size_t  count;
    void   *host_obj;
} kino_ref_t;

/* Generic method pointer.
 */
typedef void
(*kino_method_t)(const void *vself);

/* Access the function pointer for a given method from the vtable.
 */
#define LUCY_METHOD(_vtable, _class_nick, _meth_name) \\
     kino_method(_vtable, \\
     Kino_ ## _class_nick ## _ ## _meth_name ## _OFFSET)

static CHY_INLINE kino_method_t
kino_method(const void *vtable, size_t offset) 
{
    union { char *cptr; kino_method_t *fptr; } ptr;
    ptr.cptr = (char*)vtable + offset;
    return ptr.fptr[0];
}

/* Access the function pointer for the given method in the superclass's
 * vtable. */
#define LUCY_SUPER_METHOD(_vtable, _class_nick, _meth_name) \\
     kino_super_method(_vtable, \\
     Kino_ ## _class_nick ## _ ## _meth_name ## _OFFSET)

extern size_t kino_VTable_offset_of_parent;
static CHY_INLINE kino_method_t
kino_super_method(const void *vtable, size_t offset) 
{
    char *vt_as_char = (char*)vtable;
    kino_VTable **parent_ptr 
        = (kino_VTable**)(vt_as_char + kino_VTable_offset_of_parent);
    return kino_method(*parent_ptr, offset);
}

/* Return a boolean indicating whether a method has been overridden.
 */
#define LUCY_OVERRIDDEN(_self, _class_nick, _meth_name, _micro_name) \\
        (kino_method(*((kino_VTable**)_self), \\
            Kino_ ## _class_nick ## _ ## _meth_name ## _OFFSET )\\
            != (kino_method_t)kino_ ## _class_nick ## _ ## _micro_name )

#ifdef KINO_USE_SHORT_NAMES
  #define METHOD                   LUCY_METHOD
  #define SUPER_METHOD             LUCY_SUPER_METHOD
  #define OVERRIDDEN               LUCY_OVERRIDDEN
#endif

typedef struct kino_Callback {
    const char    *name;
    size_t         name_len;
    kino_method_t  func;
    size_t         offset;
} kino_Callback;

#endif /* BOIL_H */

$self->{footer}

END_STUFF
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Core - Generate core C code for a Clownfish::Hierarchy.

=head1 SYNOPSIS

    my $hierarchy = Clownfish::Hierarchy->new(
        source => '/path/to/clownfish/files',
        dest   => 'autogen',
    );
    $hierarchy->build;
    my $core_binding = Clownfish::Binding::Core->new(
        hierarchy => $hierarchy,
        dest      => 'autogen',
        header    => "/* Auto-generated file. */\n",
        footer    => $copyfoot,
    );
    my $modified = $core_binding->write_all_modified($modified);

=head1 DESCRIPTION

A Clownfish::Hierarchy describes an abstract specifiction for a class
hierarchy; Clownfish::Binding::Core is responsible for auto-generating C
code which implements that specification.

=head1 METHODS

=head2 new

    my $binding = Clownfish::Binding::Core->new(
        hierarchy => $hierarchy,            # required
        dest      => '/path/to/autogen',    # required
        header    => $header,               # required
        footer    => $footer,               # required
    );

=over

=item * B<hierarchy> - A L<Clownfish::Hierarchy>.

=item * B<dest> - The directory where C output files will be written.

=item * B<header> - Text which will be prepended to each generated C file --
typically, an "autogenerated file" warning.

=item * B<footer> - Text to be appended to the end of each generated C file --
typically copyright information.

=back

=head2 write_all_modified

Call C<< $hierarchy->propagate_modified >> to establish which classes do not
have up-to-date generated .c and .h files, then traverse the hierarchy writing
all necessary files.

=cut

