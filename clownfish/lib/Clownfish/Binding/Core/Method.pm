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

package Clownfish::Binding::Core::Method;
use Clownfish;

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Core::Method - Generate core C code for a method.

=head1 DESCRIPTION

Clownfish::Method is an abstract specification; this class generates C code
which implements the specification.

=head1 METHODS

=head2 method_def

    my $c_code = Clownfish::Binding::Core::Method->method_def(
        method => $method,
        $class => $class,
    );

Return C code for the static inline vtable method invocation function.  

=over

=item * B<method> - A L<Clownfish::Method>.

=item * B<class> - The L<Clownfish::Class> which will be invoking the method -
LobsterClaw needs its own method invocation function even if the method was
defined in Claw.

=back

=head2 typedef_dec

    my $c_code = Clownfish::Binding::Core::Method->typedef_dec($method);

Return C code expressing a typedef declaration for the method.

=head2 callback_dec

    my $c_code = Clownfish::Binding::Core::Method->callback_dec($method);

Return C code declaring the Callback object for this method.

=head2 callback_obj_def

    my $c_code 
        = Clownfish::Binding::Core::Method->callback_obj_def($method);

Return C code defining the Callback object for this method, which stores
introspection data and a pointer to the callback function.

=head2 callback_def

    my $c_code = Clownfish::Binding::Core::Method->callback_def($method);

Return C code implementing a callback to the Host for this method.  This code
is used when a Host method has overridden a method in a Clownfish class.

=head2 abstract_method_def

    my $c_code 
        = Clownfish::Binding::Core::Method->abstract_method_def($method);

Return C code implementing a version of the method which throws an "abstract
method" error at runtime, for methods which are declared as "abstract" in a
Clownfish header file.

=cut
