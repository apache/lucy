use strict;
use warnings;

package Boilerplater::Binding::Perl::TypeMap;
use base qw( Exporter );
use Scalar::Util qw( blessed );
use Carp;
use Fcntl;
use Config;

our @EXPORT_OK = qw( from_perl to_perl );

# Convert from a Perl scalar to a primitive type.
my %primitives_from_perl = (
    double => sub {"$_[0] = SvNV( $_[1] );"},
    float  => sub {"$_[0] = (float)SvNV( $_[1] );"},
    int    => sub {"$_[0] = (int)SvIV( $_[1] );"},
    short  => sub {"$_[0] = (short)SvIV( $_[1] );"},
    long   => sub {
        $Config{longsize} <= $Config{ivsize}
            ? "$_[0] = (long)SvIV( $_[1] );"
            : "$_[0] = (long)SvNV( $_[1] );";
    },
    size_t     => sub {"$_[0] = (size_t)SvIV( $_[1] );"},
    chy_u64_t  => sub {"$_[0] = (chy_u64_t)SvNV( $_[1] );"},
    chy_u32_t  => sub {"$_[0] = (chy_u32_t)SvUV( $_[1] );"},
    chy_u16_t  => sub {"$_[0] = (chy_u16_t)SvUV( $_[1] );"},
    chy_u8_t   => sub {"$_[0] = (chy_u8_t)SvUV( $_[1] );"},
    chy_i64_t  => sub {"$_[0] = (chy_i64_t)SvNV( $_[1] );"},
    chy_i32_t  => sub {"$_[0] = (chy_i32_t)SvIV( $_[1] );"},
    chy_i16_t  => sub {"$_[0] = (chy_i16_t)SvIV( $_[1] );"},
    chy_i8_t   => sub {"$_[0] = (chy_i8_t)SvIV( $_[1] );"},
    chy_bool_t => sub {"$_[0] = SvTRUE( $_[1] ) ? 1 : 0;"},
);

# Convert from a primitive type to a Perl scalar.
my %primitives_to_perl = (
    double => sub {"$_[0] = newSVnv( $_[1] );"},
    float  => sub {"$_[0] = newSVnv( $_[1] );"},
    int    => sub {"$_[0] = newSViv( $_[1] );"},
    short  => sub {"$_[0] = newSViv( $_[1] );"},
    long   => sub {
        $Config{longsize} <= $Config{ivsize}
            ? "$_[0] = newSViv( $_[1] );"
            : "$_[0] = newSVnv( (NV)$_[1] );";
    },
    size_t    => sub {"$_[0] = newSViv( $_[1] );"},
    chy_u64_t => sub {
        $Config{uvsize} == 8
            ? "$_[0] = newSVuv( $_[1] );"
            : "$_[0] = newSVnv( (NV)$_[1] );";
    },
    chy_u32_t => sub {"$_[0] = newSVuv( $_[1] );"},
    chy_u16_t => sub {"$_[0] = newSVuv( $_[1] );"},
    chy_u8_t  => sub {"$_[0] = newSVuv( $_[1] );"},
    chy_i64_t => sub {
        $Config{ivsize} == 8
            ? "$_[0] = newSViv( $_[1] );"
            : "$_[0] = newSVnv( (NV)$_[1] );";
    },
    chy_i32_t  => sub {"$_[0] = newSViv( $_[1] );"},
    chy_i16_t  => sub {"$_[0] = newSViv( $_[1] );"},
    chy_i8_t   => sub {"$_[0] = newSViv( $_[1] );"},
    chy_bool_t => sub {"$_[0] = newSViv( $_[1] );"},
);

# Extract a Boilerplater object from a Perl SV.
sub _sv_to_bp_obj {
    my ( $type, $bp_var, $xs_var, $stack_var ) = @_;
    my $struct_sym = $type->get_specifier;
    my $vtable     = uc($struct_sym);
    my $third_arg;
    if ( $struct_sym =~ /^[a-z_]*(Obj|CharBuf)$/ ) {
        # Share buffers rather than copy between Perl scalars and BP string
        # types.  Assume that the appropriate ZombieCharBuf has been declared
        # on the stack.
        $third_arg = "&$stack_var";
    }
    else {
        $third_arg = 'NULL';
    }
    return "$bp_var = ($struct_sym*)XSBind_sv_to_lucy_obj($xs_var, "
        . "$vtable, $third_arg);";
}

sub _void_star_to_lucy {
    my ( $type, $bp_var, $xs_var ) = @_;
    # Assume that void* is a reference SV -- either a hashref or an arrayref.
    return qq|if (SvROK($xs_var)) {
            $bp_var = SvRV($xs_var);
        }
        else {
            $bp_var = NULL; /* avoid uninitialized compiler warning */
            LUCY_THROW(LUCY_ERR, "$bp_var is not a reference");
        }\n|;
}

sub from_perl {
    my ( $type, $bp_var, $xs_var, $stack_var ) = @_;
    confess("Not a Boilerplater::Type")
        unless blessed($type) && $type->isa('Boilerplater::Type');

    if ( $type->is_object ) {
        return _sv_to_bp_obj( $type, $bp_var, $xs_var, $stack_var );
    }
    elsif ( $type->is_primitive ) {
        if ( my $sub = $primitives_from_perl{ $type->to_c } ) {
            return $sub->( $bp_var, $xs_var );
        }
    }
    elsif ( $type->is_composite ) {
        if ( $type->to_c eq 'void*' ) {
            return _void_star_to_lucy( $type, $bp_var, $xs_var );
        }
    }

    confess( "Missing typemap for " . $type->to_c );
}

sub to_perl {
    my ( $type, $xs_var, $bp_var ) = @_;
    confess("Not a Boilerplater::Type")
        unless ref($type) && $type->isa('Boilerplater::Type');
    my $type_str = $type->to_c;

    if ( $type->is_object ) {
        return "$xs_var = $bp_var == NULL ? newSV(0) : "
            . "XSBind_lucy_to_perl((lucy_Obj*)$bp_var);";
    }
    elsif ( $type->is_primitive ) {
        if ( my $sub = $primitives_to_perl{$type_str} ) {
            return $sub->( $xs_var, $bp_var );
        }
    }
    elsif ( $type->is_composite ) {
        if ( $type_str eq 'void*' ) {
            # Assume that void* is a reference SV -- either a hashref or an
            # arrayref.
            return "$xs_var = newRV_inc( (SV*)($bp_var) );";
        }
    }

    confess("Missing typemap for '$type_str'");
}

sub write_xs_typemap {
    my ( undef, %args ) = @_;
    my $hierarchy = $args{hierarchy};

    my $typemap_start  = _typemap_start();
    my $typemap_input  = _typemap_input_start();
    my $typemap_output = _typemap_output_start();

    for my $class ( $hierarchy->ordered_classes ) {
        my $full_struct_sym = $class->full_struct_sym;
        my $vtable          = $class->full_vtable_var;
        my $label           = $vtable . "_";
        $typemap_start .= "$full_struct_sym*\t$label\n";
        $typemap_input .= <<END_INPUT;
$label
    \$var = ($full_struct_sym*)XSBind_sv_to_lucy_obj(\$arg, $vtable, NULL);

END_INPUT

        $typemap_output .= <<END_OUTPUT;
$label
    \$arg = (SV*)Lucy_Obj_To_Host(\$var);
    LUCY_DECREF(\$var);

END_OUTPUT
    }

    # Blast it out.
    sysopen( my $typemap_fh, 'typemap', O_CREAT | O_WRONLY | O_EXCL )
        or die "Couldn't open 'typemap' for writing: $!";
    print $typemap_fh "$typemap_start $typemap_input $typemap_output"
        or die "Print to 'typemap' failed: $!";
}

sub _typemap_start {
    my $content = <<END_STUFF;
# Auto-generated file.

TYPEMAP
chy_bool_t\tCHY_BOOL
chy_i8_t\tCHY_SIGNED_INT
chy_i16_t\tCHY_SIGNED_INT
chy_i32_t\tCHY_SIGNED_INT
chy_i64_t\tCHY_BIG_SIGNED_INT
chy_u8_t\tCHY_UNSIGNED_INT
chy_u16_t\tCHY_UNSIGNED_INT
chy_u32_t\tCHY_UNSIGNED_INT
chy_u64_t\tCHY_BIG_UNSIGNED_INT

lucy_ZombieCharBuf\tZOMBIECHARBUF_NOT_POINTER
END_STUFF

    return $content;
}

sub _typemap_input_start {
    my ( $big_signed_convert, $big_unsigned_convert );
    if ( $Config{ivsize} == 8 ) {
        $big_signed_convert   = '$var = ($type)SvIV($arg);';
        $big_unsigned_convert = '$var = ($type)SvUV($arg);';
    }
    else {
        $big_signed_convert   = '$var   = ($type)SvNV($arg);';
        $big_unsigned_convert = '$var = ($type)SvNV($arg);';
    }
    return <<END_STUFF;
    
INPUT

CHY_BOOL
    \$var = (\$type)SvTRUE(\$arg);

CHY_SIGNED_INT 
    \$var = (\$type)SvIV(\$arg);

CHY_UNSIGNED_INT
    \$var = (\$type)SvUV(\$arg);

CHY_BIG_SIGNED_INT 
    $big_signed_convert

CHY_BIG_UNSIGNED_INT 
    $big_unsigned_convert

ZOMBIECHARBUF_NOT_POINTER
    \$var = lucy_ZCB_make_str(SvPVutf8_nolen(\$arg), SvCUR(\$arg));

END_STUFF
}

sub _typemap_output_start {
    my ( $big_signed_convert, $big_unsigned_convert );
    if ( $Config{ivsize} == 8 ) {
        $big_signed_convert   = 'sv_setiv($arg, (IV)$var);';
        $big_unsigned_convert = 'sv_setuv($arg, (UV)$var);';
    }
    else {
        $big_signed_convert   = 'sv_setnv($arg, (NV)$var);';
        $big_unsigned_convert = 'sv_setnv($arg, (NV)$var);';
    }
    return <<END_STUFF;

OUTPUT

CHY_BOOL
    sv_setiv(\$arg, (IV)\$var);

CHY_SIGNED_INT
    sv_setiv(\$arg, (IV)\$var);

CHY_UNSIGNED_INT
    sv_setuv(\$arg, (UV)\$var);

CHY_BIG_SIGNED_INT
    $big_signed_convert

CHY_BIG_UNSIGNED_INT
    $big_unsigned_convert

END_STUFF
}

1;

__END__

__POD__

=head1 NAME

Boilerplater::Binding::Perl::TypeMap - Convert between BP and Perl via XS.

=head1 DESCRIPTION

TypeMap serves up C code fragments for translating between Perl data
structures and Boilerplater data structures.  The functions to_perl() and
from_perl() achieve this for individual types; write_xs_typemap() exports all
types using the XS "typemap" format documented in C<perlxs>.

=head1 FUNCTIONS

=head2 from_perl

    my $c_code = from_perl( $type, $bp_var, $xs_var, $stack_var );

Return C code which converts from a Perl scalar to a variable of type $type.

Variable declarations must precede the returned code, as from_perl() won't
make any declarations itself.

=over

=item * B<type> - A Boilerplater::Type, which will be used to select the
mapping code.

=item * B<bp_var> - The name of the variable being assigned to.

=item * B<xs_var> - The C name of the Perl scalar from which we are extracting
a value.

=item * B<stack_var> - Only required needed when C<type> is
Boilerplater::Object indicating that C<bp_var> is an either an Obj or a
CharBuf.  When passing strings or other simple types to Boilerplater functions
from Perl, we allow the user to supply simple scalars rather than forcing them
to create Boilerplater objects.  We do this by creating a ZombieCharBuf on the
stack and assigning the string from the Perl scalar to it.  C<stack_var> is
the name of that ZombieCharBuf wrapper.  

=back

=head2 to_perl

    my $c_code = to_perl( $type, $xs_var, $bp_var );

Return C code which converts from a variable of type $type to a Perl scalar.

Variable declarations must precede the returned code, as to_perl() won't make
any declarations itself.

=over

=item * B<type> - A Boilerplater::Type, which will be used to select the
mapping code.

=item * B<xs_var> - The C name of the Perl scalar being assigned to.

=item * B<bp_var> - The name of the variable from which we are extracting a
value.

=back

=head1 CLASS METHODS

=head2 write_xs_typemap 

    Boilerplater::Binding::Perl::Typemap->write_xs_typemap(
        hierarchy => $hierarchy,
    );

=over

=item * B<hierarchy> - A L<Boilerplater::Hierarchy>.

=back 

Auto-generate a "typemap" file that adheres to the conventions documented in
L<perlxs>.  

We generate this file on the fly rather than maintain a static copy because we
want an entry for each Boilerplater type so that we can differentiate between
them when checking arguments.  Keeping the entries up-to-date manually as
classes come and go would be a pain.

=head1 COPYRIGHT AND LICENSE

    /**
     * Copyright 2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

=cut

