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

package Clownfish::ParamList;
use Clownfish::Variable;
use Clownfish::Util qw( verify_args );
use Carp;

our %new_PARAMS = (
    variables      => undef,
    initial_values => undef,
    variadic       => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $class_name = ref($either) || $either;
    my $variables  = delete $args{variables};
    my $values     = delete $args{initial_values};
    my $variadic   = delete $args{variadic} || 0;

    # Validate variables.
    confess "variables must be an arrayref"
        unless ref($variables) eq 'ARRAY';
    for my $var (@$variables) {
        confess "invalid variable: '$var'"
            unless ref($var) && $var->isa("Clownfish::Variable");
    }

    # Validate or init initial_values.
    if ( defined $values ) {
        confess "variables must be an arrayref"
            unless ref($values) eq 'ARRAY';
        my $num_init = scalar @$values;
        my $num_vars = scalar @$variables;
        confess("mismatch of num vars and init values: $num_vars $num_init")
            unless $num_init == $num_vars;
    }
    else {
        my @initial_values;
        $#initial_values = $#$variables;
        $values          = \@initial_values;
    }

    return $class_name->_new( $variables, $values, $variadic );
}

sub num_vars           { scalar @{ shift->get_variables } }

sub to_c {
    my $self = shift;
    my $string = join( ', ', map { $_->local_c } @{ $self->get_variables } );
    $string .= ", ..." if $self->variadic;
    return $string;
}

sub name_list {
    my $self = shift;
    return join( ', ', map { $_->micro_sym } @{ $self->get_variables } );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::ParamList - parameter list.

=head1 DESCRIPTION

=head1 METHODS

=head2 new

    my $type = Clownfish::ParamList->new(
        variables      => \@vars,    # required
        initial_values => \@vals,    # default: undef
        variadic       => 1,         # default: false
    );

=over

=item * B<variables> - An array where each element is a
L<Clownfish::Variable>. 

=item * B<initial_values> - If supplied, an array of default values, one for
each variable.

=item * B<variadic> - Should be true if the function is variadic.

=back

=head2 get_variables get_initial_values variadic

Accessors. 

=head2 num_vars

Return the number of variables in the ParamList, including "self" for methods.

=head2 to_c

    # Prints "Obj* self, Foo* foo, Bar* bar".
    print $param_list->to_c;

Return a list of the variable's types and names, joined by commas.

=head2 name_list

    # Prints "self, foo, bar".
    print $param_list->name_list;

Return the variable's names, joined by commas.

=cut

