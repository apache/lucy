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

package Clownfish::Type::VAList;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = ( specifier => 'va_list' );

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    return $either->SUPER::new(
        specifier => 'va_list',
        c_string  => 'va_list',
        va_list   => 1,
    );
}

1;

__END__


__POD__

=head1 NAME

Clownfish::Type::VAList - A Type to support C's va_list.

=head1 DESCRIPTION

Clownfish::Type::VAList represents the C va_list type, from stdarg.h.

=head1 METHODS

=head2 new

    my $type = Clownfish::Type::VAList->new(
        specifier => 'va_list',    # default: va_list
    );

=over

=item * B<specifier>.  Must be "va_list" if supplied.

=back

=cut

