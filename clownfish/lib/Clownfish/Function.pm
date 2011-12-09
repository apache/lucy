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

package Clownfish::Function;
use Clownfish;

1;

__END__

__POD__

=head1 NAME

Clownfish::Function - Metadata describing a function.

=head1 METHODS

=head2 new

    my $type = Clownfish::Function->new(
        class_name  => 'Crustacean::Lobster::LobsterClaw',  # required
        class_cnick => 'LobClaw',                           # default: special
        return_type => $int_type,                           # required
        param_list  => $param_list,                         # required
        micro_sym   => 'compare',                           # required
        docucomment => $docucomment,                        # default: undef
        parcel      => 'Crustacean',                        # default: special
        exposure    => 'public',                            # default: parcel
        inline      => 1,                                   # default: false
    );

=over

=item * B<class_name> - The full name of the class in whose namespace the
function resides.

=item * B<class_cnick> - The C nickname for the class. 

=item * B<return_type> - A L<Clownfish::Type> representing the function's
return type.

=item * B<param_list> - A L<Clownfish::ParamList> representing the
function's argument list.

=item * B<micro_sym> - The lower case name of the function, without any
namespacing prefixes.

=item * B<docucomment> - A L<Clownfish::DocuComment> describing the
function.

=item * B<parcel> - A L<Clownfish::Parcel> or a parcel name.

=item * B<exposure> - The function's exposure (see L<Clownfish::Symbol>).

=item * B<inline> - Should be true if the function should be inlined by the
compiler.

=back

=head2 get_return_type get_param_list get_docucomment inline 

Accessors.

=head2 void

Returns true if the function has a void return type, false otherwise.

=head2 full_func_sym

A synonym for full_sym().

=head2 short_func_sym

A synonym for short_sym().

=cut
