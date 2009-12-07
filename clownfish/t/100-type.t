use strict;
use warnings;

package MyType;
use base qw( Clownfish::Type );

package main;
use Test::More tests => 12;
use Clownfish::Parcel;

my $boil_parcel = Clownfish::Parcel->singleton( name => 'Boil' );

my $type = MyType->new( parcel => 'Boil', specifier => 'mytype_t' );
is( $type->get_parcel, $boil_parcel,
    "constructor changes parcel name to Parcel singleton" );

ok( !defined $type->to_c, "to_c()" );
$type->set_c_string("mytype_t");
is( $type->to_c, "mytype_t", "set_c_string()" );
ok( !$type->const, "const() is off by default" );
is( $type->get_specifier, "mytype_t", "get_specifier()" );

ok( !$type->is_object,      "is_object() false by default" );
ok( !$type->is_integer,     "is_integer() false by default" );
ok( !$type->is_floating,    "is_floating() false by default" );
ok( !$type->is_void,        "is_void() false by default" );
ok( !$type->is_composite,   "is_composite() false by default" );
ok( !$type->is_string_type, "is_string_type() false by default" );

ok( $type->equals( MyType->new ), "equals() depends solely on class" );

