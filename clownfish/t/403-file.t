use strict;
use warnings;

use Test::More tests => 17;

use Boilerplater::File;
use Boilerplater::Parser;

my $parser = Boilerplater::Parser->new;

my $parcel_declaration = "parcel Stuff;";
my $class_content      = qq|
    class Stuff::Thing {
        Foo *foo;
        Bar *bar;
    }
|;
my $c_block = "__C__\nint foo;\n__END_C__\n";

my $file = $parser->file( "$parcel_declaration\n$class_content\n$c_block",
    0, source_class => 'Stuff::Thing' );

is( $file->get_source_class, "Stuff::Thing", "get_source_class" );

my $guard_name = $file->guard_name;
is( $guard_name, "H_STUFF_THING", "guard_name" );
like( $file->guard_start, qr/$guard_name/, "guard_start" );
like( $file->guard_close, qr/$guard_name/,
    "guard_close includes guard_name" );

ok( !$file->get_modified, "modified false at start" );
$file->set_modified(1);
ok( $file->get_modified, "set_modified, get_modified" );

is( $file->bp_path('/path/to'), "/path/to/Stuff/Thing.bp", "bp_path" );
is( $file->c_path('/path/to'),  "/path/to/Stuff/Thing.c",  "c_path" );
is( $file->h_path('/path/to'),  "/path/to/Stuff/Thing.h",  "h_path" );

my @classes = $file->classes;
is( scalar @classes, 1, "classes() filters blocks" );
my $class = $classes[0];
my ( $foo, $bar ) = $class->member_vars;
is( $foo->get_type->get_specifier,
    'stuff_Foo', 'file production picked up parcel def' );
is( $bar->get_type->get_specifier, 'stuff_Bar', 'parcel def is sticky' );

my @blocks = $file->blocks;
is( scalar @blocks, 3, "all three blocks" );
isa_ok( $blocks[0], "Boilerplater::Parcel" );
isa_ok( $blocks[1], "Boilerplater::Class" );
isa_ok( $blocks[2], "Boilerplater::CBlock" );

$file = $parser->file( $class_content, 0, source_class => 'Stuff::Thing' );
($class) = $file->classes;
( $foo, $bar ) = $class->member_vars;
is( $foo->get_type->get_specifier, 'Foo', 'file production resets parcel' );

