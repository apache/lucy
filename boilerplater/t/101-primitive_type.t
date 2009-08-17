use strict;
use warnings;

package MyPrimitiveType;
use base qw( Boilerplater::Type::Primitive );

package main;
use Test::More tests => 4;

my $type = MyPrimitiveType->new( specifier => 'hump_t' );
ok( $type->is_primitive, "is_primitive" );

my $other = MyPrimitiveType->new( specifier => 'hump_t' );
ok( $type->equals($other), "equals()" );

$other = MyPrimitiveType->new( specifier => 'dump_t' );
ok( !$type->equals($other), "equals() spoiled by specifier" );

$other = MyPrimitiveType->new( specifier => 'hump_t', const => 1 );
ok( !$type->equals($other), "equals() spoiled by const" );

