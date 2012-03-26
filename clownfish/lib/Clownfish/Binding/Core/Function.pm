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

package Clownfish::Binding::Core::Function;
use Clownfish::Util qw( a_isa_b );

sub func_declaration {
    my ( undef, $function ) = @_;
    confess("Not a Function")
        unless a_isa_b( $function, "Clownfish::Function" );
    my $return_type = $function->get_return_type;
    my $param_list  = $function->get_param_list;
    my $dec         = $function->inline ? 'static CHY_INLINE ' : '';
    $dec .= $return_type->to_c . "\n";
    $dec .= $function->full_func_sym;
    $dec .= "(" . $param_list->to_c . ");";
    return $dec;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Core::Function - Generate core C code for a function.

=head1 CLASS METHODS

=head2 func_declaration

    my $declaration 
        = Clownfish::Binding::Core::Function->func_declaration($function);

Return C code declaring the function's C implementation.

=cut
