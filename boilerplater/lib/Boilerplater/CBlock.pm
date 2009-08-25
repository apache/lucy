use strict;
use warnings;

package Boilerplater::CBlock;
use Boilerplater::Util qw( verify_args );
use Carp;

our %new_PARAMS = ( contents => undef, );

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless { %new_PARAMS, @_ }, ref($either) || $either;
    confess("Missing required param 'contents'")
        unless defined $self->{contents};
    return $self;
}

# Accessors.
sub get_contents { shift->{contents} }

1;

__END__

__POD__

=head1 NAME

Boilerplater::CBlock - A block of embedded C code.

=head1 DESCRIPTION

CBlock exists to support embedding literal C code within .bp files:

    parcel Boil;

    class Foo {
        /** Print a greeting. 
         */
        public inline void
        Say_Hello(Foo *self);
    }

    __C__
    #include <stdio.h>
    static CHY_INLINE void
    boil_Foo_say_hello(boil_Foo *self)
    {
        printf("Greetings, Earthlings.\n");
    }
    __END_C__

=head1 METHODS

=head2 new

    my $c_block = Boilerplater::CBlock->new(
        contents => $text,
    );

=over

=item * B<contents> - Raw C code.

=back

=head2 get_contents

Accessor.

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

