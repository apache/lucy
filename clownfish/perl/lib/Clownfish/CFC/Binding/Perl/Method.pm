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

package Clownfish::CFC::Binding::Perl::Method;
use Clownfish::CFC;

1;

__END__

__POD__

=head1 NAME

Clownfish::CFC::Binding::Perl::Method - Binding for an object method.

=head1 DESCRIPTION

This class isa Clownfish::CFC::Binding::Perl::Subroutine -- see its
documentation for various code-generating routines.

Method bindings use labeled parameters if the C function takes more than one
argument (other than C<self>).  If there is only one argument, the binding
will be set up to accept a single positional argument.

=head1 METHODS

=head2 new

    my $binding = Clownfish::CFC::Binding::Perl::Method->new(
        method => $method,    # required
    );

=over

=item * B<method> - A L<Clownfish::CFC::Method>.

=back

=head2 xsub_def

Generate the XSUB code.

=cut
