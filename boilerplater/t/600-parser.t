use strict;
use warnings;

use Test::More tests => 30;

BEGIN { use_ok('Boilerplater::Parser') }

my $parser = Boilerplater::Parser->new;
isa_ok( $parser, "Boilerplater::Parser" );

isa_ok( $parser->parcel_definition("parcel Fish;"),
    "Boilerplater::Parcel", "parcel_definition" );
isa_ok( $parser->parcel_definition("parcel Crustacean cnick Crust;"),
    "Boilerplater::Parcel", "parcel_definition with cnick" );

# Set and leave parcel.
my $parcel = $parser->parcel_definition('parcel Boil;')
    or die "failed to process parcel_definition";
is( $Boilerplater::Parser::parcel, $parcel,
    "parcel_definition sets internal \$parcel var" );

is( $parser->class_name_component($_), $_, "class_name_component: $_" )
    for (qw( Foo FooJr FooIII Foo4th ));

ok( !$parser->class_name_component($_), "illegal class_name_component: $_" )
    for (qw( foo fooBar Foo_Bar FOOBAR 1Foo 1FOO ));

is( $parser->class_name($_), $_, "class_name: $_" )
    for (qw( Foo Foo::FooJr Foo::FooJr::FooIII Foo::FooJr::FooIII::Foo4th ));

ok( !$parser->class_name($_), "illegal class_name: $_" )
    for (qw( foo fooBar Foo_Bar FOOBAR 1Foo 1FOO ));

is( $parser->cnick(qq|cnick $_|), $_, "cnick: $_" ) for (qw( Foo FF ));

ok( !$parser->cnick(qq|cnick $_|), "Illegal cnick: $_" )
    for (qw( foo fOO 1Foo ));

