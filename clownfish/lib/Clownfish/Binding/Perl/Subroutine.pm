use strict;
use warnings;

package Clownfish::Binding::Perl::Subroutine;
use Carp;
use Clownfish::Class;
use Clownfish::Function;
use Clownfish::Method;
use Clownfish::Variable;
use Clownfish::ParamList;
use Clownfish::Util qw( verify_args );

our %new_PARAMS = (
    param_list         => undef,
    alias              => undef,
    class_name         => undef,
    retval_type        => undef,
    use_labeled_params => undef,
);

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless { %new_PARAMS, @_, }, ref($either) || $either;
    for (qw( param_list class_name alias retval_type )) {
        confess("$_ is required") unless defined $self->{$_};
    }
    return $self;
}

sub get_class_name     { shift->{class_name} }
sub use_labeled_params { shift->{use_labeled_params} }

sub perl_name { 
    my $self = shift;
    return "$self->{class_name}::$self->{alias}";
}

sub c_name {
    my $self   = shift;
    my $c_name = "XS_" . $self->perl_name;
    $c_name =~ s/:+/_/g;
    return $c_name;
}

sub c_name_list {
    my $self = shift;
    return $self->{param_list}->name_list;
}

my %params_hash_vals_map = (
    NULL  => 'undef',
    true  => 1,
    false => 0,
);

sub params_hash_def {
    my $self = shift;
    return unless $self->{use_labeled_params};

    my $params_hash_name = $self->perl_name . "_PARAMS";
    my $arg_vars         = $self->{param_list}->get_variables;
    my $vals             = $self->{param_list}->get_initial_values;
    my @pairs;
    for ( my $i = 1; $i < @$arg_vars; $i++ ) {
        my $var = $arg_vars->[$i];
        my $val = $vals->[$i];
        if ( !defined $val ) {
            $val = 'undef';
        }
        elsif ( exists $params_hash_vals_map{$val} ) {
            $val = $params_hash_vals_map{$val};
        }
        push @pairs, $var->micro_sym . " => $val,";
    }

    if (@pairs) {
        my $list = join( "\n    ", @pairs );
        return qq|\%$params_hash_name = (\n    $list\n);\n|;
    }
    else {
        return qq|\%$params_hash_name = ();\n|;
    }
}

sub var_declarations {
    my $self     = shift;
    my $arg_vars = $self->{param_list}->get_variables;
    my @var_declarations = map { $_->local_declaration } @$arg_vars;
    if ( !$self->{retval_type}->is_void ) {
        my $return_type = $self->{retval_type}->to_c;
        push @var_declarations, "$return_type retval;";
    }
    if ( $self->{use_labeled_params} ) {
        push @var_declarations,
            map { "SV* " . $_->micro_sym . "_sv = NULL;" }
            @$arg_vars[ 1 .. $#$arg_vars ];
    }
    return join( "\n        ", @var_declarations );
}

sub xsub_def { confess "Abstract method" }

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Perl::Subroutine - Abstract base binding for a
Clownfish::Function.

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
        retval_type        => $type,                 # required
        use_labeled_params => 1,                     # default: false
    );

Abstract constructor.

=over

=item * B<param_list> - A L<Clownfish::ParamList>.

=item * B<alias> - The local, unqualified name for the Perl subroutine that
will be used to invoke the function.

=item * B<class_name> - The name of the Perl class that the subroutine belongs
to.

=item * B<retval_type> - The return value's L<Type|Clownfish::Type>.

=item * B<use_labeled_params> - True if the binding should take hash-style
labeled parameters, false if it should take positional arguments.

=back

=head2 xsub_def

Abstract method which must return C code (not XS code) defining the Perl XSUB.

=head2 var_declarations

Generate C code containing declarations for subroutine-specific automatic
variables needed by the XSUB.

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

