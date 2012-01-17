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

package Clownfish::CFC::Binding::Perl::TypeMap;
use Clownfish::CFC;

1;

__END__

__POD__

=head1 NAME

Clownfish::CFC::Binding::Perl::TypeMap - Convert between Clownfish and Perl via XS.

=head1 DESCRIPTION

TypeMap serves up C code fragments for translating between Perl data
structures and Clownfish data structures.  The functions to_perl() and
from_perl() achieve this for individual types; write_xs_typemap() exports all
types using the XS "typemap" format documented in C<perlxs>.

=head1 FUNCTIONS

=head2 from_perl

    my $expression = from_perl( $type, $xs_var );

Return an expression which converts from a Perl scalar to a variable of type
$type.

=over

=item * B<type> - A Clownfish::CFC::Type, which will be used to select the
mapping code.

=item * B<xs_var> - The C name of the Perl scalar from which we are extracting
a value.

=back

=head2 to_perl

    my $c_code = to_perl( $type, $cf_var );

Return an expression converts from a variable of type $type to a Perl scalar.

=over

=item * B<type> - A Clownfish::CFC::Type, which will be used to select the
mapping code.

=item * B<cf_var> - The name of the variable from which we are extracting a
value.

=back

=head1 CLASS METHODS

=head2 write_xs_typemap 

    Clownfish::CFC::Binding::Perl::Typemap->write_xs_typemap(
        hierarchy => $hierarchy,
    );

=over

=item * B<hierarchy> - A L<Clownfish::CFC::Hierarchy>.

=back 

Auto-generate a "typemap" file that adheres to the conventions documented in
L<perlxs>.  

We generate this file on the fly rather than maintain a static copy because we
want an entry for each Clownfish type so that we can differentiate between
them when checking arguments.  Keeping the entries up-to-date manually as
classes come and go would be a pain.

=cut
