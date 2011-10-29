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

package Clownfish::Binding::Perl::Constructor;
use base qw( Clownfish::Binding::Perl::Subroutine );
use Carp;
use Clownfish::Util qw( verify_args );

our %new_PARAMS = (
    class => undef,
    alias => undef,
);

sub new {
    my ( $either, %args ) = @_;
    confess $@ unless verify_args( \%new_PARAMS, %args );
    return _new( @args{qw( class alias )} );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Perl::Constructor - Binding for an object method.

=head1 DESCRIPTION

This class isa Clownfish::Binding::Perl::Subroutine -- see its
documentation for various code-generating routines.

Constructors are always bound to accept labeled params, even if there is only
a single argument.

=head1 METHODS

=head2 new

    my $constructor_binding = Clownfish::Binding::Perl::Constructor->new(
        class => $class,
        alias => "_new|init2",
    );

=over

=item * B<class> - A L<Clownfish::Class>.

=item * B<alias> - A specifier for the name of the constructor, and
optionally, a specifier for the implementing function.  If C<alias> has a pipe
character in it, the text to the left of the pipe will be used as the Perl
alias, and the text to the right will be used to determine which C function
should be bound.  The default function is "init".

=back

=head2 xsub_def

Generate the XSUB code.

=cut
