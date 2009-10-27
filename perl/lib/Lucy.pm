package Lucy;
use strict;
use warnings;

use 5.008003;

our $VERSION = 0.01;
$VERSION = eval $VERSION;

use XSLoader;
BEGIN { XSLoader::load( 'Lucy', '0.01' ) }

use Lucy::Autobinding;

sub error {$Lucy::Object::Err::error}

{
    package Lucy::Util::ToolSet;
    use Carp qw( carp croak cluck confess );
    use Scalar::Util qw( blessed );
    use Storable qw( nfreeze thaw );

    BEGIN {
        push our @ISA, 'Exporter';
        our @EXPORT_OK = qw(
            carp
            croak
            cluck
            confess
            blessed
            nfreeze
            thaw
        );
    }
}

{
    package Lucy::Object::Err;
    use Lucy::Util::ToolSet qw( blessed );

    sub do_to_string { shift->to_string }
    use Carp qw( longmess );
    use overload
        '""'     => \&do_to_string,
        fallback => 1;

    sub do_throw {
        my $err = shift;
        $err->cat_mess( longmess() );
        die $err;
    }

    our $error;
    sub set_error {
        my $val = $_[1];
        if ( defined $val ) {
            confess("Not a Lucy::Object::Err")
                unless ( blessed($val)
                && $val->isa("Lucy::Object::Err") );
        }
        $error = $val;
    }
    sub get_error {$error}
}

{
    package Lucy::Object::VArray;
    no warnings 'redefine';
    sub clone { CORE::shift->_clone }
}

{
    package Lucy::Object::VTable;

    sub find_parent_class {
        my ( undef, $package ) = @_;
        no strict 'refs';
        for my $parent ( @{"$package\::ISA"} ) {
            return $parent if $parent->isa('Lucy::Object::Obj');
        }
        return;
    }

    sub novel_host_methods {
        my ( undef, $package ) = @_;
        no strict 'refs';
        my $stash = \%{"$package\::"};
        my $methods
            = Lucy::Object::VArray->new( capacity => scalar keys %$stash );
        while ( my ( $symbol, $glob ) = each %$stash ) {
            next if ref $glob;
            next unless *$glob{CODE};
            $methods->push( Lucy::Object::CharBuf->new($symbol) );
        }
        return $methods;
    }

    sub _register {
        my ( undef, %args ) = @_;
        my $singleton_class = $args{singleton}->get_name;
        my $parent_class    = $args{parent}->get_name;
        if ( !$singleton_class->isa($parent_class) ) {
            no strict 'refs';
            push @{"$singleton_class\::ISA"}, $parent_class;
        }
    }
}

1;

__END__

__BINDING__

my $lucy_xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy 

BOOT:
    lucy_Lucy_bootstrap();

IV
_dummy_function()
CODE:
    RETVAL = 1;
OUTPUT:
    RETVAL
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
    parcel     => "Lucy",
    class_name => "Lucy",
    xs_code    => $lucy_xs_code,
);

__POD__

=head1 NAME

Lucy - Search engine library.

=head1 SYNOPSIS

To do.

=head1 DESCRIPTION

To do.

=head1 AUTHOR

Marvin Humphrey E<lt>marvin at rectangular dot comE<gt>

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

