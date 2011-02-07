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

package Clownfish::Type::Void;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = (
    const     => undef,
    specifier => 'void',
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $c_string = $args{const} ? "const void" : "void";
    return $either->SUPER::new(
        %new_PARAMS,
        %args,
        specifier => 'void',
        c_string  => $c_string,
        void      => 1,
    );
}

1;

__END__

=head1 NAME

Clownfish::Type::Void - The void Type.

=head1 DESCRIPTION

Clownfish::Type::Void is used to represent a void return type.  It is also
used in conjuction with with C<< Clownfish::Type->new_composite >> to support
the C<void*> opaque pointer type.

=head1 METHODS

=head2 new

    my $type = Clownfish::Type::Void->new(
        specifier => 'void',    # default: void
        const     => 1,         # default: undef
    );

=over

=item * B<specifier> - Must be "void" if supplied.

=item * B<const> - Should be true if the type is const.  (Useful in the
context of C<const void*>).

=back

=cut
