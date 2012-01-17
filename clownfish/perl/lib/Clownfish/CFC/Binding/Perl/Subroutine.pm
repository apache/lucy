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

package Clownfish::CFC::Binding::Perl::Subroutine;
use Clownfish::CFC;

1;

__END__

__POD__

=head1 NAME

Clownfish::CFC::Binding::Perl::Subroutine - Abstract base binding for a
Clownfish::CFC::Function.

=head1 SYNOPSIS

    # Abstract base class.

=head1 DESCRIPTION

This class is used to generate binding code for invoking Clownfish's
functions and methods across the Perl/C barrier.

=head1 METHODS

=head2 new

    my $binding = $subclass->SUPER::new(
        param_list         => $param_list,           # required
        alias              => 'pinch',               # required
        class_name         => 'Crustacean::Claw',    # required
        use_labeled_params => 1,                     # default: false
    );

Abstract constructor.

=over

=item * B<param_list> - A L<Clownfish::CFC::ParamList>.

=item * B<alias> - The local, unqualified name for the Perl subroutine that
will be used to invoke the function.

=item * B<class_name> - The name of the Perl class that the subroutine belongs
to.

=item * B<use_labeled_params> - True if the binding should take hash-style
labeled parameters, false if it should take positional arguments.

=back

=head2 xsub_def

Abstract method which must return C code (not XS code) defining the Perl XSUB.

=head2 get_class_name use_labeled_params

Accessors.

=head2 perl_name

Returns the fully-qualified perl sub name.

=head2 c_name

Returns the fully-qualified name of the C function that implements the XSUB.

=head2 c_name_list

Returns a string containing the names of arguments to feed to bound C
function, joined by commas.

=head2 params_hash_def

Return Perl code initializing a package-global hash where all the keys are the
names of labeled params.  The hash's name consists of the the binding's
perl_name() plus "_PARAMS".

=cut
