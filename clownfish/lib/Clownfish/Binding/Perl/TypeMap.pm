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

package Clownfish::Binding::Perl::TypeMap;
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
    uint64_t   => sub {"$_[0] = (uint64_t)SvNV( $_[1] );"},
    uint32_t   => sub {"$_[0] = (uint32_t)SvUV( $_[1] );"},
    uint16_t   => sub {"$_[0] = (uint16_t)SvUV( $_[1] );"},
    uint8_t    => sub {"$_[0] = (uint8_t)SvUV( $_[1] );"},
    int64_t    => sub {"$_[0] = (int64_t)SvNV( $_[1] );"},
    int32_t    => sub {"$_[0] = (int32_t)SvIV( $_[1] );"},
    int16_t    => sub {"$_[0] = (int16_t)SvIV( $_[1] );"},
    int8_t     => sub {"$_[0] = (int8_t)SvIV( $_[1] );"},
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
    size_t   => sub {"$_[0] = newSViv( $_[1] );"},
    uint64_t => sub {
        $Config{uvsize} == 8
            ? "$_[0] = newSVuv( $_[1] );"
            : "$_[0] = newSVnv( (NV)$_[1] );";
    },
    uint32_t => sub {"$_[0] = newSVuv( $_[1] );"},
    uint16_t => sub {"$_[0] = newSVuv( $_[1] );"},
    uint8_t  => sub {"$_[0] = newSVuv( $_[1] );"},
    int64_t  => sub {
        $Config{ivsize} == 8
            ? "$_[0] = newSViv( $_[1] );"
            : "$_[0] = newSVnv( (NV)$_[1] );";
    },
    int32_t    => sub {"$_[0] = newSViv( $_[1] );"},
    int16_t    => sub {"$_[0] = newSViv( $_[1] );"},
    int8_t     => sub {"$_[0] = newSViv( $_[1] );"},
    chy_bool_t => sub {"$_[0] = newSViv( $_[1] );"},
);

# Extract a Clownfish object from a Perl SV.
sub _sv_to_cf_obj {
    my ( $type, $cf_var, $xs_var ) = @_;
    my $struct_sym = $type->get_specifier;
    my $vtable     = uc($struct_sym);
    if ( $struct_sym =~ /^[a-z_]*(Obj|CharBuf)$/ ) {
        # Share buffers rather than copy between Perl scalars and Clownfish
        # string types.  Assume that the appropriate ZombieCharBuf has been
        # declared on the stack.
        return "$cf_var = ($struct_sym*)XSBind_sv_to_cfish_obj($xs_var, "
            . "$vtable, alloca(cfish_ZCB_size()));";
    }
    else {
        return "$cf_var = ($struct_sym*)XSBind_sv_to_cfish_obj($xs_var, "
            . "$vtable, NULL);";
    }
}

sub from_perl {
    my ( $type, $cf_var, $xs_var ) = @_;
    confess("Not a Clownfish::Type")
        unless blessed($type) && $type->isa('Clownfish::Type');

    if ( $type->is_object ) {
        return _sv_to_cf_obj( $type, $cf_var, $xs_var );
    }
    elsif ( $type->is_primitive ) {
        if ( my $sub = $primitives_from_perl{ $type->to_c } ) {
            return $sub->( $cf_var, $xs_var );
        }
    }

    confess( "Missing typemap for " . $type->to_c );
}

sub to_perl {
    my ( $type, $xs_var, $cf_var ) = @_;
    confess("Not a Clownfish::Type")
        unless ref($type) && $type->isa('Clownfish::Type');
    my $type_str = $type->to_c;

    if ( $type->is_object ) {
        return "$xs_var = $cf_var == NULL ? newSV(0) : "
            . "XSBind_cfish_to_perl((cfish_Obj*)$cf_var);";
    }
    elsif ( $type->is_primitive ) {
        if ( my $sub = $primitives_to_perl{$type_str} ) {
            return $sub->( $xs_var, $cf_var );
        }
    }
    elsif ( $type->is_composite ) {
        if ( $type_str eq 'void*' ) {
            # Assume that void* is a reference SV -- either a hashref or an
            # arrayref.
            return "$xs_var = newRV_inc( (SV*)($cf_var) );";
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
    \$var = ($full_struct_sym*)XSBind_sv_to_cfish_obj(\$arg, $vtable, NULL);

END_INPUT

        $typemap_output .= <<END_OUTPUT;
$label
    \$arg = (SV*)Cfish_Obj_To_Host((cfish_Obj*)\$var);
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
int8_t\tCHY_SIGNED_INT
int16_t\tCHY_SIGNED_INT
int32_t\tCHY_SIGNED_INT
int64_t\tCHY_BIG_SIGNED_INT
uint8_t\tCHY_UNSIGNED_INT
uint16_t\tCHY_UNSIGNED_INT
uint32_t\tCHY_UNSIGNED_INT
uint64_t\tCHY_BIG_UNSIGNED_INT

const lucy_CharBuf*\tCONST_CHARBUF
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

CONST_CHARBUF
    \$var = (const cfish_CharBuf*)CFISH_ZCB_WRAP_STR(SvPVutf8_nolen(\$arg), SvCUR(\$arg));

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

Clownfish::Binding::Perl::TypeMap - Convert between Clownfish and Perl via XS.

=head1 DESCRIPTION

TypeMap serves up C code fragments for translating between Perl data
structures and Clownfish data structures.  The functions to_perl() and
from_perl() achieve this for individual types; write_xs_typemap() exports all
types using the XS "typemap" format documented in C<perlxs>.

=head1 FUNCTIONS

=head2 from_perl

    my $c_code = from_perl( $type, $cf_var, $xs_var );

Return C code which converts from a Perl scalar to a variable of type $type.

Variable declarations must precede the returned code, as from_perl() won't
make any declarations itself.

=over

=item * B<type> - A Clownfish::Type, which will be used to select the
mapping code.

=item * B<cf_var> - The name of the variable being assigned to.

=item * B<xs_var> - The C name of the Perl scalar from which we are extracting
a value.

=back

=head2 to_perl

    my $c_code = to_perl( $type, $xs_var, $cf_var );

Return C code which converts from a variable of type $type to a Perl scalar.

Variable declarations must precede the returned code, as to_perl() won't make
any declarations itself.

=over

=item * B<type> - A Clownfish::Type, which will be used to select the
mapping code.

=item * B<xs_var> - The C name of the Perl scalar being assigned to.

=item * B<cf_var> - The name of the variable from which we are extracting a
value.

=back

=head1 CLASS METHODS

=head2 write_xs_typemap 

    Clownfish::Binding::Perl::Typemap->write_xs_typemap(
        hierarchy => $hierarchy,
    );

=over

=item * B<hierarchy> - A L<Clownfish::Hierarchy>.

=back 

Auto-generate a "typemap" file that adheres to the conventions documented in
L<perlxs>.  

We generate this file on the fly rather than maintain a static copy because we
want an entry for each Clownfish type so that we can differentiate between
them when checking arguments.  Keeping the entries up-to-date manually as
classes come and go would be a pain.

=cut
